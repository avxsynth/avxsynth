#ifndef __WINGDI_LINUX_H__
#define __WINGDI_LINUX_H__

namespace avxsynth {

/////////////////////////////////
// Copied from Windows WinGDI.h
/////////////////////////////////


typedef struct tagRGBQUAD { 
  BYTE rgbBlue;
  BYTE rgbGreen;
  BYTE rgbRed;
  BYTE rgbReserved;
} RGBQUAD;    
    
typedef struct
{

    DWORD      biSize;
    LONG       biWidth;
    LONG       biHeight;
    WORD       biPlanes;
    WORD       biBitCount;
    DWORD      biCompression;
    DWORD      biSizeImage;
    LONG       biXPelsPerMeter;
    LONG       biYPelsPerMeter;
    DWORD      biClrUsed;
    DWORD      biClrImportant;

} BITMAPINFOHEADER, *LPBITMAPINFOHEADER;

typedef struct tagBITMAPINFO {
  BITMAPINFOHEADER bmiHeader;
  RGBQUAD          bmiColors[1];
} BITMAPINFO, *PBITMAPINFO;

#define BI_RGB        (0L)
#define RGB           (0x32424752)
#define RGBA          (0x41424752)
#define RGBT          (0x54424752)
#define BI_RLE8       (1L)
#define RLE8          (0x38454C52)
#define BI_RLE4       (2L)
#define RLE           (0x34454C52)
#define BI_BITFIELDS  (3L)
#define BI_RAW        (0x32776173)
#define BI_JPEG       (4L)
#define BI_PNG        (5L)



}; // namespace avxsynth

#endif //  __WINGDI_LINUX_H__
