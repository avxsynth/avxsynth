#ifndef __AVXIFACE_H__
#define __AVXIFACE_H__

#include <stdint.h>
#include "./windowsPorts/basicDataTypeConversions.h"
#include "./windowsPorts/windows2linux.h"

#include "./windowsPorts/UnknwnLinux.h"
#include "./windowsPorts/WinDefLinux.h"
#include "./windowsPorts/WinGDILinux.h"
#include "./windowsPorts/VfwLinux.h"
#include "./windowsPorts/WinBaseLinux.h"
#include "./windowsPorts/MMRegLinux.h"

namespace avxsynth {

typedef struct tagIAVXPersistFile
{
	virtual HRESULT Load(LPCOLESTR lpszFileName, DWORD grfMode)                                     = 0;
	virtual HRESULT Save(LPCOLESTR lpszFileName, BOOL fRemember)                                    = 0;
	virtual HRESULT SaveCompleted(LPCOLESTR lpszFileName)                                           = 0;
	virtual HRESULT GetCurFile(LPOLESTR *lplpszFileName)                                            = 0;
} IAVXPersistFile;

typedef struct tagIAVXFile
{
	virtual HRESULT CreateStream(PAVISTREAM *ppStream, AVISTREAMINFOW *psi)                    = 0;
	virtual HRESULT EndRecord()                                                                     = 0;
	virtual HRESULT GetStream(PAVISTREAM *ppStream, DWORD fccType, LONG lParam)                = 0;
	virtual HRESULT Info(AVIFILEINFOW *psi, LONG lSize)                                             = 0;

	virtual HRESULT Open(LPCSTR szFile, UINT mode, LPCOLESTR lpszFileName)                          = 0;
        virtual HRESULT Save(LPCSTR szFile, AVICOMPRESSOPTIONS* lpOptions,      
				AVISAVECALLBACK lpfnCallback)                                           = 0;

	virtual HRESULT ReadData(DWORD fcc, LPVOID lp, LONG *lpcb)                                      = 0;
	virtual HRESULT WriteData(DWORD fcc, LPVOID lpBuffer, LONG cbBuffer)                            = 0;
	virtual HRESULT DeleteStream(DWORD fccType, LONG lParam)                                        = 0;
} IAVXFile;

typedef struct tagIAVXClipInfo {
	virtual int  GetError(const char** ppszMessage)							= 0;
	virtual bool GetParity(int n)									= 0;
	virtual bool IsFieldBased()									= 0;
} IAVXClipInfo;


typedef struct tagIAVXSynth : IAVXFile, IAVXPersistFile, IAVXClipInfo
{
    // combined interface
} IAVXSynth;

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
typedef unsigned long (*PCREATEAVISYNTH)(IAVXSynth **pIAVXSynth);

HRESULT CreateAVISynth(IAVXSynth **pIAVXSynth);
#ifdef __cplusplus
}
#endif // __cplusplus

}; // namespace avxsynth

#endif // __AVXIFACE_H__
