#include "pci.h"


uint32_t pci_size_map[100];
pci_t dev_zero = {0};

uint32_t pci_read(pci_t dev, uint32_t field)
{
    dev.field_num = (field & 0xFC) >> 2;
	dev.enable = 1;
	outl(PCI_CONFIG_PORT, dev.bits);

	uint32_t size = pci_size_map[field];
	if(size == 1)
    {
		uint8_t t = inb(PCI_DATA_PORT + (field & 3));
		return t;
	}
	else if(size == 2)
    {
		uint16_t t = inw(PCI_DATA_PORT + (field & 2));
		return t;
	}
	else if(size == 4)
    {
		uint32_t t = inl(PCI_DATA_PORT);
		return t;
	}
	return 0xffff;
}

void pci_write(pci_t dev, uint32_t field, uint32_t value)
{
    dev.field_num = (field & 0xFC) >> 2;
	dev.enable = 1;
	outl(PCI_CONFIG_PORT, dev.bits);
	outl(PCI_DATA_PORT, value);
}

uint32_t get_device_type(pci_t dev)
{
    uint32_t t = pci_read(dev, PCI_OFF_CLASS) << 8;
	return t | pci_read(dev, PCI_OFF_SUBCLASS);
}

uint32_t get_secondary_bus(pci_t dev)
{
    return pci_read(dev, PCI_OFF_SECONDARY_BUS);
}

uint32_t pci_reach_end(pci_t dev)
{
    uint32_t t = pci_read(dev, PCI_OFF_HEADER_TYPE);
	return !t;
}

pci_t pci_scan_function(uint16_t vendor_id, uint16_t device_id, uint32_t bus, uint32_t device, uint32_t function, int device_type)
{
    pci_t dev = {0};
	dev.bus_num = bus;
	dev.device_num = device;
	dev.function_num = function;

	if(get_device_type(dev) == PCI_TYPE_BRIDGE)
    {
		pci_scan_bus(vendor_id, device_id, get_secondary_bus(dev), device_type);
	}

	if(device_type == -1 || device_type == get_device_type(dev))
    {
		uint32_t devid  = pci_read(dev, PCI_OFF_DEVICE_ID);
		uint32_t vendid = pci_read(dev, PCI_OFF_VENDOR_ID);
		if(devid == device_id && vendor_id == vendid)
			return dev;
	}
	return dev_zero;
}

pci_t pci_scan_device(uint16_t vendor_id, uint16_t device_id, uint32_t bus, uint32_t device, int device_type)
{
    pci_t dev = {0};
	dev.bus_num = bus;
	dev.device_num = device;

	if(pci_read(dev,PCI_OFF_VENDOR_ID) == PCI_NONE)
		return dev_zero;

	pci_t t = pci_scan_function(vendor_id, device_id, bus, device, 0, device_type);
	if(t.bits)
		return t;

	if(pci_reach_end(dev))
		return dev_zero;

	for(int function = 1; function < FUNC_PER_DEV; function++)
    {
		if(pci_read(dev,PCI_OFF_VENDOR_ID) != PCI_NONE)
        {
			t = pci_scan_function(vendor_id, device_id, bus, device, function, device_type);
			if(t.bits)
				return t;
		}
	}
	return dev_zero;
}

pci_t pci_scan_bus(uint16_t vendor_id, uint16_t device_id, uint32_t bus, int device_type)
{
    for(int device = 0; device < DEV_PER_BUS; device++)
    {
		pci_t t = pci_scan_device(vendor_id, device_id, bus, device, device_type);
		if(t.bits)
			return t;
	}
	return dev_zero;
}

pci_t pci_get_device(uint16_t vendor_id, uint16_t device_id, int device_type)
{
    pci_t t = pci_scan_bus(vendor_id, device_id, 0, device_type);
	if(t.bits)
		return t;

	if(pci_reach_end(dev_zero))
    {
		printf("PCI device not found\n");
	}
	for(int function = 1; function < FUNC_PER_DEV; function++)
    {
		pci_t dev = {0};
		dev.function_num = function;

		if(pci_read(dev, PCI_OFF_VENDOR_ID) == PCI_NONE)
			break;
		t = pci_scan_bus(vendor_id, device_id, function, device_type);
		if(t.bits)
			return t;
	}
	return dev_zero;
}

bool DeviceHasFunctions(pci_t dev)
{
	return pci_read(dev, PCI_OFF_HEADER_TYPE) & (1<<7);
}

void identify_pci_device(pci_t dev)
{
	switch(pci_read(dev, PCI_OFF_CLASS))
	{
		case 0x01:
		{
			printf("Storage "); // Mass Storage Controller
		}break;
		case 0x02:
		{
			printf("Network "); // Network Controller
		}break;
		case 0x03:
		{
			printf("Graphics "); // Video Controller
			switch(pci_read(dev, PCI_OFF_SUBCLASS))
			{
				case 0x00:
					printf("VGA ");
					break;
				default:
					break;
			};
		}break;
		case 0x04:
		{
			printf("Multimedia "); // Audio Controller
		}break;
		case 0x05:
		{
			printf("Memory "); // Memory Controller
		}break;
		case 0x06:
		{
			printf("Bridge "); // Bridge Device
		}break;
	};

	switch(pci_read(dev, PCI_OFF_VENDOR_ID))
	{
		case 0x1022: // AMD
		{
			printf("AMD ");
			switch (pci_read(dev, PCI_OFF_DEVICE_ID))
			{
				case 0x2000:
					printf("PCnet-FAST III ");
					break;
				default:
					break;
			};
		}break;
		case 0x8086: // Intel
		{
			printf("Intel ");
			switch(pci_read(dev, PCI_OFF_DEVICE_ID))
			{
				case 0x100E:
					printf("82579LM Gigabit Ethernet ");
					break;
				case 0x1237:
					printf("440FX - 82441FX PMC ");
					break;
				case 0x7000:
					printf("82371SB PIIX3 ISA ");
					break;
				case 0x7010:
				{
					printf("82371SB PIIX3 IDE ");
				}break;
				default:
					break;
			};
		}break;
		case 0x1AF4: // Red Hat
		{
			printf("Red Hat ");
			switch(pci_read(dev, PCI_OFF_DEVICE_ID))
			{
				case 0x1100:
					printf("QEMU Virtual Machine ");
					break;
				default:
					break;
			};
		}break;
		case 0x10EC: // Realtek
		{
			printf("Realtek ");
			switch(pci_read(dev, PCI_OFF_DEVICE_ID))
			{
				case 0x8139:
					printf("RTL8139 ");
					break;
				default:
					break;
			};
		}break;
		default:
			break;
	};
	printf("\n");
}

void enumerate_pci_devices()
{
	for(int bus = 0; bus < BUS_PER_DOMAIN; bus++)
	{
		for(int device = 0; device < DEV_PER_BUS; device++)
		{
			pci_t dev = {0};
			dev.bus_num = bus;
			dev.device_num = device;
			int numFunctions = DeviceHasFunctions(dev) ? 8 : 1;

			for(int function = 0; function < numFunctions; function++)
			{
				dev.function_num = function;
				uint32_t devid  = pci_read(dev, PCI_OFF_DEVICE_ID);
				uint32_t vendid = pci_read(dev, PCI_OFF_VENDOR_ID);
				if(devid == PCI_NONE)
					break;
				printf("{%x, %x, %x, %x, %x} ", bus, device, function, vendid, devid);
				identify_pci_device(dev);
			}
		}
	}
}


void init_pci()
{
    pci_size_map[PCI_OFF_VENDOR_ID] = 2;
	pci_size_map[PCI_OFF_DEVICE_ID] = 2;
	pci_size_map[PCI_OFF_COMMAND] = 2;
	pci_size_map[PCI_OFF_STATUS] = 2;
	pci_size_map[PCI_OFF_SUBCLASS] = 1;
	pci_size_map[PCI_OFF_CLASS] = 1;
	pci_size_map[PCI_OFF_CACHE_LINE_SIZE] = 1;
	pci_size_map[PCI_OFF_LATENCY_TIMER]	= 1;
	pci_size_map[PCI_OFF_HEADER_TYPE] = 1;
	pci_size_map[PCI_OFF_BIST] = 1;
	pci_size_map[PCI_OFF_BAR0] = 4;
	pci_size_map[PCI_OFF_BAR1] = 4;
	pci_size_map[PCI_OFF_BAR2] = 4;
	pci_size_map[PCI_OFF_BAR3] = 4;
	pci_size_map[PCI_OFF_BAR4] = 4;
	pci_size_map[PCI_OFF_BAR5] = 4;
	pci_size_map[PCI_OFF_INTERRUPT_LINE] = 1;
	pci_size_map[PCI_OFF_SECONDARY_BUS]	= 1;

	printf("PCI successfully initialized\n");
	printf("Enumerating PCI devices: \n");
    enumerate_pci_devices();
}

int pci_isDeviceAvailable(uint16_t vendor_id, uint16_t device_id)
{
	pci_t t = pci_get_device(vendor_id, device_id, -1);

	if(memcmp((uint8_t*)&t, (uint8_t*)&dev_zero, sizeof(pci_t)) == 1)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

pci_bar_t GetBaseAddressRegister(pci_t dev, uint16_t bar)
{
    pci_bar_t result;

    uint32_t headertype = pci_read(dev, PCI_OFF_HEADER_TYPE) & 0x7F;
    int maxBARs = 6 - (4 * headertype);
    if (bar >= maxBARs)
        return result;

    uint32_t bar_value = pci_read(dev, 0x10 + 4 * bar);
    result.type = (bar_value & 0x1) ? PCI_DEVTYPE_IO : PCI_DEVTYPE_MMIO;

    if (result.type == PCI_DEVTYPE_MMIO)
    {

        switch ((bar_value >> 1) & 0x3)
        {

        case 0: // 32 Bit Mode
        case 1: // 20 Bit Mode
        case 2: // 64 Bit Mode
            break;
        }
    }
    else // InputOutput
    {
        result.address = (uint8_t *)(bar_value & ~0x3);
        result.prefetchable = false;
    }

    return result;
}

uint16_t pci_get_portBase(pci_t dev)
{
	uint16_t portBase = 0;
	for (int barNum = 0; barNum < 6; barNum++)
    {
        pci_bar_t bar = GetBaseAddressRegister(dev, barNum);
        if (bar.address && (bar.type == InputOutput))
            portBase = (uint32_t)bar.address;
    }

	return portBase;
}