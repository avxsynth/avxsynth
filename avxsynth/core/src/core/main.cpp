// Avisynth v4.0  Copyright 2007 Ben Rudiak-Gould et al.
// http://www.avisynth.org
// http://www.avxsynth.org


// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .
//
// Linking Avisynth statically or dynamically with other modules is making a
// combined work based on Avisynth.  Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Avisynth give you
// permission to link Avisynth with independent modules that communicate with
// Avisynth solely through the interfaces defined in avisynth.h, regardless of the license
// terms of these independent modules, and to copy and distribute the
// resulting combined work under terms of your choice, provided that
// every copy of the combined work is accompanied by a complete copy of
// the source code of Avisynth (the version of Avisynth used to produce the
// combined work), being distributed under the terms of the GNU General
// Public License plus this exception.  An independent module is a module
// which is not derived from or based on Avisynth, such as 3rd-party filters,
// import and export plugins, or graphical user interfaces.


#define INITGUID
#define FP_STATE 0x9001f

#include <stdio.h>
#include <malloc.h>
#include <limits.h>
#include <math.h>
#include <cstdarg>
#include <ctype.h>
#include <float.h>
#include <wchar.h>
#include <algorithm>
using namespace std;

#include "../internal.h"
#include "../core/cache.h"
#include "../filters/debug.h"
#include "avxiface.h"
#include "./windowsPorts/excptLinux.h"
#include "./windowsPorts/WinNTLinux.h"
#include "./windowsPorts/MMRegLinux.h"
#include "avxlog.h"

namespace avxsynth {
	
#ifdef MODULE_NAME
#undef MODULE_NAME
#endif

#define MODULE_NAME core::main

const char _AVS_VERSTR[]    = AVS_VERSTR;
const char _AVS_COPYRIGHT[] = AVS_AVX_SYNTH AVS_COPYRIGHT AVS_VERSTR;

static long gRefCnt=0;


extern "C" const GUID CLSID_CAVIFileSynth   // {E6D6B700-124D-11D4-86F3-DB80AFD98778}
  = {0xe6d6b700, 0x124d, 0x11d4, {0x86, 0xf3, 0xdb, 0x80, 0xaf, 0xd9, 0x87, 0x78}};
  
class CAVIFileSynth : public IAVXSynth {
  friend class CAVIStreamSynth;
private:
    std::string GetScriptDirectoryAndFilename(char* & szScriptName);
    
private:
    char* szScriptName;
    IScriptEnvironment* env;
    PClip filter_graph;
    const VideoInfo* vi;
    const char* error_msg;

    CRITICAL_SECTION cs_filter_graph;

    bool DelayInit();
    bool DelayInit2();

    void MakeErrorStream(const char* msg);

    void Lock();
    void Unlock();

public:

	CAVIFileSynth();
	~CAVIFileSynth();

	//////////// IPersistFile
	STDMETHODIMP Load(LPCOLESTR lpszFileName, DWORD grfMode);
	STDMETHODIMP Save(LPCOLESTR lpszFileName, BOOL fRemember);
	STDMETHODIMP SaveCompleted(LPCOLESTR lpszFileName);
	STDMETHODIMP GetCurFile(LPOLESTR *lplpszFileName);

	//////////// IAVIFile

	STDMETHODIMP CreateStream(PAVISTREAM *ppStream, AVISTREAMINFOW *psi);       // 5
	STDMETHODIMP EndRecord();                                                   // 8
	STDMETHODIMP GetStream(PAVISTREAM *ppStream, DWORD fccType, LONG lParam);   // 4
	STDMETHODIMP Info(AVIFILEINFOW *psi, LONG lSize);                           // 3

	STDMETHODIMP Open(LPCSTR szFile, UINT mode, LPCOLESTR lpszFileName);        // ???
    STDMETHODIMP Save(LPCSTR szFile, AVICOMPRESSOPTIONS* lpOptions,         // ???
                      AVISAVECALLBACK lpfnCallback);

	STDMETHODIMP ReadData(DWORD fcc, LPVOID lp, LONG *lpcb);                    // 7
	STDMETHODIMP WriteData(DWORD fcc, LPVOID lpBuffer, LONG cbBuffer);          // 6
	STDMETHODIMP DeleteStream(DWORD fccType, LONG lParam);                      // 9

	//////////// IAvisynthClipInfo

	int __stdcall GetError(const char** ppszMessage);
	bool __stdcall GetParity(int n);
	bool __stdcall IsFieldBased();
    
    STDMETHODIMP_(ULONG) Release();
};

///////////////////////////////////
class CAVIStreamSynth;

class CAVIStreamSynth: public IAVIStream /*, public IAVIStreaming*/ {
public:

	CAVIStreamSynth(CAVIFileSynth *parentPtr, bool isAudio);
	~CAVIStreamSynth();

	//////////// IAVIStream

	STDMETHODIMP Create(LONG lParam1, LONG lParam2);
	STDMETHODIMP Delete(LONG lStart, LONG lSamples);
	STDMETHODIMP_(LONG) Info(AVISTREAMINFOW *psi, LONG lSize);
	STDMETHODIMP_(LONG) FindSample(LONG lPos, LONG lFlags);
	STDMETHODIMP Read(LONG lStart, LONG lSamples, LPVOID lpBuffer, LONG cbBuffer, LONG *plBytes, LONG *plSamples);
	STDMETHODIMP ReadData(DWORD fcc, LPVOID lp, LONG *lpcb);
	STDMETHODIMP ReadFormat(LONG lPos, LPVOID lpFormat, LONG *lpcbFormat);
	STDMETHODIMP SetFormat(LONG lPos, LPVOID lpFormat, LONG cbFormat);
	STDMETHODIMP Write(LONG lStart, LONG lSamples, LPVOID lpBuffer,
		LONG cbBuffer, DWORD dwFlags, LONG* plSampWritten, 
		LONG* plBytesWritten);
	STDMETHODIMP WriteData(DWORD fcc, LPVOID lpBuffer, LONG cbBuffer);
	STDMETHODIMP SetInfo(AVISTREAMINFOW *psi, LONG lSize);

	//////////// IAVIStreaming

	STDMETHODIMP Begin(LONG lStart, LONG lEnd, LONG lRate);
	STDMETHODIMP End();

private:
	CAVIFileSynth *parent;
	BOOL fAudio;
    
	char *sName;

	//////////// internal

	void ReadHelper(void* lpBuffer, int lStart, int lSamples, unsigned code[4]);
	void ReadFrame(void* lpBuffer, int n);

	HRESULT Read2(LONG lStart, LONG lSamples, LPVOID lpBuffer, LONG cbBuffer, LONG *plBytes, LONG *plSamples);
};

///////////////////////////////////////////////////////////////////////////
//
//	CAVIFileSynth
//
///////////////////////////////////////////////////////////////////////////
STDMETHODIMP CAVIFileSynth::Load(LPCOLESTR lpszFileName, DWORD grfMode) {
	char filename[FILENAME_MAX];

	// instead of the original WideCharToMultiByte
	memset(filename, 0, FILENAME_MAX*sizeof(char));
	strcpy(filename, lpszFileName);
	AVXLOG_INFO("%p->CAVIFileSynth::Load(\"%s\", 0x%X)", this, lpszFileName, grfMode);

	return Open(filename, grfMode, lpszFileName);
}

STDMETHODIMP CAVIFileSynth::Save(LPCOLESTR lpszFileName, BOOL fRemember) {
	AVXLOG_INFO("%p->CAVIFileSynth::Save()", this);
	return E_FAIL;
}

STDMETHODIMP CAVIFileSynth::SaveCompleted(LPCOLESTR lpszFileName) {
	AVXLOG_INFO("%p->CAVIFileSynth::SaveCompleted()", this);
	return S_OK;
}

STDMETHODIMP CAVIFileSynth::GetCurFile(LPOLESTR *lplpszFileName) {
	AVXLOG_INFO("%p->CAVIFileSynth::GetCurFile()", this);

	if (lplpszFileName) *lplpszFileName = NULL;

	return E_FAIL;
}

////////////////////////////////////////////////////////////////////////
//
//		CAVIFileSynth
//
////////////////////////////////////////////////////////////////////////
//////////// IAVIFile

STDMETHODIMP CAVIFileSynth::CreateStream(PAVISTREAM *ppStream, AVISTREAMINFOW *psi) {
	AVXLOG_INFO("%p->CAVIFileSynth::CreateStream()", this);
	*ppStream = NULL;
	return S_OK;//AVIERR_READONLY;
}

STDMETHODIMP CAVIFileSynth::EndRecord() {
	AVXLOG_INFO("%p->CAVIFileSynth::EndRecord()", this);
	return AVIERR_READONLY;
}

STDMETHODIMP CAVIFileSynth::Save(LPCSTR szFile, AVICOMPRESSOPTIONS* lpOptions,
				AVISAVECALLBACK lpfnCallback) {
	AVXLOG_INFO("%p->CAVIFileSynth::Save()", this);
	return AVIERR_READONLY;
}

STDMETHODIMP CAVIFileSynth::ReadData(DWORD fcc, LPVOID lp, LONG *lpcb) {
	AVXLOG_INFO("%p->CAVIFileSynth::ReadData()", this);
	return AVIERR_NODATA;
}

STDMETHODIMP CAVIFileSynth::WriteData(DWORD fcc, LPVOID lpBuffer, LONG cbBuffer) {
	AVXLOG_INFO("%p->CAVIFileSynth::WriteData()", this);
	return AVIERR_READONLY;
}

STDMETHODIMP CAVIFileSynth::DeleteStream(DWORD fccType, LONG lParam) {
	AVXLOG_INFO("%p->CAVIFileSynth::DeleteStream()", this);
	return AVIERR_READONLY;
}


///////////////////////////////////////////////////
/////// local

CAVIFileSynth::CAVIFileSynth() {
	AVXLOG_INFO("%p->CAVIFileSynth::CAVIFileSynth()", this);

    szScriptName = 0;
    env = 0;

    error_msg = 0;

    InitializeCriticalSection(&cs_filter_graph);
}

CAVIFileSynth::~CAVIFileSynth() {
	AVXLOG_INFO("%p->CAVIFileSynth::~CAVIFileSynth(), gRefCnt = %d", this, gRefCnt);

    Lock();

    delete[] szScriptName;

    filter_graph = 0;
    
	delete env;

    DeleteCriticalSection(&cs_filter_graph);
}


STDMETHODIMP CAVIFileSynth::Open(LPCSTR szFile, UINT mode, LPCOLESTR lpszFileName) {

//	AVXLOG_INFO("%p->CAVIFileSynth::Open(\"%s\", 0x%08lx)", this, szFile, mode);

    if (mode & (OF_CREATE|OF_WRITE))
      return E_FAIL;

    delete env;   // just in case
    env = 0;
    filter_graph = 0;
    vi = 0;

    szScriptName = new char[lstrlen(szFile)+1];
    if (!szScriptName)
      return AVIERR_MEMORY;
    lstrcpy(szScriptName, szFile);

    return S_OK;
}

bool CAVIFileSynth::DelayInit() {

    Lock();

    bool result;
    try{
        result = DelayInit2();
    }
    catch(...)
    {
        Unlock();
        throw;
    }

    Unlock();

    return result;
}

std::string CAVIFileSynth::GetScriptDirectoryAndFilename(char* & szScriptName)
{
    char strRealPath[PATH_MAX];
    memset(strRealPath, 0, PATH_MAX*sizeof(char));
    if(NULL == realpath(szScriptName, (char*)strRealPath))
    {
       throw AvisynthError("Invalid script path"); 
    }
    
    std::string strDirectory(strRealPath);
    size_t nLastSlash = strDirectory.find_last_of("/");
    if(std::string::npos == nLastSlash)
    {
       throw AvisynthError("Strangely formed script path"); 
    }
    strDirectory = strDirectory.assign(strDirectory, 0, nLastSlash);
    
    std::string strFilename(strRealPath);
    strFilename.replace(0, nLastSlash+1, "");
    
    delete[] szScriptName;
    szScriptName = NULL;
    
    szScriptName = new char[strFilename.length() + 1];
    memset(szScriptName, 0, (strFilename.length() + 1)*sizeof(char));
    sprintf(szScriptName, "%s", strFilename.c_str());
    
    return strDirectory;
}

bool CAVIFileSynth::DelayInit2() {
    
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
 AVXLOG_INFO("Original: 0x%.4x", _control87( 0, 0 ) );
 int fp_state = _control87( 0, 0 );
 _control87( FP_STATE, 0xffffffff );
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
 
 if (szScriptName) {
#ifndef _DEBUG
    try {
#endif
      try {
        // create a script environment and load the script into it
        env = CreateScriptEnvironment();
        if (!env) return false;
      }
      catch (AvisynthError error) {
        error_msg = error.msg;
        return false;
      }
      try {
          
#if 1 // needed for Linux
        std::string strNewWorkingDirectory = GetScriptDirectoryAndFilename(szScriptName);
        AVSValue return_val = env->Invoke("SetWorkingDir", strNewWorkingDirectory.c_str());
        if(0 != return_val.AsInt())
            throw AvisynthError("Failed setting the script directory as working directory");
#endif 
            
        return_val = env->Invoke("Import", szScriptName);
        // store the script's return value (a video clip)
        if (return_val.IsClip()) {

          // Allow WAVE_FORMAT_IEEE_FLOAT audio output
          bool AllowFloatAudio = false;

          try {
            AVSValue v = env->GetVar("OPT_AllowFloatAudio");
            AllowFloatAudio = v.IsBool() ? v.AsBool() : false;
          }
          catch (IScriptEnvironment::NotFound) { }

          filter_graph = return_val.AsClip();

          if (!AllowFloatAudio) // Ensure samples are int     
            filter_graph = ConvertAudio::Create(filter_graph, SAMPLE_INT8|SAMPLE_INT16|SAMPLE_INT24|SAMPLE_INT32, SAMPLE_INT16);

          filter_graph = Cache::Create_Cache(AVSValue(filter_graph), 0, env).AsClip();

          filter_graph->SetCacheHints(CACHE_ALL, 999); // Give the top level cache a big head start!!
        }
        else
          throw AvisynthError("The script's return value was not a video clip");

        if (!filter_graph)
          throw AvisynthError("The returned video clip was nil (this is a bug)");

        // get information about the clip
        vi = &filter_graph->GetVideoInfo();
/**** FORCED CONVERSIONS FOR NOW - ENABLE WHEN IMPLEMENTED  ****/
/*
        if (vi->IsYV16() || vi->IsYV411()) {
          AVSValue args[1] = { filter_graph };
          filter_graph = env->Invoke("ConvertToYUY2", AVSValue(args,1)).AsClip();
          vi = &filter_graph->GetVideoInfo();
        }
        if (vi->IsYV24()) {
          AVSValue args[1] = { filter_graph };
          filter_graph = env->Invoke("ConvertToRGB32", AVSValue(args,1)).AsClip();
          vi = &filter_graph->GetVideoInfo();
        }
*/
        if (vi->IsYV12()&&(vi->width&3))
          throw AvisynthError("Avisynth error: YV12 images for output must have a width divisible by 4 (use crop)!");
        if (vi->IsYUY2()&&(vi->width&3))
          throw AvisynthError("Avisynth error: YUY2 images for output must have a width divisible by 4 (use crop)!");
      }
      catch (AvisynthError error) {
        error_msg = error.msg;
        AVSValue args[2] = { error.msg, 0xff3333 };
        static const char* arg_names[2] = { 0, "text_color" };
        try {
	  if(ErrorHandlingExternal::IsErrorHandlingExternal())
	      throw error;
	  else
	  {
	    filter_graph = env->Invoke("MessageClip", AVSValue(args, 2), arg_names).AsClip();
	    vi = &filter_graph->GetVideoInfo();
	  }
        }
        catch (AvisynthError) {
	  if(ErrorHandlingExternal::IsErrorHandlingExternal())
	    throw;
	  else
	    filter_graph = 0;
        }
      }

      if (szScriptName)
        delete[] szScriptName;
      szScriptName = 0;
      
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
      _clear87();
      __asm {emms};
      _control87( fp_state, 0xffffffff );
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
      
      return true;
#ifndef _DEBUG
    }
    catch (...) {
      AVXLOG_ALERT("%s", "DelayInit() caught general exception!");
      if(ErrorHandlingExternal::IsErrorHandlingExternal())
	throw;
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
      _clear87();
      __asm {emms};
      _control87( fp_state, 0xffffffff );
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
      return false;
    }
#endif
  } else {
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
    _clear87();
    __asm {emms};
    _control87( fp_state, 0xffffffff );
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE

    return (env && filter_graph && vi);
  }
}


void CAVIFileSynth::MakeErrorStream(const char* msg) {
  error_msg = msg;
  filter_graph = Create_MessageClip(msg, vi->width, vi->height, vi->pixel_type, false, 0xFF3333, 0, 0, env);
}

void CAVIFileSynth::Lock() {
  
  EnterCriticalSection(&cs_filter_graph);

}

void CAVIFileSynth::Unlock() {
  
  LeaveCriticalSection(&cs_filter_graph);

}

///////////////////////////////////////////////////
//////////// IAVIFile

STDMETHODIMP CAVIFileSynth::Info(AVIFILEINFOW *pfi, LONG lSize) {

	AVXLOG_INFO("%p->CAVIFileSynth::Info(pfi, %d)", this, lSize);

	if (!pfi) return E_POINTER;

	if (!DelayInit()) return E_FAIL;

	AVIFILEINFOW afi;

    memset(&afi, 0, sizeof(afi));

	afi.dwMaxBytesPerSec	= 0;
	afi.dwFlags				= AVIFILEINFO_HASINDEX | AVIFILEINFO_ISINTERLEAVED;
	afi.dwCaps				= AVIFILECAPS_CANREAD | AVIFILECAPS_ALLKEYFRAMES | AVIFILECAPS_NOCOMPRESSION;
	
	int nrStreams=0;
	if (vi->HasVideo()==true)	nrStreams=1;
	if (vi->HasAudio()==true)	nrStreams++;

	afi.dwStreams				= nrStreams;
	afi.dwSuggestedBufferSize	= 0;
	afi.dwWidth					= vi->width;
	afi.dwHeight				= vi->height;
	afi.dwEditCount				= 0;

    afi.dwRate					= vi->fps_numerator;
	afi.dwScale					= vi->fps_denominator;
	afi.dwLength				= vi->num_frames;

	wcscpy((wchar_t*)afi.szFileType, L"Avisynth");

// Maybe should return AVIERR_BUFFERTOOSMALL for lSize < sizeof(afi)
    memset(pfi, 0, lSize);
    memcpy(pfi, &afi, std::min(size_t(lSize), sizeof(afi)));
	return S_OK;
}

static inline char BePrintable(int ch) {
  ch &= 0xff;
  return isprint(ch) ? ch : '.';
}


STDMETHODIMP CAVIFileSynth::GetStream(PAVISTREAM *ppStream, DWORD fccType, LONG lParam) {
	CAVIStreamSynth *casr;
	char fcc[5];

	fcc[0] = BePrintable(fccType      );
	fcc[1] = BePrintable(fccType >>  8);
	fcc[2] = BePrintable(fccType >> 16);
	fcc[3] = BePrintable(fccType >> 24);
	fcc[4] = 0;

	AVXLOG_INFO("%p->CAVIFileSynth::GetStream(*, %08x(%s), %ld)", this, fccType, fcc, lParam);

	if (!DelayInit()) return E_FAIL;

    *ppStream = NULL;

	if (!fccType) 
	{
// Maybe an Option to set the order of stream discovery
		if ((lParam==0) && (vi->HasVideo()) )
			fccType = streamtypeVIDEO;
		else 
		  if ( ((lParam==1) && (vi->HasVideo())) ||  ((lParam==0) && vi->HasAudio()) )
		  {
			lParam=0;
			fccType = streamtypeAUDIO;
		  }
	}

	if (lParam > 0) return AVIERR_NODATA;

	if (fccType == streamtypeVIDEO) {
		if (!vi->HasVideo())
			return AVIERR_NODATA;

		if ((casr = new CAVIStreamSynth(this, false)) == 0)
		  return AVIERR_MEMORY;

		*ppStream = (IAVIStream *)casr;

	} else if (fccType == streamtypeAUDIO) {
		if (!vi->HasAudio())
			return AVIERR_NODATA;

		if ((casr = new CAVIStreamSynth(this, true)) == 0)
			return AVIERR_MEMORY;

		*ppStream = (IAVIStream *)casr;
	} else
		return AVIERR_NODATA;

	return S_OK;
}


////////////////////////////////////////////////////////////////////////
//////////// IAvisynthClipInfo

int __stdcall CAVIFileSynth::GetError(const char** ppszMessage) {
  if (!DelayInit() && !error_msg)
    error_msg = "Avisynth: script open failed!";

  if (ppszMessage)
    *ppszMessage = error_msg;
  return !!error_msg;
}

bool __stdcall CAVIFileSynth::GetParity(int n) {
  if (!DelayInit())
    return false;
  return filter_graph->GetParity(n);
}

bool __stdcall CAVIFileSynth::IsFieldBased() {
  if (!DelayInit())
    return false;
  return vi->IsFieldBased();
}

STDMETHODIMP_(ULONG) CAVIFileSynth::Release() {
#if 0
    const int refs = InterlockedDecrement(&m_refs);
    InterlockedDecrement(&gRefCnt);

    _RPT3(0,"%p->CAVIFileSynth::Release() gRefCnt=%d, m_refs=%d\n", this, gRefCnt, refs);

    if (!refs) delete this;
    return refs;
#else
    delete this;
    return 0;
#endif
}
////////////////////////////////////////////////////////////////////////
//
//		CAVIStreamSynth
//
////////////////////////////////////////////////////////////////////////
//////////// IAVIStreaming

STDMETHODIMP CAVIStreamSynth::Begin(LONG lStart, LONG lEnd, LONG lRate) {
	AVXLOG_INFO("%p->CAVIStreamSynth::Begin(%ld, %ld, %ld) (%s)", this, lStart, lEnd, lRate, sName);
	return S_OK;
}

STDMETHODIMP CAVIStreamSynth::End() {
	AVXLOG_INFO("%p->CAVIStreamSynth::End() (%s)", this, sName);
	return S_OK;
}

//////////// IAVIStream

STDMETHODIMP CAVIStreamSynth::Create(LONG lParam1, LONG lParam2) {
	AVXLOG_INFO("%p->CAVIStreamSynth::Create()", this);
	return AVIERR_READONLY;
}

STDMETHODIMP CAVIStreamSynth::Delete(LONG lStart, LONG lSamples) {
	AVXLOG_INFO("%p->CAVIStreamSynth::Delete()", this);
	return AVIERR_READONLY;
}

STDMETHODIMP CAVIStreamSynth::ReadData(DWORD fcc, LPVOID lp, LONG *lpcb) {
	AVXLOG_INFO("%p->CAVIStreamSynth::ReadData()", this);
	return AVIERR_NODATA;
}

STDMETHODIMP CAVIStreamSynth::SetFormat(LONG lPos, LPVOID lpFormat, LONG cbFormat) {
	AVXLOG_INFO("%p->CAVIStreamSynth::SetFormat()", this);
	return AVIERR_READONLY;
}

STDMETHODIMP CAVIStreamSynth::WriteData(DWORD fcc, LPVOID lpBuffer, LONG cbBuffer) {
	AVXLOG_INFO("%p->CAVIStreamSynth::WriteData()", this);
	return AVIERR_READONLY;
}

STDMETHODIMP CAVIStreamSynth::SetInfo(AVISTREAMINFOW *psi, LONG lSize) {
	AVXLOG_INFO("%p->CAVIStreamSynth::SetInfo()", this);
	return AVIERR_READONLY;
}

////////////////////////////////////////////////////////////////////////
//////////// local

CAVIStreamSynth::CAVIStreamSynth(CAVIFileSynth *parentPtr, bool isAudio) {

  sName = (char*)(isAudio ? "audio" : "video");

  AVXLOG_INFO("%p->CAVIStreamSynth(%s)", this, sName);

  parent			= parentPtr;
  fAudio			= isAudio;
}

CAVIStreamSynth::~CAVIStreamSynth() {
  AVXLOG_INFO("%p->~CAVIStreamSynth() (%s), gRefCnt = %d", this, sName, gRefCnt);
}

////////////////////////////////////////////////////////////////////////
//////////// IAVIStream

STDMETHODIMP_(LONG) CAVIStreamSynth::Info(AVISTREAMINFOW *psi, LONG lSize) {
	AVXLOG_INFO("%p->CAVIStreamSynth::Info(%p, %ld) (%s)", this, psi, lSize, sName);

	if (!psi) return E_POINTER;

	AVISTREAMINFOW asi;

    const VideoInfo* const vi = parent->vi;

    memset(&asi, 0, sizeof(asi));
    asi.fccType = fAudio ? streamtypeAUDIO : streamtypeVIDEO;
    asi.dwQuality = DWORD(-1);
    if (fAudio) {
      asi.fccHandler = 0;
      int bytes_per_sample = vi->BytesPerAudioSample();
      asi.dwScale = bytes_per_sample;
      asi.dwRate = vi->audio_samples_per_second * bytes_per_sample;
      asi.dwLength = (unsigned long)vi->num_audio_samples;
      asi.dwSampleSize = bytes_per_sample;
      wcscpy((wchar_t*)asi.szName, L"Avisynth audio #1");
    } else {
      const int image_size = vi->BMPSize();
      asi.fccHandler = MAKEDWORD('U','N','K','N');
      if (vi->IsRGB()) 
        asi.fccHandler = MAKEDWORD(' ','B','I','D');
      else if (vi->IsYUY2())
        asi.fccHandler = MAKEDWORD('2','Y','U','Y');
      else if (vi->IsYV12())
        asi.fccHandler = MAKEDWORD('2','1','V','Y'); 
/* For 2.6
      else if (vi->IsY8())
        asi.fccHandler = '008Y'; 
      else if (vi->IsYV24())
        asi.fccHandler = '42VY'; 
      else if (vi->IsYV16()) 
        asi.fccHandler = '61VY'; 
      else if (vi->IsYV411()) 
        asi.fccHandler = 'B14Y'; 
*/
      else {
        _ASSERT(FALSE);
      }

      asi.dwScale = vi->fps_denominator;
      asi.dwRate = vi->fps_numerator;
      asi.dwLength = vi->num_frames;
      asi.rcFrame.right = vi->width;
      asi.rcFrame.bottom = vi->height;
      asi.dwSampleSize = image_size;
      asi.dwSuggestedBufferSize = image_size;
      wcscpy((wchar_t*)asi.szName, L"Avisynth video #1");
    }

// Maybe should return AVIERR_BUFFERTOOSMALL for lSize < sizeof(asi)
    memset(psi, 0, lSize);
    memcpy(psi, &asi, min(size_t(lSize), sizeof(asi)));
	return S_OK;
}

STDMETHODIMP_(LONG) CAVIStreamSynth::FindSample(LONG lPos, LONG lFlags) {
//	AVXLOG_INFO("%p->CAVIStreamSynth::FindSample(%ld, %08lx)", this, lPos, lFlags);

	if (lFlags & FIND_FORMAT)
		return -1;

	if (lFlags & FIND_FROM_START)
		return 0;

	return lPos;
}


////////////////////////////////////////////////////////////////////////
//////////// local

void CAVIStreamSynth::ReadFrame(void* lpBuffer, int n) {
  PVideoFrame frame = parent->filter_graph->GetFrame(n, parent->env);
  if (!frame)
    parent->env->ThrowError("Avisynth error: generated video frame was nil (this is a bug)");

  VideoInfo vi = parent->filter_graph->GetVideoInfo();
  const int pitch    = frame->GetPitch();
  const int row_size = frame->GetRowSize();
  const int height   = frame->GetHeight();

  // BMP scanlines are always dword-aligned
  const int out_pitch = (row_size+3) & -4;
  int out_pitchUV = (frame->GetRowSize(PLANAR_U)+3) & -4;
  if (vi.IsYV12()) {  // We know this has special alignment.
    out_pitchUV = out_pitch / 2;
  }

  BitBlt((BYTE*)lpBuffer, out_pitch, frame->GetReadPtr(), pitch, row_size, height);

  BitBlt((BYTE*)lpBuffer + (out_pitch*height),
         out_pitchUV,               frame->GetReadPtr(PLANAR_V),
		 frame->GetPitch(PLANAR_V), frame->GetRowSize(PLANAR_V),
		 frame->GetHeight(PLANAR_V) );

  BitBlt((BYTE*)lpBuffer + (out_pitch*height + frame->GetHeight(PLANAR_V)*out_pitchUV),
         out_pitchUV,               frame->GetReadPtr(PLANAR_U),
		 frame->GetPitch(PLANAR_U), frame->GetRowSize(PLANAR_U),
		 frame->GetHeight(PLANAR_U) );
}

#define OPT_OWN_SEH_HANDLER
#ifdef OPT_OWN_SEH_HANDLER
EXCEPTION_DISPOSITION __cdecl _Exp_except_handler2(struct _EXCEPTION_RECORD *ExceptionRecord, void * EstablisherFrame,
												  struct _CONTEXT *ContextRecord, void * DispatcherContext)
{
  struct Est_Frame {  // My extended EXCEPTION_REGISTRATION record
	void	  *prev;
	void	  *handler;
	unsigned  *retarg;	  // pointer where to stash exception code
  };

  if (ExceptionRecord->ExceptionFlags == 0)	{  // First pass?
	(((struct Est_Frame *)EstablisherFrame)->retarg)[0] = ExceptionRecord->ExceptionCode;
	(((struct Est_Frame *)EstablisherFrame)->retarg)[1] = (size_t)ExceptionRecord->ExceptionAddress;
	if (ExceptionRecord->NumberParameters >= 2)	{  // Extra Info?
	  (((struct Est_Frame *)EstablisherFrame)->retarg)[2] = (size_t)ExceptionRecord->ExceptionInformation[0];
	  (((struct Est_Frame *)EstablisherFrame)->retarg)[3] = (size_t)ExceptionRecord->ExceptionInformation[1];
	}
  }
  return ExceptionContinueSearch;
}

void CAVIStreamSynth::ReadHelper(void* lpBuffer, int lStart, int lSamples, unsigned code[4]) {

  DWORD handler = (size_t)_Exp_except_handler2;

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
  __asm { // Build EXCEPTION_REGISTRATION record:
  push  code        // Address of return argument
  push  handler     // Address of handler function
  push  FS:[0]      // Address of previous handler
  mov   FS:[0],esp  // Install new EXCEPTION_REGISTRATION
  }
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
  if (fAudio)
    parent->filter_graph->GetAudio(lpBuffer, lStart, lSamples, parent->env);
  else
    ReadFrame(lpBuffer, lStart);
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
 __asm { // Remove our EXCEPTION_REGISTRATION record
  mov   eax,[esp]   // Get pointer to previous record
  mov   FS:[0], eax // Install previous record
  add   esp, 12     // Clean our EXCEPTION_REGISTRATION off stack
  }
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
}

static const char * const StringSystemError2(const unsigned code)
{
  switch (code) {
  case STATUS_GUARD_PAGE_VIOLATION:      // 0x80000001
    return "Guard Page Violation";
  case STATUS_DATATYPE_MISALIGNMENT:     // 0x80000002
    return "Datatype Misalignment";
  case STATUS_BREAKPOINT:                // 0x80000003
    return "Breakpoint";
  case STATUS_SINGLE_STEP:               // 0x80000004
    return "Single Step";
  default:
    break;
  }
  
  switch (code) {
  case STATUS_ACCESS_VIOLATION:          // 0xc0000005
    return "*Access Violation";
  case STATUS_IN_PAGE_ERROR:             // 0xc0000006
    return "In Page Error";
  case STATUS_INVALID_HANDLE:            // 0xc0000008
    return "Invalid Handle";
  case STATUS_NO_MEMORY:                 // 0xc0000017
    return "No Memory";
  case STATUS_ILLEGAL_INSTRUCTION:       // 0xc000001d
    return "Illegal Instruction";
  case STATUS_NONCONTINUABLE_EXCEPTION:  // 0xc0000025
    return "Noncontinuable Exception";
  case STATUS_INVALID_DISPOSITION:       // 0xc0000026
    return "Invalid Disposition";
  case STATUS_ARRAY_BOUNDS_EXCEEDED:     // 0xc000008c
    return "Array Bounds Exceeded";
  case STATUS_FLOAT_DENORMAL_OPERAND:    // 0xc000008d
    return "Float Denormal Operand";
  case STATUS_FLOAT_DIVIDE_BY_ZERO:      // 0xc000008e
    return "Float Divide by Zero";
  case STATUS_FLOAT_INEXACT_RESULT:      // 0xc000008f
    return "Float Inexact Result";
  case STATUS_FLOAT_INVALID_OPERATION:   // 0xc0000090
    return "Float Invalid Operation";
  case STATUS_FLOAT_OVERFLOW:            // 0xc0000091
    return "Float Overflow";
  case STATUS_FLOAT_STACK_CHECK:         // 0xc0000092
    return "Float Stack Check";
  case STATUS_FLOAT_UNDERFLOW:           // 0xc0000093
    return "Float Underflow";
  case STATUS_INTEGER_DIVIDE_BY_ZERO:    // 0xc0000094
    return "Integer Divide by Zero";
  case STATUS_INTEGER_OVERFLOW:          // 0xc0000095
    return "Integer Overflow";
  case STATUS_PRIVILEGED_INSTRUCTION:    // 0xc0000096
    return "Privileged Instruction";
  case STATUS_STACK_OVERFLOW:            // 0xc00000fd
    return "Stack Overflow";
  default:
    break;
  }
  return 0;
}
#else
void CAVIStreamSynth::ReadHelper(void* lpBuffer, int lStart, int lSamples, unsigned &xcode[4]) {
  // It's illegal to call GetExceptionInformation() inside an __except
  // block!  Hence this variable and the horrible hack below...
#ifndef _DEBUG
  EXCEPTION_POINTERS* ei;
  DWORD code;
  __try { 
#endif
    if (fAudio)
      parent->filter_graph->GetAudio(lpBuffer, lStart, lSamples, parent->env);
    else
      ReadFrame(lpBuffer, lStart);
#ifndef _DEBUG
  }
  __except (ei = GetExceptionInformation(), code = GetExceptionCode(), (code >> 28) == 0xC) {
    switch (code) {
    case EXCEPTION_ACCESS_VIOLATION:
      parent->env->ThrowError("Avisynth: caught an access violation at 0x%08x,\nattempting to %s 0x%08x",
        ei->ExceptionRecord->ExceptionAddress,
        ei->ExceptionRecord->ExceptionInformation[0] ? "write to" : "read from",
        ei->ExceptionRecord->ExceptionInformation[1]);
    case EXCEPTION_ILLEGAL_INSTRUCTION:
      parent->env->ThrowError("Avisynth: illegal instruction at 0x%08x",
        ei->ExceptionRecord->ExceptionAddress);
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
      parent->env->ThrowError("Avisynth: division by zero at 0x%08x",
        ei->ExceptionRecord->ExceptionAddress);
    case EXCEPTION_STACK_OVERFLOW:
      throw AvisynthError("Avisynth: stack overflow");
    default:
      parent->env->ThrowError("Avisynth: unknown exception 0x%08x at 0x%08x",
        code, ei->ExceptionRecord->ExceptionAddress);
    }
  }
#endif
}
#endif

////////////////////////////////////////////////////////////////////////
//////////// IAVIStream

STDMETHODIMP CAVIStreamSynth::Read(LONG lStart, LONG lSamples, LPVOID lpBuffer, LONG cbBuffer, LONG *plBytes, LONG *plSamples) {

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
  __asm { // Force compiler to protect these registers!
    mov ebx,ebx;
    mov esi,esi;
    mov edi,edi;
  }
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE

  parent->Lock();

  HRESULT result = Read2(lStart, lSamples, lpBuffer, cbBuffer, plBytes, plSamples);

  parent->Unlock();

  return result;
}

HRESULT CAVIStreamSynth::Read2(LONG lStart, LONG lSamples, LPVOID lpBuffer, LONG cbBuffer, LONG *plBytes, LONG *plSamples) {

  AVXLOG_DEBUG("%p->CAVIStreamSynth::Read(%ld samples at %ld)", this, lSamples, lStart);
  AVXLOG_DEBUG("\tbuffer: %ld bytes at %p", cbBuffer, lpBuffer);

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
  int fp_state = 87( 0, 0 );
  _control87( FP_STATE, 0xffffffff );
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
  
  unsigned code[4] = {0, 0, 0, 0};

  if (fAudio) {
    // buffer overflow patch -- Avery Lee - Mar 2006
    if (lSamples == AVISTREAMREAD_CONVENIENT)
      lSamples = (long)parent->vi->AudioSamplesFromFrames(1);

    if (__int64(lStart)+lSamples > parent->vi->num_audio_samples) {
      lSamples = (long)(parent->vi->num_audio_samples - lStart);
      if (lSamples < 0) lSamples = 0;
    }

    long bytes = (long)parent->vi->BytesFromAudioSamples(lSamples);
    if (lpBuffer && bytes > cbBuffer) {
      lSamples = (long)parent->vi->AudioSamplesFromBytes(cbBuffer);
      bytes = (long)parent->vi->BytesFromAudioSamples(lSamples);
    }
    if (plBytes) *plBytes = bytes;
    if (plSamples) *plSamples = lSamples;
    if (!lpBuffer || !lSamples)
      return S_OK;

  } else {
    if (lStart >= parent->vi->num_frames) {
      if (plSamples) *plSamples = 0;
      if (plBytes) *plBytes = 0;
      return S_OK;
    }

    int image_size = parent->vi->BMPSize();
    if (plSamples) *plSamples = 1;
    if (plBytes) *plBytes = image_size;

    if (!lpBuffer) {
      return S_OK;
    } else if (cbBuffer < image_size) {
      AVXLOG_WARN("Buffer too small; should be %ld samples", image_size);
      return AVIERR_BUFFERTOOSMALL;
    }
  }

#ifndef _DEBUG
  try {
    try {
#endif
      // VC compiler says "only one form of exception handling permitted per
      // function."  Sigh...
      ReadHelper(lpBuffer, lStart, lSamples, code);
#ifndef _DEBUG
    }
    catch (AvisynthError error) {
      parent->MakeErrorStream(error.msg);
      if (fAudio)
	    throw;
	  else
	    ReadHelper(lpBuffer, lStart, lSamples, code);
    }
    catch (...) {
#ifdef OPT_OWN_SEH_HANDLER
      if (code[0] == 0xE06D7363) {
	    parent->MakeErrorStream("Avisynth: unhandled C++ exception");
	  }
      else if (code[0]) {
		char buf[128];
        const char * const extext = StringSystemError2(code[0]);
        if (extext) {
		  if (extext[0] == '*') {
			const char * const rwtext = code[2] ? "writing to" : "reading from";
			snprintf(buf, 127, "CAVIStreamSynth: System exception - %s at 0x%x, %s 0x%x", extext+1, code[1], rwtext, code[3]);
		  }
		  else
			snprintf(buf, 127, "CAVIStreamSynth: System exception - %s at 0x%x", extext, code[1]);
		}
        else {
          snprintf(buf, 127, "CAVIStreamSynth: Unknown system exception - 0x%x at 0x%x", code[0], code[1]);
        }
		parent->MakeErrorStream(parent->env->SaveString(buf));
      }
      else parent->MakeErrorStream("Avisynth: unknown exception");
      code[0] = 0;
#else
      parent->MakeErrorStream("Avisynth: unknown exception");
#endif
      if (fAudio)
	    throw;
	  else
        ReadHelper(lpBuffer, lStart, lSamples, code);
    }
  }
  catch (...) {
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
    _clear87();
    __asm {emms};
    _control87( fp_state, 0xffffffff );
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
    return E_FAIL;
  }
#endif

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
  _clear87();
    __asm {emms};
  _control87( fp_state, 0xffffffff );
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
  return S_OK;
}

STDMETHODIMP CAVIStreamSynth::ReadFormat(LONG lPos, LPVOID lpFormat, LONG *lpcbFormat) {
  AVXLOG_INFO("%p->CAVIStreamSynth::ReadFormat() (%s)", this, sName);

  if (!lpcbFormat) return E_POINTER;

  bool UseWaveExtensible = false;
  try {
	AVSValue v = parent->env->GetVar("OPT_UseWaveExtensible");
	UseWaveExtensible = v.IsBool() ? v.AsBool() : false;
  }
  catch (IScriptEnvironment::NotFound) { }

  if (!lpFormat) {
    *lpcbFormat = fAudio ? ( UseWaveExtensible ? sizeof(WAVEFORMATEXTENSIBLE)
                                               : sizeof(WAVEFORMATEX) )
                         : sizeof(BITMAPINFOHEADER);
    return S_OK;
  }

  memset(lpFormat, 0, *lpcbFormat);

  const VideoInfo* const vi = parent->vi;

  if (fAudio) {
	if (UseWaveExtensible) {  // Use WAVE_FORMAT_EXTENSIBLE audio output format 
	  //const GUID KSDATAFORMAT_SUBTYPE_PCM       = {0x00000001, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
	  //const GUID KSDATAFORMAT_SUBTYPE_IEEE_FLOAT= {0x00000003, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
	  WAVEFORMATEXTENSIBLE wfxt;

	  memset(&wfxt, 0, sizeof(wfxt));
	  wfxt.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	  wfxt.Format.nChannels = vi->AudioChannels();
	  wfxt.Format.nSamplesPerSec = vi->SamplesPerSecond();
	  wfxt.Format.wBitsPerSample = vi->BytesPerChannelSample() * 8;
	  wfxt.Format.nBlockAlign = vi->BytesPerAudioSample();
	  wfxt.Format.nAvgBytesPerSec = wfxt.Format.nSamplesPerSec * wfxt.Format.nBlockAlign;
	  wfxt.Format.cbSize = sizeof(wfxt) - sizeof(wfxt.Format);
	  wfxt.Samples.wValidBitsPerSample = wfxt.Format.wBitsPerSample;

	  const int SpeakerMasks[8] = { 0,
	    0x00004, // 1         Cf
		0x00003, // 2   Lf Rf
		0x00007, // 3   Lf Rf Cf
		0x00033, // 4   Lf Rf       Lr Rr
		0x00037, // 5   Lf Rf Cf    Lr Rr
		0x0003F, // 5.1 Lf Rf Cf Sw Lr Rr
		0x0013F, // 6.1 Lf Rf Cf Sw Lr Rr Cr
	  };
	  wfxt.dwChannelMask = (unsigned)vi->AudioChannels() <= 7 ? SpeakerMasks[vi->AudioChannels()]
	                     : (unsigned)vi->AudioChannels() <=18 ? DWORD(-1) >> (32-vi->AudioChannels())
						 : SPEAKER_ALL;

	  wfxt.SubFormat = vi->IsSampleType(SAMPLE_FLOAT) ? KSDATAFORMAT_SUBTYPE_IEEE_FLOAT : KSDATAFORMAT_SUBTYPE_PCM;
	  *lpcbFormat = std::min(*lpcbFormat, (LONG)sizeof(wfxt));
	  memcpy(lpFormat, &wfxt, size_t(*lpcbFormat));
	}
	else {
	  WAVEFORMATEX wfx;
	  memset(&wfx, 0, sizeof(wfx));
	  wfx.wFormatTag = vi->IsSampleType(SAMPLE_FLOAT) ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM;
	  wfx.nChannels = vi->AudioChannels();
	  wfx.nSamplesPerSec = vi->SamplesPerSecond();
	  wfx.wBitsPerSample = vi->BytesPerChannelSample() * 8;
	  wfx.nBlockAlign = vi->BytesPerAudioSample();
	  wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
	  *lpcbFormat = std::min(*lpcbFormat, (LONG)sizeof(wfx));
	  memcpy(lpFormat, &wfx, size_t(*lpcbFormat));
	}
  } else {
    BITMAPINFOHEADER bi;
    memset(&bi, 0, sizeof(bi));
    bi.biSize = sizeof(bi);
    bi.biWidth = vi->width;
    bi.biHeight = vi->height;
    bi.biPlanes = 1;
    bi.biBitCount = vi->BitsPerPixel();
      if (vi->IsRGB()) 
        bi.biCompression = BI_RGB;
      else if (vi->IsYUY2())
        bi.biCompression = MAKEDWORD('2','Y','U','Y');
      else if (vi->IsYV12())
        bi.biCompression = MAKEDWORD('2','1','V','Y');
/* For 2.6
      else if (vi->IsY8())
        bi.biCompression = '008Y'; 
      else if (vi->IsYV24())
        bi.biCompression = '42VY'; 
      else if (vi->IsYV16()) 
        bi.biCompression = '61VY'; 
      else if (vi->IsYV411()) 
        bi.biCompression = 'B14Y'; 
*/
      else {
        _ASSERT(FALSE);
      }
    bi.biSizeImage = vi->BMPSize();
    *lpcbFormat = std::min(*lpcbFormat, (LONG)sizeof(bi));
    memcpy(lpFormat, &bi, size_t(*lpcbFormat));
  }
  return S_OK;
}

STDMETHODIMP CAVIStreamSynth::Write(LONG lStart, LONG lSamples, LPVOID lpBuffer,
	LONG cbBuffer, DWORD dwFlags, LONG* plSampWritten, 
    LONG* plBytesWritten) {

	AVXLOG_INFO("%p->CAVIStreamSynth::Write()", this);

	return AVIERR_READONLY;
}

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
HRESULT CreateAVISynth(IAVXSynth **ppIAVXSynth) {
	HRESULT hresult = S_OK;

	AVXLOG_INFO("%s", "CAVIFileSynth::Create()");

	CAVIFileSynth* pAVIFileSynth = new CAVIFileSynth();

	if (!pAVIFileSynth) return E_OUTOFMEMORY;

	AVXLOG_INFO("CAVIFileSynth::Create() exit, result=0x%X", hresult);

	*ppIAVXSynth = (IAVXSynth*)pAVIFileSynth;
	return hresult;
}

void DeleteAVISynth(IAVXSynth *pIAVXSynth)
{
    if(pIAVXSynth)
        pIAVXSynth->Release();
}

#ifdef __cplusplus
}
#endif // __cplusplus

}; // namespace avxsynth
