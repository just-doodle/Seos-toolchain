#include "tcp.h"
#include "logdisk.h"

TCPSocket_t* t_sockets[TCP_MAX_SOCKETS];
uint16_t numSockets = 0;
uint16_t freePorts = TCP_RESERVED_PORTS+1;
void debug_tcp_print_header(TCPHeader_t* header)
{
    ldprintf("TCP", LOG_DEBUG, "SPort: %d", ntohs(header->srcPort));
    ldprintf("TCP", LOG_DEBUG, "DPort: %d", ntohs(header->dstPort));
    ldprintf("TCP", LOG_DEBUG, "seqnum: %d", header->seqNum);
    ldprintf("TCP", LOG_DEBUG, "ackNum: %d", header->ackNum);
    ldprintf("TCP", LOG_DEBUG, "flags: 0x%04x", header->flags);
    ldprintf("TCP", LOG_DEBUG, "%s|%s|%s|%s|%s", ibit(header->flags, 1) ? "F" : "f", ibit(header->flags, 2) ? "S" : "s", ibit(header->flags, 3) ? "R" : "r", ibit(header->flags, 4) ? "P" : "p", ibit(header->flags, 5) ? "A" : "a");
}

char* get_socketState(TCPSocket_t* socket)
{
    switch(socket->state)
    {
    case CLOSED:
        return "Closed";
    case LISTEN:
        return "Listening";
    case SYN_SENT:
        return "SYN sent";
    case SYN_RECEIVED:
        return "SYN received";
    case ESTABLISHED:
        return "Connection established";
    case FIN_WAIT1:
        return "Waiting for FIN 1";
    case FIN_WAIT2:
        return "Waiting for FIN 2";
    case CLOSING:
        return "Closing the socket";
    case TIME_WAIT:
        return "Time Wait";
    case CLOSE_WAIT:
        return "Waiting for closure";
    case LAST_ACK:
        return "Last ACK";
    };
}

uint32_t prev_seq = 0;
TCPSocket_t* prev_socket = 0;

void tcp_handle_packet(uint8_t* packet, uint8_t* src_addr, uint8_t* dst_addr, uint32_t len)
{
    if(len < 20)
        return;

    TCPHeader_t* h = (TCPHeader_t*)packet;

    uint16_t localPort = ntohs(h->dstPort);
    uint16_t remotePort = ntohs(h->srcPort);

    debug_tcp_print_header(h);
    uint8_t f_ntohl = h->flags;

    // if(prev_socket != 0)
    // {
    //     if(h->seqNum == prev_seq)
    //     {
    //         ldprintf("TCP", LOG_DEBUG, "RETRANSMISSION\n");
    //         prev_socket->state = LISTEN;
    //     }
    // }

    TCPSocket_t* socket = 0;
    for(uint16_t i = 0; i < numSockets; i++)
    {
        if(socket == 0 && i != 0)
            break;
        if (t_sockets[i]->localPort == h->dstPort && (t_sockets[i]->localIP) == ntohl(ipatoh(dst_addr)) && t_sockets[i]->state == LISTEN && (((f_ntohl) & (SYN | ACK)) == SYN))
        {
            socket = (t_sockets[i]);
        }
        else if (t_sockets[i]->localPort == h->dstPort && (t_sockets[i]->localIP) == ntohl(ipatoh(dst_addr)) && t_sockets[i]->remPort == h->srcPort && t_sockets[i]->remIP == ipatoh(src_addr))
        {
            socket = (t_sockets[i]);
        }
        ldprintf("TCP", LOG_DEBUG, "%d ||| lp:%d == dp:%d ||| lip:0x%x == dip:0x%x ||| rp:%d == sp:%d ||| rip:0x%x == sip:0x%x\n", i, \
        t_sockets[i]->localPort, h->dstPort, t_sockets[i]->localIP, ipatoh(dst_addr), t_sockets[i]->remPort, h->srcPort, t_sockets[i]->remIP, ipatoh(src_addr));
    }

    int reset = 0;

    prev_seq = h->seqNum;
    prev_socket = socket;

    ldprintf("TCP", LOG_DEBUG, "Socket %d state: %s", ntohs(socket->localPort), get_socketState(socket));

    if(socket != 0 && (f_ntohl & RST))
        socket->state = CLOSED;
    
    if(socket != 0 && socket->state != CLOSED)
    {
        switch((f_ntohl) & (SYN | ACK | FIN))
        {
        case SYN:
            if(socket->state == LISTEN)
            {
                //ldprintf("TCP", LOG_DEBUG, "SYN\n");
                socket->state = SYN_RECEIVED;
                socket->remPort = h->srcPort;
                socket->remIP = ipatoh(src_addr);
                socket->ackNum = ntohl(h->seqNum)+1;
                socket->seqNum = 0xbeefcafe;
                //ldprintf("TCP", LOG_DEBUG, "END_SYN\n");
                tcp_send(socket, 0, 0, SYN | ACK);
                socket->seqNum++;
            }
            else
            {
                ldprintf("TCP", LOG_DEBUG, "SYN RESET");
                reset = true;
            }
            break;
        case SYN | ACK:
            if (socket->state == SYN_SENT)
            {
                socket->state = ESTABLISHED;
                socket->ackNum = ntohl(h->seqNum) + 1;
                socket->seqNum++;
                tcp_send(socket, 0, 0, ACK);
            }
            else
            {
                ldprintf("TCP", LOG_DEBUG, "SYN ACK RESET");
                reset = true;
            }
            break;

        case SYN | FIN:
        case SYN | FIN | ACK:
            ldprintf("TCP", LOG_DEBUG, "SYN FIN ACK Reset");
            reset = true;
            break;

        case FIN:
        case FIN | ACK:
            if (socket->state == ESTABLISHED)
            {
                socket->state = CLOSE_WAIT;
                socket->ackNum++;
                tcp_send(socket, 0, 0, ACK);
                tcp_send(socket, 0, 0, FIN | ACK);
            }
            else if (socket->state == CLOSE_WAIT)
            {
                socket->state = CLOSED;
            }
            else if (socket->state == FIN_WAIT1 || socket->state == FIN_WAIT2)
            {
                socket->state = CLOSED;
                socket->ackNum++;
                if(socket->keep_listening == 1)
                    socket->state = LISTEN;
                tcp_send(socket, 0, 0, ACK);
            }
            else
            {
                ldprintf("TCP", LOG_DEBUG, "FIN ACK RESET");
                reset = true;
            }
            break;

        case ACK:
            if (socket->state == SYN_RECEIVED)
            {
                ldprintf("TCP", LOG_DEBUG, "Connection established");
                socket->state = ESTABLISHED;
                socket->ackNum = htonl(h->seqNum);
            }
            else if (socket->state == FIN_WAIT1)
            {
                socket->state = FIN_WAIT2;
            }
            else if (socket->state == CLOSE_WAIT)
            {
                socket->state = CLOSED;
                break;
            }

            if (f_ntohl == ACK)
                break;

        default:
            ldprintf("TCP", LOG_DEBUG, "%d == %d", ntohl(h->seqNum), socket->ackNum);
            if (ntohl(h->seqNum) == socket->ackNum)
            {
                ldprintf("TCP", LOG_DEBUG, "Handling packet");
                reset = !(socket->handler(socket, packet + h->headerSize32 * 4, len - h->headerSize32 * 4));
                if (!reset)
                {
                    int x = 0;
                    for (int i = h->headerSize32 * 4; i < len; i++)
                        if (packet[i] != 0)
                            x = i;
                    socket->ackNum += x - h->headerSize32 * 4 + 1;
                    tcp_send(socket, 0, 0, ACK);
                }
            }
            else
            {
                ldprintf("TCP", LOG_ERR, "Data in wrong order");
                reset = true;
            }
        }
    }

    if (reset)
    {
        if (socket != 0)
        {
            tcp_send(socket, 0, 0, RST);
        }
        else
        {
            TCPSocket_t socket2;
            socket2.state = CLOSED;
            socket2.remPort = h->srcPort;
            socket2.remIP = ipatoh(src_addr);
            socket2.localPort = h->dstPort;
            socket2.localIP = ipatoh(dst_addr);
            socket2.seqNum = ntohl(h->ackNum);
            socket2.ackNum = ntohl(h->seqNum) + 1;
            tcp_send(&socket2, 0, 0, RST);
        }
    }

    if (socket != 0 && socket->state == CLOSED)
        for (uint16_t i = 0; i < numSockets && socket == 0; i++)
            if ((t_sockets[i]) == (socket))
            {
                t_sockets[i] = t_sockets[--numSockets];
                free(socket);
                break;
            }

    return false;
}

void tcpsocket_send(TCPSocket_t* socket, uint8_t* buffer, uint16_t len)
{
    tcp_send(socket, buffer, len, PSH | ACK);
}

void tcp_send(TCPSocket_t* socket, uint8_t* data, uint16_t len, uint16_t flags)
{
    uint16_t totalLength = len + sizeof(TCPHeader_t);
    uint16_t lengthInclPHdr = totalLength + sizeof(TCPPseudoHeader_t);

    uint8_t *buffer = (uint8_t *)zalloc(lengthInclPHdr);

    TCPPseudoHeader_t* phdr = (TCPPseudoHeader_t*)buffer;
    TCPHeader_t* msg = (TCPHeader_t*)(buffer + sizeof(TCPPseudoHeader_t));

    uint8_t* b2 = buffer + (sizeof(TCPPseudoHeader_t) + sizeof(TCPHeader_t));

    msg->headerSize32 = sizeof(TCPHeader_t) / 4;

    msg->srcPort = socket->localPort;
    msg->dstPort = socket->remPort;

    if((flags & ACK) != 0)
        msg->ackNum = htonl(socket->ackNum);
    msg->seqNum = htonl(socket->seqNum);

    msg->reserved = 0;

    msg->flags = flags;

    msg->window_size = 0xFFFF;
    msg->urgent = 0;

    msg->options = ((flags & SYN) != 0) ? 0xB4050402 : 0;

    socket->seqNum += len;

    for (int i = 0; i < len; i++)
        b2[i] = data[i];

    phdr->srcIP = htonl(socket->localIP);
    phdr->dstIP = (socket->remIP);
    phdr->reserved = 0;
    phdr->protocol = PROTOCOL_TCP;
    phdr->total_length = htons(totalLength);

    msg->checksum = 0;
    msg->checksum = htons(calculate_checksum(buffer, lengthInclPHdr));

    uint32_t rip = (socket->remIP);

    ip_send_packet(((uint8_t*)&(rip)), (uint8_t *)msg, totalLength, PROTOCOL_TCP);
    free(buffer);
}

TCPSocket_t* tcp_connect(uint8_t* addr, uint16_t port)
{
    TCPSocket_t *socket = (TCPSocket_t *)zalloc(sizeof(TCPSocket_t));

    if (socket != 0)
    {
        socket->remPort = (port);
        socket->remIP = ipatoh(addr);
        socket->localPort = freePorts++;
        socket->localIP = interface_getip() & 0xFFFFFFFF;

        socket->remPort = ((socket->remPort & 0xFF00) >> 8) | ((socket->remPort & 0x00FF) << 8);
        socket->localPort = ((socket->localPort & 0xFF00) >> 8) | ((socket->localPort & 0x00FF) << 8);

        printf("REMPORT: %d\n", socket->remPort);

        t_sockets[numSockets++] = socket;
        socket->state = SYN_SENT;

        socket->seqNum = 0xbeefcafe;

        tcp_send(socket, 0, 0, SYN);

        return socket;
    }
}

TCPSocket_t* tcp_listen(uint16_t port)
{
    TCPSocket_t* socket = ZALLOC_TYPES(TCPSocket_t);
    if (socket != 0)
    {

        socket->state = LISTEN;
        socket->localIP = interface_getip();
        socket->localPort = ((port & 0xFF00) >> 8) | ((port & 0x00FF) << 8);

        printf("LOCALPORT: %d\n", socket->localPort);

        t_sockets[numSockets++] = socket;

        return socket;
    }
}

void tcp_disconnect(TCPSocket_t* socket)
{
    socket->state = FIN_WAIT1;
    tcp_send(socket, 0, 0, FIN + ACK);
    socket->seqNum++;
}

void tcp_bind(TCPSocket_t* socket, TCPHandleTransmission_t handler)
{
    if(socket != NULL)
        socket->handler = handler;
    ldprintf("TCP", LOG_DEBUG, "BINDED");
}

void init_tcp()
{
    for (int i = 0; i < 65535; i++)
        t_sockets[i] = 0;
    numSockets = 0;
    freePorts = TCP_RESERVED_PORTS+1;
    register_ipv4_protocol(tcp_handle_packet, "UDP", PROTOCOL_TCP);
}
