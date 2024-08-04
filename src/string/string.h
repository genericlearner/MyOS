#ifndef STRING_H
#define STRING_H 
#include <stdbool.h>

int strlen(const char* ptr);
bool isdigit(char c);
int chartonum(char c);
int strnlen(const char* ptr, int max);
char* strcpy(char* dest, const char* src);


#endif