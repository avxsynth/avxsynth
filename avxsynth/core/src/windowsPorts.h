#ifndef __WINDOWS_PORTS_H__
#define __WINDOWS_PORTS_H__

#include <stdarg.h>
#include <limits.h>
#include <time.h>
#include "windowsPorts/windows2linux.h"

namespace avxsynth {
  
char* _strrev(char *str);
char* _strupr(char *str);
char* _strlwr(char *str);


struct _finddata_t 
{
	unsigned    attrib;
    time_t      time_create;    /* -1 for FAT file systems */
    time_t      time_access;    /* -1 for FAT file systems */
    time_t      time_write;
    _fsize_t    size;
    char        name[PATH_MAX]; // originally name[260]
};

intptr_t _findfirst(const char *filespec, struct _finddata_t *fileinfo);

}; // namespace avxsynth
#endif // __WINDOWS_PORTS_H__
