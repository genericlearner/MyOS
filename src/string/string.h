#ifndef STRING_H
#define STRING_H 
#include <stdbool.h>

int strlen(const char* ptr);
bool isdigit(char c);
int chartonum(char c);
int strnlen(const char* ptr, int max);
char* strcpy(char* dest, const char* src);
int strnlen_terminator(const char* str, int max, char terminator);
int istrncmp(const char* s1, const char* s2, int n);
int strncmp(const char* str1, const char* str2, int n);
char tolower(char s1);

#endif