#ifndef __DATA_TYPE_CONVERSIONS_H__
#define __DATA_TYPE_CONVERSIONS_H__

#include <inttypes.h>
#include <pthread.h>

namespace avxsynth {
	
typedef int64_t __int64;
typedef int32_t __int32;
typedef bool	BOOL;
typedef void* HMODULE;
typedef void* LPVOID;
typedef void* PVOID;
typedef PVOID HANDLE;
typedef HANDLE HWND;
typedef HANDLE HINSTANCE;
typedef void* HDC;
typedef void* HBITMAP;
typedef void* HICON;
typedef void* HFONT;
typedef void* HGDIOBJ;
typedef void* HBRUSH;
typedef void* HMMIO;
typedef void* HACMSTREAM;
typedef void* HACMDRIVER;
typedef void* HIC;
typedef void* HACMOBJ;
typedef void* HACMDRIVER;
typedef void* HACMSTREAM;
typedef HACMSTREAM* LPHACMSTREAM;
typedef void* HACMDRIVERID;
typedef void* LPHACMDRIVER;
typedef unsigned char BYTE;
typedef BYTE* LPBYTE;
typedef char TCHAR;
typedef TCHAR* LPTSTR;
typedef const TCHAR* LPCTSTR;
typedef char* LPSTR;
typedef LPSTR LPOLESTR;
typedef const char* LPCSTR;
typedef LPCSTR LPCOLESTR;
typedef wchar_t WCHAR;
typedef unsigned short WORD;
typedef unsigned int UINT;
typedef UINT MMRESULT;
typedef uint32_t DWORD;
typedef DWORD COLORREF;
typedef DWORD FOURCC;
typedef DWORD HRESULT;
typedef DWORD* LPDWORD;
typedef DWORD* DWORD_PTR;
typedef int32_t LONG;
typedef int32_t* LONG_PTR;
typedef LONG_PTR LRESULT;
typedef uint32_t ULONG;
typedef uint32_t* ULONG_PTR;
//typedef __int64_t intptr_t;
typedef __uint64_t _fsize_t;
typedef pthread_mutex_t CRITICAL_SECTION;


//
// Structures
//

typedef struct _GUID {
  DWORD Data1;
  WORD  Data2;
  WORD  Data3;
  BYTE  Data4[8];
} GUID;

typedef GUID REFIID;
typedef GUID CLSID;
typedef CLSID* LPCLSID;
typedef GUID IID;

}; // namespace avxsynth
#endif //  __DATA_TYPE_CONVERSIONS_H__
