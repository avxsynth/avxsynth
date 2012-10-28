#include "windowsPorts.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dirent.h>

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
    DIR* pDir = opendir(".");
    if(NULL == pDir)
        return 0;
    
    char fullFilename[PATH_MAX];
    memset(fullFilename, 0, PATH_MAX*sizeof(char));
    char* pRetVal = realpath(filespec, fullFilename);
    if((NULL == pRetVal) || (0 == strlen(fullFilename)))
        return -1;
    
	return 0;
}


}; // namespace avxsynth
