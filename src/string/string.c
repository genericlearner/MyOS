#include "string.h"

char tolower(char s1)
{
    if(s1>=65 && s1<= 90)
    {
        s1+=32;
    }
    return s1; 
}
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
int strnlen_terminator(const char* str, int max, char terminator)
{
    int i = 0;
    for(i = 0; i<max; i++)
    {
        if(str[i] == '\0' || str[i] == terminator)
            break;

        
    }
    return i;
}
int istrncmp(const char* s1, const char* s2, int n)
{
    unsigned char u1, u2;
    while(n-- >0)
    {
        u1 = (unsigned char )* s1++;
        u2 = (unsigned char)* s2++;
        if(u1 != u2 && tolower(u1) != tolower(u2))
            return u1 - u2;
        if(u1 == '\0')
            return 0;

    }
    return 0;

}
int strncmp(const char* str1, const char* str2, int n)
{
    unsigned char u1, u2;
    while(n-- >0)
    {
        u1 = (unsigned char )*str1++;
        u2 = (unsigned char)*str2++;
        if(u1 != u2)
            return u1 - u2;
        if(u1 == '\0')
            return 0;
    }
    return 0;
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