#ifndef UTILS_H
#define UTILS_H

#include "types.h"

void init_heap(void);
void* malloc(uint32 size);
void free(void* ptr);
void* memset(void* ptr, int value, uint32 num);
int abs(int n);  // Add abs function

#endif