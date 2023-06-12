#ifndef __PCNET_H__
#define __PCNET_H__

#include "system.h"
#include "net.h"
#include "kheap.h"
#include "string.h"
#include "netutils.h"
#include "ports.h"
#include "pci.h"
#include "ethernet.h"
#include "isr.h"

// Driver for AMD PCNET FAST-III

#define PCNET_PIO_MAC_ADDRESS_0(portBase) portBase
#define PCNET_PIO_MAC_ADDRESS_2(portBase) (portBase + 0x02)
#define PCNET_PIO_MAC_ADDRESS_4(portBase) (portBase + 0x04)
#define PCNET_PIO_REGISTER_DATA(portBase) (portBase + 0x10)
#define PCNET_PIO_REGISTER_ADDRESS(portBase) (portBase + 0x12)
#define PCNET_PIO_RESET(portBase) (portBase + 0x14)
#define PCNET_PIO_BUS_CONTROL_REGISTER(portBase) (portBase + 0x16)

typedef struct pcnet_initblock_struct
{
    uint16_t mode;
    unsigned reserved1 : 4;
    unsigned nTxBuffers : 4;
    unsigned reserved2 : 4;
    unsigned nRxBuffers : 4;
    unsigned long long physical_address : 48;
    uint16_t reserved3;
    uint64_t logical_address;
    uint32_t RxBufferDescriptor;
    uint32_t TxBufferDescriptor;
}__attribute__((packed))pcnet_initblock_t;

typedef struct pcnet_bufferDescriptor_struct
{
    uint32_t address;
    uint32_t flags;
    uint32_t flags2;
    uint32_t avail;
}__attribute__((packed))pcnet_bufferDescriptor_t;

typedef struct pcnet_struct
{
    pcnet_initblock_t initblock;

    uint16_t portBase;
    uint16_t MAC0_port;
    uint16_t MAC2_port;
    uint16_t MAC4_port;
    uint16_t RegData_port;
    uint16_t RegAddr_port;
    uint16_t Reset_port;
    uint16_t BusControl_port;

    pcnet_bufferDescriptor_t* dRxBuffer;
    uint8_t dRxBufferMem[2048 + 15];
    uint8_t RxBuffers[(2 * 1024)+15][8];
    uint8_t currentRxBuffer;

    uint32_t dRxBuf_physaddr;
    uint32_t RxBuf_physaddr;

    pcnet_bufferDescriptor_t* dTxBuffer;
    uint8_t dTxBufferMem[2048 + 15];
    uint8_t TxBuffers[(2 * 1024)+15][8];
    uint8_t currentTxBuffer;

    uint32_t dTxBuf_physaddr;
    uint32_t TxBuf_physaddr;
}pcnet_t;

void init_pcnet();
void pcnet_send(void* data, uint32_t len);
void pcnet_handler(registers_t* regs);
uint64_t pcnet_getMACAddress();
uint32_t pcnet_getIPAddress();
void pcnet_setIPAddress(uint32_t ip);
void pcnet_readMACAddress();
void pcnet_reset();
void pcnet_receive();

#endif /*__PCNET_H__*/