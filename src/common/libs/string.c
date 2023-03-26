#include "string.h"

int strlen(const char* s)
{
    int ret = 0;
    for(ret = 0; s[ret] != '\0'; ret++);
    return ret;
}

int memcmp(uint8_t* data1, uint8_t* data2, int n)
{
    while(n--)
    {
        if(*data1 != *data2)
            return 0;
        
        data1++;
        data2++;
    }
    return 1;
}

void* memcpy(void* dst, const void* src, int n)
{
    char* ret = (char*)dst;
    char* d = (char*)dst;
    char* s = (char*)src;

    while(n--)
    {
        *d = *s;
        d++;
        s++;
    }

    return ret;
}

void* memset(void* dst, char val, int n)
{
    char* ret = (char*)dst;
    char* d = (char*)dst;

    while(n--)
    {
        *d++ = val;
    }

    return (void*)ret;
}

int strcpy(char* dst, const char* src)
{
    int n = strlen(src);
    int ret = n;

    while(n--)
    {
        *dst++ = *src++;
    }
    return ret;
}

char* strncpy(char* dst, const char* src, int n)
{
    unsigned count;
    if((dst == (char*)NULL) || (src == (char*)NULL))
    {
        return (dst == NULL);
    }

    if(n > 255)
    {
        n = 255;
    }

    for(count = 0; (int)count < n; count++)
    {
        dst[count] = src[count];

        if(src[count] == '\0')
            break;
    }

    if(count >= 255)
    {
        return (dst == NULL);
    }
}

int strcmp(const char* dst, char* src)
{
    int i = 0;
    while(dst[i] == src[i])
    {
        if(src[i++] == 0)
            return 0;
    }

    return 1;
}

int strncmp(const char *s1, const char *s2, int c)
{
    int result = 0;

    while (c)
    {
        result = *s1 - *s2++;

        if ((result != 0) || (*s1++ == 0))
        {
            break;
        }

        c--;
    }

    return result;
}

char* strstr(const char* in, const char* str)
{
    char c;
    size_t len;

    c = *str;
    if(!c)
        return (char*)in;

    len = strlen(str);

    do
    {
        char sc;
        do
        {
            sc = *in++;
            if (!sc)
                return (char *)0;
        } while (sc != c);
    } while (strncmp(in, str, len) != 0);

    return (char *)(in - 1);
}