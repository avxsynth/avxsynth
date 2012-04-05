#include "windowsPorts.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

namespace avxsynth{  
    
char *_strrev(char *str)
{
    unsigned long nLength = strlen(str);
    for(unsigned long i = 0; i < nLength/2; i++)
    {
        char chTemp = str[i];
        str[i] = str[nLength - i - 1];
        str[nLength - i - 1] = chTemp;
    }
    return str;
}

char *_strupr(char *str)
{
    if (str)
    {
        unsigned long nLength = strlen(str);
        for(unsigned long i = 0; i < nLength; i++)
        {
            str[i] = toupper(str[i]);
        }
    }   
    
	return str;
}

char *_strlwr(char *str)
{
    if (str)
    {
        unsigned long nLength = strlen(str);
        for(unsigned long i = 0; i < nLength; i++)
        {
            str[i] = tolower(str[i]);
        }
    }   
    
    return str;
}


intptr_t _findfirst(const char *filespec, struct _finddata_t *fileinfo)
{
	return 0;
}


}; // namespace avxsynth
