#include "net.h"
#include "ethernet.h"
#include "logdisk.h"

list_t* ninterface_list;
net_t* current_interface = NULL;

int isNetworkON = 0;

void register_network_interface(net_t* interface)
{
    list_insert_back(ninterface_list, interface);
}

void interface_receive(uint8_t* packet, uint32_t size)
{
    if(isNetworkON)
    {
        ethernet_handle_packet((etherframe_t*)packet, size);
    }
}

char* identify_interface(net_t* interface)
{
    switch(interface->IUID)
    {
    case AM79C973_IUID:
        return "AMD PCNET fast III";
        break;
    case RTL8139_IUID:
        return "Realtek RTL8139";
        break;
    default:
        return "Unknown";
        break;
    };
}

int isCurrentInterface(net_t* interface)
{
    if(interface == current_interface)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void print_interfaces()
{
    printf("%sInterfaces List:\n", ((isNetworkON == 1) ? "Network Interface is enabled\n" : "Network Interface is disabled\n"));
    serialprintf("%sInterfaces List:\n", ((isNetworkON == 1) ? "Network Interface is enabled\n" : "Network Interface is disabled\n"));
    int nif = 0;
    foreach(t, ninterface_list)
    {
        net_t* interface = t->val;
        printf("%02d: %s %s\n", nif, identify_interface(interface), isCurrentInterface(interface) == 1 ? "<" : "");
        serialprintf("%02d: %s %s\n", nif, identify_interface(interface), isCurrentInterface(interface) == 1 ? "<" : "");
        nif++;
    }
}

void init_networkInterfaceManager()
{
    ninterface_list = list_create();
    isNetworkON = 1;
    current_interface = NULL;
}

void switch_interface(uint32_t index)
{
    uint32_t numinterface = list_size(ninterface_list);
    if(index > (numinterface - 1) || index < 0 || numinterface == 0)
    {
        ldprintf("NIM", LOG_ERR, "Index %d out of bounds", index);
        return;
    }

    listnode_t* node = list_get_node_by_index(ninterface_list, index);
    net_t* interface = node->val;
    current_interface = interface;

    switch(interface->IUID)
    {
    case AM79C973_IUID:
        ldprintf("NIM", LOG_INFO, "AMD PCNET fast III is selected as the current network interface!");
        break;
    case RTL8139_IUID:
        ldprintf("NIM", LOG_INFO, "Realtek RTL8139 is selected as the current network interface!");
        break;
    default:
        ldprintf("NIM", LOG_WARN, "Unknown network device is used as the current network interface!");
        break;
    };
}

bool getInterfaceState()
{
    return (isNetworkON == 1);
}

void interface_send(uint8_t* buffer, int size)
{
    if(isNetworkON == 1)
    {
        if(current_interface == NULL) return;
        network_sent_t send = current_interface->send;

        send(buffer, size);
    }
    else
    {
        //printf("[Network interface manager] Network interface is disabled!\n");
    }
}

uint32_t interface_getip()
{
    if(isNetworkON == 1)
    {
        if(current_interface == NULL) return 0;
        network_getip_t getip = current_interface->getip;

        uint32_t ip = getip();
        return ip;
    }
    else
    {
        //printf("[Network interface manager] Network interface is disabled!\n");
        return 0;
    }
}

uint64_t interface_getmac()
{
    if(isNetworkON == 1)
    {
        if(current_interface == NULL) return 0;
        network_getmac_t getmac = current_interface->getmac;

        uint64_t mac = getmac();
        return mac;
    }
    else
    {
        //printf("[Network interface manager] Network interface is disabled!\n");
        return 0;
    }
}

void interface_setip(uint32_t ip)
{
    if(isNetworkON == 1)
    {
        if(current_interface == NULL) return;
        network_setip_t setip = current_interface->setip;
        setip(ip);
    }
    else
    {
        //printf("[Network interface manager] Network interface is disabled!\n");
    }
}

void changeNetworkState(bool state)
{
    (state == false) ? (isNetworkON = 0) : (isNetworkON = 1);
    if(isNetworkON == 0)
    {
        ldprintf("NIM", LOG_INFO, "Network interface is disabled!");
    }
    else if(isNetworkON == 1)
    {
        ldprintf("NIM", LOG_INFO, "Network interface is enabled!");
    }
}