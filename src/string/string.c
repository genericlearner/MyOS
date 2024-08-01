#include "string.h"


int strlen(const char* ptr)
{
    int i=0;
    while(*ptr!=0)
    {
        i++;
        ptr+=1;
    }
    return i;
}

int strnlen(const char* ptr, int max)
{
   int i;
    for (i = 0; i < max && ptr[i] != '\0'; i++)
    {
        // Loop continues while `i` is less than `max` and `ptr[i]` is not the null terminator
    }
    return i;
}

bool isdigit(char c)
{
    return c>=48 && c<=57;
}

//limited for numerical char
int chartonum(char c)
{   
    
    return c-48;
}