#ifndef __MMREG_LINUX_H__
#define __MMREG_LINUX_H__

namespace avxsynth {

/////////////////////////////////
// Copied from Windows MMReg.h
/////////////////////////////////

/* WAVE form wFormatTag IDs */
#define  WAVE_FORMAT_PCM                        1
#define  WAVE_FORMAT_IEEE_FLOAT                 0x0003 /* Microsoft Corporation */
#define  WAVE_FORMAT_MPEGLAYER3                 0x0055 /* ISO/MPEG Layer3 Format Tag */

typedef struct tWAVEFORMATEX
{
    WORD    wFormatTag;        /* format type */
    WORD    nChannels;         /* number of channels (i.e. mono, stereo...) */
    DWORD   nSamplesPerSec;    /* sample rate */
    DWORD   nAvgBytesPerSec;   /* for buffer estimation */
    WORD    nBlockAlign;       /* block size of data */
    WORD    wBitsPerSample;    /* Number of bits per sample of mono data */
    WORD    cbSize;            /* The count in bytes of the size of
                                    extra information (after cbSize) */
} WAVEFORMATEX, *LPWAVEFORMATEX;

#define SPEAKER_ALL                     0x80000000

static GUID KSDATAFORMAT_SUBTYPE_PCM = { 0x00000001, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
//{"-0000-0010-8000-00aa00389b71"}

static GUID KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = { 0x00000003, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
//("00000003-0000-0010-8000-00aa00389b71", );       

#define  WAVE_FORMAT_EXTENSIBLE                 0xFFFE
typedef struct {
    WAVEFORMATEX    Format;
    union {
        WORD wValidBitsPerSample;       /* bits of precision  */
        WORD wSamplesPerBlock;          /* valid if wBitsPerSample==0 */
        WORD wReserved;                 /* If neither applies, set to zero. */
    } Samples;
    DWORD           dwChannelMask;      /* which channels are */
                                        /* present in stream  */
    GUID            SubFormat;
} WAVEFORMATEXTENSIBLE, *PWAVEFORMATEXTENSIBLE;



}; // namespace avxsynth

#endif //  __MMREG_LINUX_H__
