#include "string.h"

//calculates the string length
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

//Calculates the max string length?
int strnlen(const char* ptr, int max)
{
   int i;
    for (i = 0; i < max && ptr[i] != '\0'; i++)
    {
        // Loop continues while `i` is less than `max` and `ptr[i]` is not the null terminator
    }
    return i;
}


//This function checks if the given char is a digit
bool isdigit(char c)
{
    return c>=48 && c<=57;
}

//limited for numerical char
int chartonum(char c)
{   
    
    return c-48;
}

//String copy function
char* strcpy(char* dest, const char* src)
{
    char* res=dest;
    while(*src != 0)
    {
        *dest =*src;
        src+=1;
        dest +=1;
    }
    *dest = 0x00;

    return res;
}