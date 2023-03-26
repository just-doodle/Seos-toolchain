#ifndef __STRING_H__
#define __STRING_H__

#include "system.h"

int memcmp(uint8_t *data1, uint8_t *data2, int n); //* Implemented
void *memcpy(void *dst, const void *src, int n);   //* Implemented

uint16_t *memsetw(uint16_t *dest, uint16_t val, uint32_t count);
uint16_t *memsetdw(uint32_t *dest, uint32_t val, uint32_t count);
void *memset(void *dst, char val, int n);          //* Implemented

int strlen(const char *s); //* Implemented

char *strncpy(char *destString, const char *sourceString, int maxLength); //* Implmented
int strcpy(char *dst, const char *src); //* Implemented

int strcmp(const char *dst, char *src); //* Implemented
int strncmp(const char *s1, const char *s2, int c); //* Implemented

char *strstr(const char *in, const char *str); //* Implemented
void strcat(void *dest, const void *src);

char* itoa_r(unsigned long int n, int base);
void itoa(char *buf, unsigned long int n, int base);
int atoi(char *string);

int isspace(char c);
int isprint(char c);

char *strdup(const char *src);
char *strndup(const char *src, uint32_t len);
char *strsep(char **stringp, const char *delim);

void stoc(size_t n, char* buffer);
char* stoc_r(size_t n);
uint32_t chbc(char* str, char c);

#endif /*__STRING_H__*/