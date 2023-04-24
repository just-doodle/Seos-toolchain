#ifndef __COMMANDDEV_H__
#define __COMMANDDEV_H__

#include "system.h"
#include "kheap.h"
#include "printf.h"
#include "vfs.h"
#include "devfs.h"

#define CMD_REQUEST_MAGIC1 0x43 
#define CMD_REQUEST_MAGIC2 0x4D 
#define CMD_REQUEST_MAGIC3 0x44 
#define CMD_REQUEST_MAGIC4 0x52 
#define CMD_REQUEST_MAGIC5 0x45 
#define CMD_REQUEST_MAGIC6 0x51
#define CMD_REQUEST_MAGIC7 0xCF

#define CMD_RESPONSE_MAGIC1 'C'
#define CMD_RESPONSE_MAGIC2 'M'
#define CMD_RESPONSE_MAGIC3 'D'
#define CMD_RESPONSE_MAGIC4 'R'
#define CMD_RESPONSE_MAGIC5 'E'
#define CMD_RESPONSE_MAGIC6 'T'
#define CMD_RESPONSE_MAGIC7 0xAC

#define CMD_REQUEST_NULL 0x00
#define CMD_REQUEST_TEST 0x02
#define CMD_REQUEST_ATTACH_INPUT_HANDLER 0xAF
#define CMD_REQUEST_ATTACH_PIT_HANDLER 0xEC
#define CMD_REQUEST_CREATE_WINDOW 0x2F
#define CMD_REQUEST_WINDOW_DISPLAY 0xFD
#define CMD_REQUEST_WINDOW_CHANGE_TITLE 0xDE

#define CMD_RESPONSE_ERROR 0x00
#define CMD_RESPONSE_OK 0x02
#define CMD_RESPONSE_NO 0x03
#define CMD_SUB_RESPONSE_NO_PTR 0x05
#define CMD_SUB_RESPONSE_HAVE_PTR 0x0A

typedef struct commanddev_request_struct
{
    uint8_t magic[7];
    int request;
    uint32_t args[5];
    int n_args;
    void* ptr; // If the request wanted ptr
    uint32_t size;
}cdev_request_t;

typedef struct commanddev_response_struct
{
    uint8_t magic[7];
    int response;
    int request;
    void* return_ptr;
}cdev_response_t;

void init_command_dev();

cdev_response_t* commanddev_interpret(cdev_request_t* req);

uint32_t commanddev_read(FILE* f, uint32_t off, uint32_t sz, char* ptr);
uint32_t commanddev_write(FILE* f, uint32_t off, uint32_t sz, char* ptr);

void commanddev_open(FILE* f, uint32_t mode);
void commanddev_close(FILE* f);

#endif /*__COMMANDDEV_H__*/