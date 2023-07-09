#include "system.h"
#include "modules.h"

#include "netutils.h"
#include "ipv4.h"
#include "kheap.h"

#define ICMP_TYPE_ECHO_REPLY            0
#define ICMP_TYPE_DEST_UNREACHABLE      3
#define ICMP_TYPE_SOUCE_QUENCH          4
#define ICMP_TYPE_REDIRECT_MSG          5
#define ICMP_TYPE_ECHO_REQUEST          8
#define ICMP_TYPE_ROUTER_ADVERTISEMENT  9
#define ICMP_TYPE_ROUTER_SOLICITATION   10
#define ICMP_TYPE_TIME_EXCEEDED         11
#define ICMP_TYPE_BAD_PARAM             12
#define ICMP_TYPE_TIMESTAMP             13
#define ICMP_TYPE_TIMESTAMP_REPLY       14
#define ICMP_TYPE_INFO_REQUEST          15
#define ICMP_TYPE_INFO_REPLY            16
#define ICMP_TYPE_ADDR_MASK_REQUEST     17
#define ICMP_TYPE_ADDR_MASK_REPLY       18
#define ICMP_TYPE_TRACEROUTE            30

typedef struct icmp_packet
{
    uint8_t type;
    uint8_t code;
    uint16_t checksum;

    uint16_t id;
    uint16_t seq;
} __attribute__((packed)) icmp_packet_t;

static int isReplied = 0;

void request_echo_reply(uint8_t* ip)
{
    icmp_packet_t* packet = (icmp_packet_t*)kmalloc(64);
    memset(packet, 0, 64);
    packet->type = ICMP_TYPE_ECHO_REQUEST;
    packet->code = 0;
    packet->checksum = 0;
    packet->id = 8913;
    packet->seq = 5454;
    packet->checksum = calculate_checksum((uint16_t*)packet, 64);
    ip_send_packet(ip, packet, 64, PROTOCOL_ICMP);
}

void icmp_echo_reply(icmp_packet_t* packet, uint8_t* src_ip, uint16_t echo_id, uint16_t echo_seq)
{
    icmp_packet_t* reply = (icmp_packet_t*)kmalloc(64);
    memset(packet, 0, 64);
    printf("[ICMP] Echo reply to %d.%d.%d.%d\n", src_ip[0], src_ip[1], src_ip[2], src_ip[3]);
    reply->type = ICMP_TYPE_ECHO_REPLY;
    reply->code = 0;
    reply->checksum = 0;
    reply->id = echo_id;
    reply->seq = echo_seq;
    reply->checksum = calculate_checksum((uint16_t*)reply, 64);
    ip_send_packet(src_ip, reply, 64, PROTOCOL_ICMP);
}

static int icmp_handle_packet(uint8_t* packet, uint8_t* src, uint8_t* dst, uint32_t len)
{
    icmp_packet_t* p = (icmp_packet_t*)packet;

    switch(p->type)
    {
        case ICMP_TYPE_ECHO_REQUEST:
            if(isReplied == 0)
            {
                ldprintf("ICMP", LOG_DEBUG, "Echo request from %d.%d.%d.%d", src[0], src[1], src[2], src[3]);
                icmp_echo_reply(p, src, p->id, p->seq);
                isReplied = 100;
            }
            else
            {
                isReplied--;
            }
            break;
        case ICMP_TYPE_ECHO_REPLY:
            ldprintf("ICMP", LOG_DEBUG, "Echo reply from %d.%d.%d.%d", src[0], src[1], src[2], src[3]);
            break;
        case ICMP_TYPE_DEST_UNREACHABLE:
            ldprintf("ICMP", LOG_DEBUG, "Destination unreachable from %d.%d.%d.%d", src[0], src[1], src[2], src[3]);
            break;
        case ICMP_TYPE_TIME_EXCEEDED:
            ldprintf("ICMP", LOG_DEBUG, "Time exceeded from %d.%d.%d.%d", src[0], src[1], src[2], src[3]);
            break;
        case ICMP_TYPE_BAD_PARAM:
            ldprintf("ICMP", LOG_DEBUG, "Parameter problem from %d.%d.%d.%d", src[0], src[1], src[2], src[3]);
            break;
        case ICMP_TYPE_ROUTER_ADVERTISEMENT:
            ldprintf("ICMP", LOG_DEBUG, "Router advertisement from %d.%d.%d.%d", src[0], src[1], src[2], src[3]);
            break;
        default:
            ldprintf("ICMP", LOG_DEBUG, "type %d", p->type);
            break;
    }
}

static int init(int argc, char** argv)
{
    uint8_t ip[4] = {127, 0, 0, 1};
    register_ipv4_protocol(icmp_handle_packet, "ICMP", PROTOCOL_ICMP);
    request_echo_reply(ip);
    ldprintf("ICMP", LOG_INFO, "ICMP protocol handler installed");
    return 0;
}

static int fini()
{
    return 0;
}

MODULE_DEF(ICMP, init, fini);