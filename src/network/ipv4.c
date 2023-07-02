#include "ipv4.h"
#include "logdisk.h"

// #include "udp.h"
// #include "icmp.h"

list_t* protocols = NULL;

uint8_t my_ip[] = {10, 0, 2, 14};
uint8_t test_target_ip[] = {10, 0, 2, 15};
uint8_t zero_hardware_addr[] = {0,0,0,0,0,0};

uint8_t gateway_ip[4];
uint8_t subnet_mask[4];

uint32_t gatewayl = 0;
uint32_t subnetl = 0; 

void get_ip_str(char* ip_str, uint8_t* ip)
{
    sprintf(ip_str, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
}

char* ip_to_string(uint8_t* ip)
{
    static char ip_str[16];
    sprintf(ip_str, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    return ip_str;
}

void set_gatewayAndMask(uint8_t gateway[4], uint8_t subnet[4])
{
    if(gateway != NULL)
        memcpy(gateway_ip, gateway, 4);

    if(subnet != NULL)
        memcpy(subnet_mask, subnet, 4);

    gatewayl = *((uint32_t*)gateway_ip);
    subnetl = *((uint32_t*)subnet_mask);
}

uint16_t ip_checksum(ip_packet_t* packet)
{
    int array_size = sizeof(ip_packet_t) / 2;

    uint16_t * array = (uint16_t*)packet;
    uint8_t * array2 = (uint8_t*)packet;

    uint32_t sum = 0;

    for(int i = 0; i < array_size; i++)
    {
        sum += flip_short(array[i]);
    }

    if(array_size % 2 == 1)
    {
        sum += flip_short((uint16_t)(((uint8_t*)(array))[array_size-1]));
    }

    uint32_t carry = sum >> 16;

    sum = sum & 0x0000ffff;
    sum = sum + carry;

    return ~sum;
}

void ip_send_packet(uint8_t* dst_ip, void* data, int len, uint8_t protocol)
{
    //serialprintf("[IPv4] dst: %d.%d.%d.%d\n", dst_ip[0], dst_ip[1], dst_ip[2], dst_ip[3]);
    int arp_sent = 3;
    ip_packet_t * packet = kmalloc(sizeof(ip_packet_t) + len);
    memset(packet, 0, sizeof(ip_packet_t));

    packet->version = IP_VERSION;
    packet->ihl = 5;
    packet->tos = 0;
    packet->length = sizeof(ip_packet_t) + len;
    packet->id = 0;
    packet->flags = 0;
    packet->fragment_offset_high = 0;
    packet->fragment_offset_low = 0;
    packet->ttl = 64;
    packet->protocol = protocol;

    get_ip2(packet->src_ip);
    memcpy(packet->dst_ip, dst_ip, 4);

    void * packet_data = (void*)packet + packet->ihl * 4;
    memcpy(packet_data, data, len);

    *((uint8_t*)(&packet->version_ihl_ptr)) = htonb(*((uint8_t*)(&packet->version_ihl_ptr)), 4);
    *((uint8_t*)(packet->flags_fragment_ptr)) = htonb(*((uint8_t*)(packet->flags_fragment_ptr)), 3);
    packet->length = htons(sizeof(ip_packet_t) + len);

    packet->checksum = 0;
    packet->checksum = htons(ip_checksum(packet));

    uint8_t dst_hardware_addr[6];

    uint32_t route = ipatoh(dst_ip);
    if((ipatoh(dst_ip) & subnetl) != (ipatoh(packet->src_ip) & subnetl))
        route = gatewayl;

    while(!arp_lookup(dst_hardware_addr, ((uint8_t*)&route)))
    {
        if(arp_sent != 0)
        {
            arp_sent--;
            arp_send_packet(zero_hardware_addr, ((uint8_t*)&route));
        }
        else
        {
            ldprintf("IPv4", LOG_ERR, "NO ARP RECORD of %d.%d.%d.%d\n", ((uint8_t*)&route)[0], ((uint8_t*)&route)[1], ((uint8_t*)&route)[2], ((uint8_t*)&route)[3]);
            break;
        }
    }
    // serialprintf("[IPv4] SENT\n");
    ethernet_send(dst_hardware_addr, packet, htons(packet->length), ETHERNET_TYPE_IP);
}

ipv4_protocol_info_t* ipv4_get_protocol(int protocol)
{
    ipv4_protocol_info_t* pinfo = NULL;
    foreach(l, protocols)
    {
        pinfo = l->val;
        if(pinfo->protocol == protocol)
            return pinfo;
    }

    return NULL;
}

void ip_handle_packet(ip_packet_t* packet)
{
    *((uint8_t*)(&packet->version_ihl_ptr)) = ntohb(*((uint8_t*)(&packet->version_ihl_ptr)), 4);
    *((uint8_t*)(packet->flags_fragment_ptr)) = ntohb(*((uint8_t*)(packet->flags_fragment_ptr)), 3);

    char src_ip[20];

    if(packet->version == IP_VERSION)
    {
        uint8_t src_addr[4];
        src_addr[0] = packet->src_ip[0];
        src_addr[1] = packet->src_ip[1];
        src_addr[2] = packet->src_ip[2];
        src_addr[3] = packet->src_ip[3];

        uint8_t dst_addr[4];
        dst_addr[0] = packet->dst_ip[0];
        dst_addr[1] = packet->dst_ip[1];
        dst_addr[2] = packet->dst_ip[2];
        dst_addr[3] = packet->dst_ip[3];

        void * data_ptr = (void*)packet + packet->ihl * 4;
        int data_len = ntohs(packet->length) - sizeof(ip_packet_t);

        ipv4_protocol_info_t* pinfo = ipv4_get_protocol(packet->protocol);
        if(pinfo == NULL)
        {
            ldprintf("IPv4", LOG_DEBUG, "Unknown protocol %d", packet->protocol);
            return;
        }

        pinfo->handle_packet(data_ptr, src_addr, dst_addr, data_len);
    }
    else
    {
        ldprintf("IPv4", LOG_ERR, "Unsupported IP version: %d\n", packet->version);
    }

    return;
}

void register_ipv4_protocol(ipv4_handle_packet handler, char name[8], int protocol)
{
    if(protocols == NULL)
        protocols = list_create();

    ipv4_protocol_info_t* pinfo = ZALLOC_TYPES(ipv4_protocol_info_t);
    pinfo->handle_packet = handler;
    pinfo->protocol = protocol;
    strcpy(pinfo->name, name);

    pinfo->self = list_insert_front(protocols, pinfo);
    ldprintf("IPv4", LOG_DEBUG, "New protocol over ipv4 named %s has been registered", name);
}