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
    
    struct dirent* pFolderItem;
    while(1)
    {
        pFolderItem = readdir(pDir);
        if(NULL == pFolderItem)
            break;

        DIR* pTemp = opendir((const char*)pFolderItem->d_name);
        if(pTemp)
            closedir(pTemp);
        else
        {
            if(0 == strcmp(filespec, pFolderItem->d_name))
                return 1;
        }
    };
    closedir(pDir);
	return -1;
}


}; // namespace avxsynth
