#include "loopback.h"
#include "ethernet.h"

loopback_t lp;

void loopback_send(void* data, uint32_t len)
{
    interface_receive(data, len);
}

uint32_t loopback_getIP()
{
    return lp.ip_addr;
}

void loopback_setIP(uint32_t ip)
{
    lp.ip_addr = ip;
}

uint64_t loopback_getMAC()
{
    return 0;
}

void init_loopback()
{
    lp.dev = ZALLOC_TYPES(net_t);
    lp.dev->getip = loopback_getIP;
    lp.dev->getmac = loopback_getMAC;
    lp.dev->send = loopback_send;
    lp.dev->setip = loopback_setIP;

    lp.dev->IUID = LOOPBACK_IUID;

    lp.ip_addr = (0x0100007F);

    register_network_interface(lp.dev);
}