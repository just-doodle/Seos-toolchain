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


void strcat(void *dest, const void *src)
{
    char *end = (char *)dest + strlen((char*)dest);
    memcpy((char *)end, (char *)src, strlen((char *)src));
    end = end + strlen((char *)src);
    *end = '\0';
}


void itoa(char *buf, unsigned long int n, int base)
{
    unsigned long int tmp;
    int i, j;

    tmp = n;
    i = 0;

    do
    {
        tmp = n % base;
        buf[i++] = (tmp < 10) ? (tmp + '0') : (tmp + 'a' - 10);
    } while (n /= base);
    buf[i--] = 0;

    for (j = 0; j < i; j++, i--)
    {
        tmp = buf[j];
        buf[j] = buf[i];
        buf[i] = tmp;
    }
}

int atoi(char *string)
{
    int result = 0;
    unsigned int digit;
    int sign;

    while (isspace(*string))
    {
        string += 1;
    }

    if (*string == '-')
    {
        sign = 1;
        string += 1;
    }
    else
    {
        sign = 0;
        if (*string == '+')
        {
            string += 1;
        }
    }

    for (;; string += 1)
    {
        digit = *string - '0';
        if (digit > 9)
        {
            break;
        }
        result = (10 * result) + digit;
    }

    if (sign)
    {
        return -result;
    }
    return result;
}


char* itoa_r(unsigned long int n, int base)
{
}

int isspace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

uint32_t chbc(char* str, char c)
{
    uint32_t i = 0;
    while (str[i] != '\0')
    {
        if (str[i] == c)
            return i;
        i++;
    }
    return -1;
}

int isprint(char c)
{
    return ((c >= ' ' && c <= '~') ? 1 : 0);
}

char *strdup(const char *src)
{
}

char *strndup(const char *src, uint32_t len)
{
}

char *strsep(char **stringp, const char *delim) {
    char *s;
    const char *spanp;
    int c, sc;
    char *tok;
    if ((s = *stringp) == NULL)
        return (NULL);
    for (tok = s;;) {
        c = *s++;
        spanp = delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;
                *stringp = s;
                return (tok);
            }
        } while (sc != 0);
    }
}

bool alloc = false;

void stoc(size_t n, char* buf)
{
    if(((n / MB) >> 10) != 0)
    {
        strcpy(buf, itoa_r(n / GB, 10));
        strcat(buf, "GB");
    }
    else if (((n / KB) >> 10) != 0)
    {
        strcpy(buf, itoa_r(n / MB, 10));
        strcat(buf, "MB");
    }
    else if (((n) >> 10) != 0)
    {
        strcpy(buf, itoa_r(n / KB, 10));
        strcat(buf, "KB");
    }
    else
    {
        strcpy(buf, itoa_r(n, 10));
        strcat(buf, "B");
    }
}

char* stoc_r(size_t n)
{
}