#include "logdisk.h"

#define TERM_ERR_COLOR "\033[1;31m"
#define TERM_WARN_COLOR "\033[1;33m"
#define TERM_INFO_COLOR "\033[1;36m"
#define TERM_DEBUG_COLOR "\033[1;35m"
#define TERM_RESET_COLOR "\033[m"

logdisk_t* ldisk = NULL;

uint32_t logdisk_read(FILE* f, uint32_t off, uint32_t size, char* buffer)
{
    if((validate(f) != 1) || (validate(f->device) != 1) || (validate(buffer) != 1))
    {
        return -1;
    }

    logdisk_t* l = f->device;
    charbuffer_read((l->buf), off, size, buffer);
    return size;
}

uint32_t logdisk_write(FILE* f, uint32_t off, uint32_t size, char* buffer)
{
    if((validate(f) != 1) || (validate(f->device) != 1) || (validate(buffer) != 1))
    {
        return -1;
    }
    if(size != strlen(buffer))
        return -1;

    logdisk_log("/dev/log", buffer, off);
    return size;
}

uint32_t logdisk_getFileSize(FILE* f)
{
    if((validate(f) != 1) || (validate(f->device) != 1))
    {
        return -1;
    }

    logdisk_t* l = f->device;
    return l->buf->rw_ptr;
}

void logdisk_open(FILE* f, uint32_t flag)
{
    return;
}

void logdisk_close(FILE* f)
{
    return;
}

void logdisk_ioctl(FILE* f, int req, void* data)
{
    if((validate(f) != 1) || (validate(data) != 1))
        return;
}

void init_logdisk(uint32_t size, int type)
{
    ldisk = ZALLOC_TYPES(logdisk_t);
    ldisk->buf = create_charbuffer(size);
    ldisk->type = type;
}

void logdisk_mount()
{
    FILE* f = ZALLOC_TYPES(FILE);
    strcpy(f, "log");
    f->device = ldisk;
    f->open = logdisk_open;
    f->close = logdisk_close;
    f->get_filesize = logdisk_getFileSize;
    f->read = logdisk_read;
    f->write = logdisk_write;
    f->ioctl = logdisk_ioctl;
    f->size = ldisk->buf->rw_ptr;
    devfs_add(f);
}

void logdisk_change_policy(int policy)
{
    if(policy > LOG_OFF)
    {
        ldprintf("logdisk", LOG_WARN, "Given policy %d is invalid. Policy is unchanged.", policy);
        return;
    }

    ldisk->type = policy;
}

char* logdisk_type_to_str(int type)
{
    switch(type)
    {
    case LOG_DEBUG:
        return "DEBUG";
        break;
    case LOG_INFO:
        return "INFO";
        break;
    case LOG_WARN:
        return "WARN";
        break;
    case LOG_ERR:
        return "ERROR";
        break;
    case LOG_VERBOSE:
        return "VERBOSE";
        break;
    case LOG_OFF:
        return "OFF";
        break;
    default:
        return "UNKNOWN";
        break;
    };
}

void logdisk_printType(char* issuer, char* msg, int type)
{
    switch(type)
    {
    case LOG_INFO:
        printf(TERM_INFO_COLOR"[ %s ] %s: %s\n"TERM_RESET_COLOR, logdisk_type_to_str(type), issuer, msg);
        serialprintf("[ %s ] %s: %s\n", logdisk_type_to_str(type), issuer, msg);
        break;
    case LOG_WARN:
        printf(TERM_WARN_COLOR"[ %s ] %s: %s\n"TERM_RESET_COLOR, logdisk_type_to_str(type), issuer, msg);
        serialprintf("[ %s ] %s: %s\n", logdisk_type_to_str(type), issuer, msg);
        break;
    case LOG_ERR:
        printf(TERM_ERR_COLOR"[ %s ] %s: %s\n"TERM_RESET_COLOR, logdisk_type_to_str(type), issuer, msg);
        serialprintf("[ %s ] %s: %s\n", logdisk_type_to_str(type), issuer, msg);
        break;
    case LOG_DEBUG:
        //printf(TERM_DEBUG_COLOR"[ %s ] %s: %s\n"TERM_RESET_COLOR, logdisk_type_to_str(type), issuer, msg);
        serialprintf("[ %s ] %s: %s\n", logdisk_type_to_str(type), issuer, msg);
        break;
    default:
        printf("[ %s ] %s: %s\n", logdisk_type_to_str(type), issuer, msg);
        serialprintf("[ %s ] %s: %s\n", logdisk_type_to_str(type), issuer, msg);
        break;
    };
}

void logdisk_charbuffer_putchar(char c)
{
    if(validate(ldisk) != 1)
        return;
    charbuffer_push(ldisk->buf, c);
}

void lprintf(char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsprintf(NULL, logdisk_charbuffer_putchar, fmt, args);
    va_end(args);
}

void ldprintf(char* issuer, int type, char* fmt, ...)
{
    char* buf = zalloc(strlen(fmt) + 1024);
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, NULL, fmt, args);
    va_end(args);

    if(type == LOG_ERR)
    {
        char* ebuf = zalloc(strlen(buf) + strlen(issuer) + 16);
        sprintf(ebuf, "Error occurred:\n[%s] %s", issuer, buf);
        compositor_message_show(ebuf);
        free(ebuf);
    }
    logdisk_log(issuer, buf, type);
    free(buf);
}

void logdisk_log(char* issuer, char* msg, int type)
{
    lprintf("[ %s ] %s: %s\n", logdisk_type_to_str(type), issuer, msg);
    if(ldisk->type <= type)
    {
        logdisk_printType(issuer, msg, type);
    }
}

void logdisk_dump(char * file)
{
    if(validate(file) != 1 || (validate(ldisk) != 1) || (validate((ldisk->buf)) != 1))
        return;
    charbuffer_dump_to_file((ldisk->buf), file);
}