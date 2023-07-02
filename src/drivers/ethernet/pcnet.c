#include "pcnet.h"

pcnet_t* device = NULL;
pci_t pci_dev;
net_t iface;

int isPCNETInit = 0;

//TODO: If this doesnt work then change all the address from virtual to physical

void init_pcnet()
{
    pci_dev = pci_get_device(0x1022, 0x2000, -1);
    if(pci_dev.device_num != 0)
    {
        device = ZALLOC_TYPES(pcnet_t);

        uint32_t ret = pci_read(pci_dev, PCI_OFF_BAR0);
        device->portBase = pci_get_portBase(pci_dev);
        device->MAC0_port = PCNET_PIO_MAC_ADDRESS_0(device->portBase);
        device->MAC2_port = PCNET_PIO_MAC_ADDRESS_2(device->portBase);
        device->MAC4_port = PCNET_PIO_MAC_ADDRESS_4(device->portBase);
        device->RegData_port = PCNET_PIO_REGISTER_DATA(device->portBase);
        device->RegAddr_port = PCNET_PIO_REGISTER_ADDRESS(device->portBase);
        device->Reset_port = PCNET_PIO_RESET(device->portBase);
        device->BusControl_port = PCNET_PIO_BUS_CONTROL_REGISTER(device->portBase);
        
        pcnet_reset();

        uint64_t MAC0 = inw(device->MAC0_port) % 256;
        uint64_t MAC1 = inw(device->MAC0_port) / 256;

        uint64_t MAC2 = inw(device->MAC2_port) % 256;
        uint64_t MAC3 = inw(device->MAC2_port) / 256;

        uint64_t MAC4 = inw(device->MAC4_port) % 256;
        uint64_t MAC5 = inw(device->MAC4_port) / 256;

        uint64_t MAC = MAC5 << 40 | MAC4 << 32 | MAC3 << 24 | MAC2 << 16 | MAC1 << 8 | MAC0;

        outw(device->RegAddr_port, 20);
        outw(device->BusControl_port, 0x102);

        outw(device->RegAddr_port, 0);
        outw(device->RegData_port, 0x04);

        pcnet_initblock_t* iblock = &(device->initblock);

        iblock->mode = 0x0000;
        iblock->reserved1 = 0;
        iblock->nTxBuffers = 3;
        iblock->reserved2 = 0;
        iblock->nRxBuffers = 3;
        iblock->physical_address = MAC;
        iblock->reserved3 = 0;
        iblock->logical_address = 0;

        device->dRxBuf_physaddr = virt2phys(kernel_page_dir, &device->dRxBufferMem[0]);
        device->dTxBuf_physaddr = virt2phys(kernel_page_dir, &device->dTxBufferMem[0]);

        device->dTxBuffer = (pcnet_bufferDescriptor_t *)((((uint32_t)&device->dTxBufferMem[0]) + 15) & ~((uint32_t)0xF));
        iblock->TxBufferDescriptor = (uint32_t)virt2phys(kernel_page_dir, device->dTxBuffer);
        //iblock->TxBufferDescriptor = (uint32_t)(device->dTxBuffer);
        device->dRxBuffer = (pcnet_bufferDescriptor_t *)((((uint32_t)&device->dRxBufferMem[0]) + 15) & ~((uint32_t)0xF));
        iblock->RxBufferDescriptor = (uint32_t)virt2phys(kernel_page_dir, device->dRxBuffer);
        //iblock->RxBufferDescriptor = (uint32_t)device->dRxBuffer;

        for (uint8_t i = 0; i < 8; i++)
        {
            device->dTxBuffer[i].address = (((uint32_t)virt2phys(kernel_page_dir, &device->TxBuffers[i])) + 15) & ~(uint32_t)0xF;
            //device->dTxBuffer[i].address = (((uint32_t)&device->TxBuffers[i]) + 15) & ~(uint32_t)0xF;
            device->dTxBuffer[i].flags = 0x7FF | 0xF000;
            device->dTxBuffer[i].flags2 = 0;
            device->dTxBuffer[i].avail = 0;

            device->dRxBuffer[i].address = (((uint32_t)virt2phys(kernel_page_dir, &device->RxBuffers[i])) + 15) & ~(uint32_t)0xF;
            //device->dRxBuffer[i].address = (((uint32_t)&device->RxBuffers[i]) + 15) & ~(uint32_t)0xF;
            device->dRxBuffer[i].flags = 0xF7FF | 0x80000000;
            device->dRxBuffer[i].flags2 = 0;
            device->dRxBuffer[i].avail = 0;
        }

        uint32_t irq_num = pci_read(pci_dev, PCI_OFF_INTERRUPT_LINE);
        register_interrupt_handler(0x20+irq_num, pcnet_handler);
        ASM_FUNC("sti");

        outw(device->RegAddr_port, 1);
        outw(device->RegData_port, (uint32_t)virt2phys(kernel_page_dir ,&device->initblock) & 0xFFFF);
        //outw(device->RegData_port, ((uint32_t)(device->initblock)) & 0xFFFF);
        outw(device->RegAddr_port, 2);
        outw(device->RegData_port, ((uint32_t)virt2phys(kernel_page_dir ,&device->initblock) >> 16) & 0xFFFF);
        //outw(device->RegData_port, ((uint32_t)&device->initblock >> 16) & 0xFFFF);

        outw(device->RegAddr_port, 0);
        outw(device->RegData_port, 0x41);

        outw(device->RegAddr_port, 4);
        uint32_t temp = inw(device->RegData_port);
        outw(device->RegAddr_port, 4);
        outw(device->RegData_port, temp | 0xC00);

        outw(device->RegAddr_port, 0);
        outw(device->RegData_port, 0x42);

        iface.getip = pcnet_getIPAddress;
        iface.getmac = pcnet_getMACAddress;
        iface.setip = pcnet_setIPAddress;
        iface.send = pcnet_send;
        iface.IUID = AM79C973_IUID;
        register_network_interface(&iface);

        serialprintf("[PCNET] Successfully initialized\n[PCNET] MAC : %02x:%02x:%02x:%02x:%02x:%02x\n", MAC0, MAC1, MAC2, MAC3, MAC4, MAC5);
    }
    else
    {
        serialprintf("[PCNET] Cannot Initialize. AMD PCNet Fast III cannot be found in PCI Bus.\n");
    }

}

void pcnet_reset()
{
    if(device != NULL)
    {
        inw(device->Reset_port);
        outw(device->Reset_port, 0);
    }
}

void pcnet_handler(registers_t* regs)
{
    if(device != NULL )
    {
        serialprintf("[PCNET] Interrupt 0x%04x\n", regs->ino);
        outw(device->RegAddr_port, 0);
        uint32_t temp = inw(device->RegData_port);

        if((temp & 0x8000) == 0x8000)
            serialprintf("[PCNET:%x] Error occurred\n", temp);
        if((temp & 0x2000) == 0x2000)
            serialprintf("[PCNET:%x] Collision Error\n", temp);
        if((temp & 0x1000) == 0x1000)
            serialprintf("[PCNET:%x] Missed Frame\n", temp);
        if((temp & 0x0800) == 0x0800)
            serialprintf("[PCNET:%x] Memory Error\n", temp);
        if((temp & 0x0400) == 0x0400)
        {
            serialprintf("[PCNET:%x] Packet received\n", temp);
            pcnet_receive();
        }
        if((temp & 0x0200) == 0x0200)
            serialprintf("[PCNET:%x] Packet sent\n", temp);
        outw(device->RegAddr_port, 0);
        outw(device->RegData_port, temp);

        if((temp & 0x0100) == 0x0100)
        {
            serialprintf("[PCNET:%x] Initialization done\n", temp);
            isPCNETInit = 1;
        }
    }
}

void pcnet_send(void* data, uint32_t len)
{
    if(device != NULL && isPCNETInit == 1)
    {
        int TxDescriptor = device->currentTxBuffer;
        device->currentTxBuffer = (device->currentTxBuffer + 1) % 8;

        if(len > 1518)
            len = 1518;

        for (uint8_t *src = data + len - 1, *dst = (uint8_t *)(device->dTxBuffer[TxDescriptor].address + len - 1); src >= len; src--, dst--)
            *dst = *src;

        device->dTxBuffer[TxDescriptor].avail = 0;
        device->dTxBuffer[TxDescriptor].flags2 = 0;
        device->dTxBuffer[TxDescriptor].flags = 0x8300F000 | ((uint16_t)((-len) & 0xFFF));
        outw(device->RegAddr_port, 0);
        outw(device->RegData_port, 0x48);
    }
}

void pcnet_receive()
{
    for (; (device->dRxBuffer[device->currentRxBuffer].flags & 0x80000000) == 0; device->currentRxBuffer = (device->currentRxBuffer + 1) % 8)
    {
        if (!(device->dRxBuffer[device->currentRxBuffer].flags & 0x40000000) && (device->dRxBuffer[device->currentRxBuffer].flags & 0x03000000) == 0x03000000)
        {
            uint32_t size = device->dRxBuffer[device->currentRxBuffer].flags & 0xFFF;
            if (size > 64)
                size -= 4;

            uint8_t *buffer = (uint8_t *)(device->dRxBuffer[device->currentRxBuffer].address);

            interface_receive(buffer, size);
            pcnet_send(buffer, size);
        }

        device->dRxBuffer[device->currentRxBuffer].flags2 = 0;
        device->dRxBuffer[device->currentRxBuffer].flags = 0x8000F7FF;
    }
}

uint64_t pcnet_getMACAddress()
{
    return device->initblock.physical_address;
}

void pcnet_setIPAddress(uint32_t ip)
{
    device->initblock.logical_address = ip;
}

uint32_t pcnet_getIPAddress()
{
    return device->initblock.logical_address;
}