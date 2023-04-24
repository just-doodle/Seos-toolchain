#include "commanddev.h"

#include "vesa.h"
#include "process.h"
#include "pit.h"

#include "compositor.h"

int read_pending = false;
cdev_response_t* pending_response = NULL;

FILE* cmd_f = NULL;

uint32_t commanddev_getfilesize(FILE* f)
{
    return sizeof(cdev_request_t) + sizeof(cdev_response_t);
}

void init_commanddev()
{
    cmd_f = zalloc(sizeof(FILE));
    strcpy(cmd_f->name, "command");
    cmd_f->close = commanddev_close;
    cmd_f->read = commanddev_read;
    cmd_f->write = commanddev_write;
    cmd_f->open = commanddev_open;
    cmd_f->flags = FS_CHARDEVICE;
    cmd_f->get_filesize = commanddev_getfilesize;

    devfs_add(cmd_f);
}


cdev_response_t* commanddev_interpret(cdev_request_t* req)
{
    serialprintf("[CMDDEV] Got command: 0x%02x with %d args\n", req->request, req->n_args);
    int request_id = req->request;
    cdev_response_t* r = ZALLOC_TYPES(cdev_response_t);
    r->request = request_id;
    r->magic[0] = CMD_RESPONSE_MAGIC1;
    r->magic[1] = CMD_RESPONSE_MAGIC2;
    r->magic[2] = CMD_RESPONSE_MAGIC3;
    r->magic[3] = CMD_RESPONSE_MAGIC4;
    r->magic[4] = CMD_RESPONSE_MAGIC5;
    r->magic[5] = CMD_RESPONSE_MAGIC6;
    r->magic[6] = CMD_RESPONSE_MAGIC7;
    r->response = CMD_RESPONSE_ERROR;

    if(req->magic[0] == CMD_REQUEST_MAGIC1 && req->magic[1] == CMD_REQUEST_MAGIC2 && req->magic[2] == CMD_REQUEST_MAGIC3 && req->magic[3] == CMD_REQUEST_MAGIC4 && req->magic[4] == CMD_REQUEST_MAGIC5 && req->magic[5] == CMD_REQUEST_MAGIC6 && req->magic[6] == CMD_REQUEST_MAGIC7)
    {
        switch(request_id)
        {
        case CMD_REQUEST_CREATE_WINDOW:
            {
                if(req->n_args < 4)
                {
                    r->response = CMD_RESPONSE_ERROR;
                }
                else
                {
                    r->response = CMD_RESPONSE_OK | CMD_SUB_RESPONSE_HAVE_PTR;
                    window_t* ret = create_window(req->ptr, req->args[0], req->args[1], req->args[2], req->args[3]);
                    r->return_ptr = ret;
                }
            }break;
        case CMD_REQUEST_ATTACH_INPUT_HANDLER:
            {
                if(req->ptr == NULL)
                {
                    r->response = CMD_RESPONSE_ERROR;
                }
                else
                {
                    r->response = CMD_RESPONSE_OK | CMD_SUB_RESPONSE_NO_PTR;
                    attach_handler(req->ptr);
                }
            }break;
        case CMD_REQUEST_ATTACH_PIT_HANDLER:
            {
                if(req->ptr == NULL || req->n_args < 1)
                {
                    r->response = CMD_RESPONSE_ERROR;
                }
                else
                {
                    r->response = CMD_RESPONSE_OK | CMD_SUB_RESPONSE_NO_PTR;
                    pit_register(req->ptr, req->args[0]);
                }
            }break;
        case CMD_REQUEST_WINDOW_DISPLAY:
            {
                if(req->ptr == NULL || req->n_args < 1)
                {
                    r->response = CMD_RESPONSE_ERROR;
                }
                else
                {
                    r->response = CMD_RESPONSE_OK | CMD_SUB_RESPONSE_NO_PTR;
                    window_display(req->ptr, req->args[0]);
                }
            }break;
        case CMD_REQUEST_WINDOW_CHANGE_TITLE:
        {
            if(req->ptr == NULL || req->n_args < 1)
            {
                r->response = CMD_RESPONSE_ERROR;
            }
            else
            {
                r->response = CMD_RESPONSE_OK | CMD_SUB_RESPONSE_NO_PTR;
                window_change_title(req->ptr, req->args[0]);
            }
        }break;
        };
        return r;
    }
}

uint32_t commanddev_read(FILE* f, uint32_t offset, uint32_t size, char* ptr)
{
    if(current_process->pending_response != NULL)
    {
        memcpy(ptr, pending_response, sizeof(cdev_response_t));
        pending_response = NULL;
        return 0;
    }
    else
    {
        return 1;
    }
}

uint32_t commanddev_write(FILE* f, uint32_t off, uint32_t size, char* buffer)
{
    if(current_process->pending_response != NULL)
    {
        return 2;
    }
    cdev_request_t *req = buffer;
    pending_response = commanddev_interpret(req);
    return 0;
}

void commanddev_open(FILE* f, uint32_t flags)
{
    return;
}

void commanddev_close(FILE* f)
{
    return;
}