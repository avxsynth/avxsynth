#ifndef __VFW_LINUX_H__
#define __VFW_LINUX_H__

namespace avxsynth {

/////////////////////////////////
// Copied from Windows Vfw.h
/////////////////////////////////

//
// flags for AVIStreamFindSample
//
#define FIND_DIR        0x0000000FL     // direction
#define FIND_NEXT       0x00000001L     // go forward
#define FIND_PREV       0x00000004L     // go backward
#define FIND_FROM_START 0x00000008L     // start at the logical beginning

#define FIND_TYPE       0x000000F0L     // type mask
#define FIND_KEY        0x00000010L     // find key frame.
#define FIND_ANY        0x00000020L     // find any (non-empty) sample
#define FIND_FORMAT     0x00000040L     // find format change

#define FIND_RET        0x0000F000L     // return mask
#define FIND_POS        0x00000000L     // return logical position
#define FIND_LENGTH     0x00001000L     // return logical size
#define FIND_OFFSET     0x00002000L     // return physical position
#define FIND_SIZE       0x00003000L     // return physical size
#define FIND_INDEX      0x00004000L     // return physical index position

typedef struct {
  DWORD dwMaxBytesPerSec;
  DWORD dwFlags;
  DWORD dwCaps;
  DWORD dwStreams;
  DWORD dwSuggestedBufferSize;
  DWORD dwWidth;
  DWORD dwHeight;
  DWORD dwScale;
  DWORD dwRate;
  DWORD dwLength;
  DWORD dwEditCount;
  TCHAR szFileType[64];
} AVIFILEINFO;

typedef struct {
  DWORD dwMaxBytesPerSec;
  DWORD dwFlags;
  DWORD dwCaps;
  DWORD dwStreams;
  DWORD dwSuggestedBufferSize;
  DWORD dwWidth;
  DWORD dwHeight;
  DWORD dwScale;
  DWORD dwRate;
  DWORD dwLength;
  DWORD dwEditCount;
  WCHAR szFileType[64];
} AVIFILEINFOW;

typedef struct _AVISTREAMINFOW {
    DWORD fccType;
    DWORD fccHandler;
    DWORD dwFlags;        /* Contains AVITF_* flags */
    DWORD dwCaps;
    WORD  wPriority;
    WORD  wLanguage;
    DWORD dwScale;
    DWORD dwRate; /* dwRate / dwScale == samples/second */
    DWORD dwStart;
    DWORD dwLength; /* In units above... */
    DWORD dwInitialFrames;
    DWORD dwSuggestedBufferSize;
    DWORD dwQuality;
    DWORD dwSampleSize;
    RECT  rcFrame;
    DWORD dwEditCount;
    DWORD dwFormatChangeCount;
    WCHAR szName[64];
} AVISTREAMINFOW, *LPAVISTREAMINFOW;

typedef struct {
  DWORD  fccType;
  DWORD  fccHandler;
  DWORD  dwKeyFrameEvery;
  DWORD  dwQuality;
  DWORD  dwBytesPerSecond;
  DWORD  dwFlags;
  LPVOID lpFormat;
  DWORD  cbFormat;
  LPVOID lpParms;
  DWORD  cbParms;
  DWORD  dwInterleaveEvery;
} AVICOMPRESSOPTIONS;

typedef long (*AVISAVECALLBACK)(int nPercent);

class IAVIStream: public IUnknown
{
public:
    // *** IUnknown methods ***
    virtual HRESULT QueryInterface (REFIID riid, LPVOID* ppvObj){ return 0;};
    virtual ULONG AddRef (void) { return 0l;};
    virtual ULONG Release (void) { return 0l;};

    // *** IAVIStream methods ***
    virtual HRESULT Create(long* lParam1, long* lParam2) { return 0;};
    virtual LONG Info   (AVISTREAMINFOW * psi, LONG lSize) = 0 ;
    virtual LONG FindSample(LONG lPos, LONG lFlags) = 0 ;
    virtual HRESULT ReadFormat  (LONG lPos,
                LPVOID lpFormat, LONG *lpcbFormat) = 0 ;
    virtual HRESULT SetFormat   (LONG lPos,
                LPVOID lpFormat, LONG cbFormat) = 0 ;
    virtual HRESULT Read        (LONG lStart, LONG lSamples,
                LPVOID lpBuffer, LONG cbBuffer,
                LONG* plBytes, LONG* plSamples) = 0 ;
    virtual HRESULT Write       (LONG lStart, LONG lSamples,
                LPVOID lpBuffer, LONG cbBuffer,
                DWORD dwFlags,
                LONG* plSampWritten,
                LONG* plBytesWritten) = 0 ;
    virtual HRESULT Delete      (LONG lStart, LONG lSamples) = 0;
    virtual HRESULT ReadData    (DWORD fcc, LPVOID lp, LONG* lpcb) = 0 ;
    virtual HRESULT WriteData   (DWORD fcc, LPVOID lp, LONG cb) = 0 ;
#if 1 //def _WIN32
    virtual HRESULT SetInfo (AVISTREAMINFOW* lpInfo,
                LONG cbInfo) = 0;
#else
    virtual HRESULT Reserved1            (void) = 0;
    virtual HRESULT Reserved2            (void) = 0;
    virtual HRESULT Reserved3            (void) = 0;
    virtual HRESULT Reserved4            (void) = 0;
    virtual HRESULT Reserved5            (void) = 0;
#endif
};

typedef IAVIStream* PAVISTREAM;


typedef struct {
  DWORD fccType;
  DWORD fccHandler;
  DWORD dwFlags;
  DWORD dwCaps;
  WORD  wPriority;
  WORD  wLanguage;
  DWORD dwScale;
  DWORD dwRate;
  DWORD dwStart;
  DWORD dwLength;
  DWORD dwInitialFrames;
  DWORD dwSuggestedBufferSize;
  DWORD dwQuality;
  DWORD dwSampleSize;
  RECT  rcFrame;
  DWORD dwEditCount;
  DWORD dwFormatChangeCount;
  TCHAR szName[64];
} AVISTREAMINFO;

class IAVIFile : public IUnknown
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID* ppvObj) = 0;
    STDMETHOD_(ULONG,AddRef) (void)  = 0;
    STDMETHOD_(ULONG,Release) (void) = 0;

    // *** IAVIFile methods ***
    STDMETHOD(Info)                 (AVIFILEINFOW* pfi,
                                     LONG lSize) = 0;
    STDMETHOD(GetStream)            (PAVISTREAM* ppStream,
				     DWORD fccType,
                                     LONG lParam) = 0;
    STDMETHOD(CreateStream)         (PAVISTREAM* ppStream,
                                     AVISTREAMINFOW * psi) = 0;
    STDMETHOD(WriteData)            (DWORD ckid,
                                     LPVOID lpData,
                                     LONG cbData) = 0;
    STDMETHOD(ReadData)             (DWORD ckid,
                                     LPVOID lpData,
                                     LONG *lpcbData) = 0;
    STDMETHOD(EndRecord)            (void) = 0;
    STDMETHOD(DeleteStream)         (DWORD fccType,
                                     LONG lParam) = 0;
};

typedef  IAVIFile* PAVIFILE;

typedef DWORD SCODE;
#define SEVERITY_ERROR 1
#define FACILITY_ITF 4
#define MAKE_SCODE(sev,fac,code) ((SCODE) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))) )
#define MAKE_AVIERR(error) MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x4000 + error)

#define AVIERR_OK 0L
#define AVIERR_UNSUPPORTED MAKE_AVIERR(101)
#define AVIERR_BADFORMAT MAKE_AVIERR(102)
#define AVIERR_MEMORY MAKE_AVIERR(103)
#define AVIERR_INTERNAL MAKE_AVIERR(104)
#define AVIERR_BADFLAGS MAKE_AVIERR(105)
#define AVIERR_BADPARAM MAKE_AVIERR(106)
#define AVIERR_BADSIZE MAKE_AVIERR(107)
#define AVIERR_BADHANDLE MAKE_AVIERR(108)
#define AVIERR_FILEREAD MAKE_AVIERR(109)
#define AVIERR_FILEWRITE MAKE_AVIERR(110)
#define AVIERR_FILEOPEN MAKE_AVIERR(111)
#define AVIERR_COMPRESSOR MAKE_AVIERR(112)
#define AVIERR_NOCOMPRESSOR MAKE_AVIERR(113)
#define AVIERR_READONLY MAKE_AVIERR(114)
#define AVIERR_NODATA MAKE_AVIERR(115)
#define AVIERR_BUFFERTOOSMALL MAKE_AVIERR(116)
#define AVIERR_CANTCOMPRESS MAKE_AVIERR(117)
#define AVIERR_USERABORT MAKE_AVIERR(198)
#define AVIERR_ERROR MAKE_AVIERR(199)

#define AVISTREAMREAD_CONVENIENT	(-1L)

// Flags for dwFlags
#define AVIFILEINFO_HASINDEX		0x00000010
#define AVIFILEINFO_MUSTUSEINDEX	0x00000020
#define AVIFILEINFO_ISINTERLEAVED	0x00000100
#define AVIFILEINFO_WASCAPTUREFILE	0x00010000
#define AVIFILEINFO_COPYRIGHTED		0x00020000

// Flags for dwCaps
#define AVIFILECAPS_CANREAD		0x00000001
#define AVIFILECAPS_CANWRITE		0x00000002
#define AVIFILECAPS_ALLKEYFRAMES	0x00000010
#define AVIFILECAPS_NOCOMPRESSION	0x00000020


#ifndef mmioFOURCC
#define mmioFOURCC( ch0, ch1, ch2, ch3 )				\
		( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |	\
		( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#endif

#define streamtypeVIDEO         mmioFOURCC('v', 'i', 'd', 's')
#define streamtypeAUDIO         mmioFOURCC('a', 'u', 'd', 's')
#define streamtypeMIDI			mmioFOURCC('m', 'i', 'd', 's')
#define streamtypeTEXT          mmioFOURCC('t', 'x', 't', 's')



}; // namespace avxsynth

#endif //  __VFW_LINUX_H__
