#ifndef __TCP_H__
#define __TCP_H__

#include "system.h"
#include "kheap.h"
#include "string.h"
#include "net.h"
#include "netutils.h"
#include "ipv4.h"

#define TCP_MAX_SOCKETS 0xffff
#define TCP_RESERVED_PORTS 1024

#define TCP_WINDOW_SIZE 0xFFFF

#define TCP_SEQUENCE_NUMBER 0xBEEFCAFE


//TODO: Make a working implementation of TCP

enum TCPSocketState
{
    CLOSED,
    LISTEN,
    SYN_SENT,
    SYN_RECEIVED,
    ESTABLISHED,
    FIN_WAIT1,
    FIN_WAIT2,
    CLOSING,
    TIME_WAIT,
    CLOSE_WAIT,
    LAST_ACK
};

enum TCPFlags
{
    FIN = 1,
    SYN = 2,
    RST = 4,
    PSH = 8,
    ACK = 16,
    URG = 32,
    ECE = 64,
    CWR = 128,
    NS  = 256
};

typedef struct TCPHeader_struct
{
    uint16_t srcPort;
    uint16_t dstPort;

    uint32_t seqNum;
    uint32_t ackNum;

    uint8_t reserved : 4;
    uint8_t headerSize32 : 4;
    uint8_t flags;

    uint16_t window_size;
    uint16_t checksum;
    uint16_t urgent;

    uint32_t options;
}__attribute__((packed)) TCPHeader_t;

typedef struct TCPPseudoHeader_struct
{
    uint32_t srcIP;
    uint32_t dstIP;
    uint8_t reserved;
    uint8_t protocol;
    uint16_t total_length;
}__attribute__((packed)) TCPPseudoHeader_t;

struct TCPSocket_struct;

typedef int (*TCPHandleTransmission_t)(struct TCPSocket_struct* socket, uint8_t* data, uint16_t len);

typedef struct TCPSocket_struct
{
    uint16_t remPort;           // Remote port
    uint16_t localPort;         // Local port

    uint32_t remIP;             // Remote IP address
    uint32_t localIP;           // Local IP address

    uint32_t seqNum;            // Sequence number
    uint32_t ackNum;            // Acknowledge number

    int keep_listening;

    enum TCPSocketState state;
    TCPHandleTransmission_t handler;
}TCPSocket_t;

TCPSocket_t* tcp_listen(uint16_t port);
TCPSocket_t* tcp_connect(uint8_t* addr, uint16_t port);
void tcp_disconnect(TCPSocket_t* socket);

void tcp_send(TCPSocket_t* socket, uint8_t* data, uint16_t len, uint16_t flags);
void tcpsocket_send(TCPSocket_t* socket, uint8_t* data, uint16_t len);

void tcp_bind(TCPSocket_t* socket, TCPHandleTransmission_t handler);

void tcp_handle_packet(uint8_t* packet, uint8_t* src_addr, uint8_t* dst_addr, uint32_t len);

void init_tcp();

#endif /*__TCP_H__*/