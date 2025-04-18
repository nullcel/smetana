#ifndef STRING_H
#define STRING_H

#include "types.h"

void *memset(void *dst, int c, uint32 n);
void *memcpy(void *dst, const void *src, uint32 n);
void *memmove(void *dst, const void *src, uint32 n);
int memcmp(uint8 *s1, uint8 *s2, uint32 n);
int strlen(const char *s);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, int c);
int strcpy(char *dst, const char *src);
void strncpy(char *dst, const char *src, uint32 n);
void strcat(char *dest, const char *src);
int isspace(char c);
int isalpha(char c);
char upper(char c);
char lower(char c);
void itoa(char *buf, int base, int d);
char *strstr(const char *in, const char *str);
char *strchr(const char *str, char c);
char *strtok(char *str, const char *delim);

#endif

