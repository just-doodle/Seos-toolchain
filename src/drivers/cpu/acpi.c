#include "acpi.h"
#include "logdisk.h"

rsdp_descriptor_old_t *old_rsdp = NULL;
rsdp_descriptor_new_t *new_rsdp = NULL;

static uint32_t acpi_revision = -1;

void init_acpi()
{
    if(multiboot2_tag_is_available(MULTIBOOT_TAG_TYPE_ACPI_OLD))
    {
        struct multiboot_tag_old_acpi* old_acpi = get_tag_ptr(MULTIBOOT_TAG_TYPE_ACPI_OLD);
        old_rsdp = old_acpi->rsdp;
        acpi_revision = old_rsdp->revision;
        ldprintf("ACPI", LOG_INFO, "Old RSDP Found with signature \"%s\" and OEMID \"%s\" and revision %d in 0x%x", old_rsdp->signature, old_rsdp->oemid, old_rsdp->revision, old_rsdp->rsdt_addr);

        alloc_region(kernel_page_dir, old_rsdp->rsdt_addr, old_rsdp->rsdt_addr+0x1000, 1, 1, 1);
        rsdt_t* rsdt = old_rsdp->rsdt_addr;
        uint32_t entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;

        for(uint32_t i = 0; i < entries; i++)
        {
            sdt_header_t* sdt = (void*)rsdt->tables[i];
            ldprintf("ACPI", LOG_INFO, "%s: \"%s\", %dB", strndup(sdt->Signature, 4), strndup(sdt->OEMID, 6), sdt->Length);
        }
    }
    else if(multiboot2_tag_is_available(MULTIBOOT_TAG_TYPE_ACPI_NEW))
    {
        struct multiboot_tag_new_acpi* new_acpi = get_tag_ptr(MULTIBOOT_TAG_TYPE_ACPI_NEW);
        new_rsdp = new_acpi->rsdp;
        acpi_revision = new_rsdp->old.revision;
        ldprintf("ACPI", LOG_INFO, "New RSDP Found with signature \"%s\" and OEMID \"%s\" in 0x%x", new_rsdp->old.signature, new_rsdp->old.oemid, new_rsdp->xsdt_address);

        alloc_region(kernel_page_dir, new_rsdp->xsdt_address, new_rsdp->xsdt_address+0x1000, 1, 1, 1);
        rsdt_t* rsdt = (void*)new_rsdp->old.rsdt_addr;
        xxd(new_rsdp->old.rsdt_addr, sizeof(rsdt_t));
        if(memcmp("RSDT", rsdt->h.Signature, 4) != 1)
        {
            ldprintf("ACPI", LOG_ERR, "RSDT is invalid %s", rsdt->h.Signature);
            return;
        }
        uint32_t entries = (rsdt->h.Length - sizeof(sdt_header_t)) / 4;
        ldprintf("ACPI", LOG_INFO, "There is %d entries in RSDT", entries);

        for(uint32_t i = 0; i < entries; i++)
        {
            sdt_header_t* sdt = (void*)(rsdt->tables[i] & 0xFFFFFFFF);
            ldprintf("ACPI", LOG_INFO, "%s: \"%s\", %dB", sdt->Signature, sdt->OEMID, sdt->Length);
        }
    }
}

uint32_t acpi_get_revision()
{
    return acpi_revision;
}

void* acpi_get_rsdp()
{
    if(acpi_revision == 0)
    {
        return old_rsdp;
    }
    else if(acpi_revision == 1)
    {
        return new_rsdp;
    }
    else
    {
        return NULL;
    }
}