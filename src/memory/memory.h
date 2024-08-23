#ifndef MEMORY_H
#define MEMORY_H
#include <stddef.h>

void* memoryset(void* ptr, int c, size_t size);

int memorycmp(void* s1, void* s2, int count);

void* memorycopy(void* dest, void* src, int size);
#endif