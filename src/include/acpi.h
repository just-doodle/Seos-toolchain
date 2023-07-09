#ifndef __ACPI_H__
#define __ACPI_H__

#include "system.h"
#include "string.h"
#include "kheap.h"
#include "paging.h"
#include "multiboot2_tags.h"

typedef struct rsdp_descriptor_old_struct
{
    char signature[8];
    uint8_t checksum;
    char oemid[6];
    uint8_t revision;
    uint32_t rsdt_addr;
}__attribute__((packed))rsdp_descriptor_old_t;

typedef struct rsdp_descriptor_new_struct
{
    rsdp_descriptor_old_t old;

    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
}__attribute__((packed))rsdp_descriptor_new_t;

typedef struct sdt_header_struct
{
  char Signature[4];
  uint32_t Length;
  uint8_t Revision;
  uint8_t Checksum;
  char OEMID[6];
  char OEMTableID[8];
  uint32_t OEMRevision;
  uint32_t CreatorID;
  uint32_t CreatorRevision;
}sdt_header_t;

typedef struct rsdt_struct
{
  sdt_header_t h;
  uint32_t tables[];
}rsdt_t;

typedef struct xsdt_struct
{
  sdt_header_t h;
  uint64_t tables[];
}xsdt_t;

void init_acpi();
uint32_t acpi_get_revision();
void* acpi_get_rsdp();

#endif /*__ACPI_H__*/