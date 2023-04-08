#ifndef __STRING_H__
#define __STRING_H__

#include "system.h"
#include "list.h"

int memcmp(uint8_t *data1, uint8_t *data2, int n); //* Implemented
void *memcpy(void *dst, const void *src, int n);   //* Implemented

uint16_t *memsetw(uint16_t *dest, uint16_t val, uint32_t count); //! Not implemented
uint16_t *memsetdw(uint32_t *dest, uint32_t val, uint32_t count);  //! Not implemented
void *memset(void *dst, char val, int n);          //* Implemented

int strlen(const char *s); //* Implemented

char *strncpy(char *destString, const char *sourceString, int maxLength); //* Implmented
int strcpy(char *dst, const char *src); //* Implemented

int strcmp(const char *dst, char *src); //* Implemented
int strncmp(const char *s1, const char *s2, int c); //* Implemented

char *strstr(const char *in, const char *str); //* Implemented
void strcat(void *dest, const void *src);      //* Implemented

char* itoa_r(unsigned long int n, int base);   //! Not implemented
void itoa(char *buf, unsigned long int n, int base);    //* Implemented
int atoi(char *string);     //* Implemented

int isspace(char c);    //* Implemented
int isprint(char c);    //* Implemented

char *strdup(const char *src);  //! Not implemented
char *strndup(const char *src, uint32_t len);   //! Not implemented
char *strsep(char **stringp, const char *delim); //* Implemented

void stoc(size_t n, char* buffer);  //? Partially implemented
char* stoc_r(size_t n); //! Not implemented
uint32_t chbc(char* str, char c);   //* Implemented

list_t *str_split(const char *str, const char *delim, uint32_t *numtokens); //* Implemented
char *list2str(list_t *list, const char *delim); //* Implemented

int memzero(uint8_t* data, size_t size);

void sprintf(char* buf, const char* fmt, ...);

#endif /*__STRING_H__*/