// Avisynth v4.0  Copyright 2002 Ben Rudiak-Gould et al.
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


#include "stdafx.h"

#include "../internal.h"
#include "source-private.h"
#include "text-overlay.h"

namespace avxsynth {
	
/********************************************************************
********************************************************************/

enum {
    COLOR_MODE_RGB = 0,
    COLOR_MODE_YUV
};

DWORD GetFileAttributes(LPCTSTR lpFileName)
{
	FILE* fp = fopen(lpFileName, "rb");
	if(NULL == fp)
		return (DWORD)-1;
	
	fclose(fp);
	return 0;
}

class StaticImage : public IClip {
  const VideoInfo vi;
  const PVideoFrame frame;
  bool parity;

public:
  StaticImage(const VideoInfo& _vi, const PVideoFrame& _frame, bool _parity)
    : vi(_vi), frame(_frame), parity(_parity) {}
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env) { return frame; }
  void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) {
    memset(buf, 0, vi.BytesFromAudioSamples(count));
  }
  const VideoInfo& __stdcall GetVideoInfo() { return vi; }
  bool __stdcall GetParity(int n) { return (vi.IsFieldBased() ? (n&1) : false) ^ parity; }
  void __stdcall SetCacheHints(int cachehints,size_t frame_range) { };
};


static PVideoFrame CreateBlankFrame(const VideoInfo& vi, int color, int mode, IScriptEnvironment* env) {

  if (!vi.HasVideo()) return 0;

  PVideoFrame frame = env->NewVideoFrame(vi);
  BYTE* p = frame->GetWritePtr();
  int size = frame->GetPitch() * frame->GetHeight();

  if (vi.IsPlanar()) {
    int color_yuv =(mode == COLOR_MODE_YUV) ? color : RGB2YUV(color);
    int Cval = (color_yuv>>16)&0xff;
    Cval |= (Cval<<8)|(Cval<<16)|(Cval<<24);
    for (int i=0; i<size; i+=4)
      *(unsigned*)(p+i) = Cval;
    p = frame->GetWritePtr(PLANAR_U);
    size = frame->GetPitch(PLANAR_U) * frame->GetHeight(PLANAR_U);
    Cval = (color_yuv>>8)&0xff;
    Cval |= (Cval<<8)|(Cval<<16)|(Cval<<24);
    for (int i=0; i<size; i+=4)
      *(unsigned*)(p+i) = Cval;
    size = frame->GetPitch(PLANAR_V) * frame->GetHeight(PLANAR_V);
    p = frame->GetWritePtr(PLANAR_V);
    Cval = (color_yuv)&0xff;
    Cval |= (Cval<<8)|(Cval<<16)|(Cval<<24);
    for (int i=0; i<size; i+=4)
      *(unsigned*)(p+i) = Cval;
  } else if (vi.IsYUY2()) {
    int color_yuv =(mode == COLOR_MODE_YUV) ? color : RGB2YUV(color);
    unsigned d = ((color_yuv>>16)&255) * 0x010001 + ((color_yuv>>8)&255) * 0x0100 + (color_yuv&255) * 0x01000000;
    for (int i=0; i<size; i+=4)
      *(unsigned*)(p+i) = d;
  } else if (vi.IsRGB24()) {
    const unsigned char clr0 = (color & 0xFF);
    const unsigned short clr1 = (color >> 8);
    const int gr = frame->GetRowSize();
    const int gp = frame->GetPitch();
    for (int y=frame->GetHeight();y>0;y--) {
      for (int i=0; i<gr; i+=3) {
        p[i] = clr0; *(unsigned short*)(p+i+1) = clr1;
      }
      p+=gp;
    }
  } else if (vi.IsRGB32()) {
    for (int i=0; i<size; i+=4)
      *(unsigned*)(p+i) = color;
  }
  return frame;
}

AVSValue __cdecl Create_BlankClip(AVSValue args, void*, IScriptEnvironment* env) {
  VideoInfo vi_default;
  memset(&vi_default, 0, sizeof(VideoInfo));
  vi_default.fps_denominator=1;
  vi_default.fps_numerator=24;
  vi_default.height=480;
  vi_default.pixel_type=VideoInfo::CS_BGR32;
  vi_default.num_frames=240;
  vi_default.width=640;
  vi_default.audio_samples_per_second=44100;
  vi_default.nchannels=1;
  vi_default.num_audio_samples=44100*10;
  vi_default.sample_type=SAMPLE_INT16;
  vi_default.SetFieldBased(false);
  bool parity=false;

  VideoInfo vi;

  if ((args[0].ArraySize() == 1) && (!args[12].Defined())) {
    vi_default = args[0][0].AsClip()->GetVideoInfo();
    parity = args[0][0].AsClip()->GetParity(0);
  }
  else if (args[0].ArraySize() != 0) {
    env->ThrowError("BlankClip: Only 1 Template clip allowed.");
  }
  else if (args[12].Defined()) {
    vi_default = args[12].AsClip()->GetVideoInfo();
    parity = args[12].AsClip()->GetParity(0);
  }

  vi.num_frames = args[1].AsInt(vi_default.num_frames);
  vi.width = args[2].AsInt(vi_default.width);
  vi.height = args[3].AsInt(vi_default.height);

  if (args[4].Defined()) {
    const char* pixel_type_string = args[4].AsString();
    if (!strcasecmp(pixel_type_string, "YUY2")) {
      vi.pixel_type = VideoInfo::CS_YUY2;
    } else if (!strcasecmp(pixel_type_string, "YV12")) {
      vi.pixel_type = VideoInfo::CS_YV12;
/* For 2.6
    } else if (!strcasecmp(pixel_type_string, "YV24")) {
      vi.pixel_type = VideoInfo::CS_YV24;
    } else if (!strcasecmp(pixel_type_string, "YV16")) {
      vi.pixel_type = VideoInfo::CS_YV16;
    } else if (!strcasecmp(pixel_type_string, "Y8")) {
      vi.pixel_type = VideoInfo::CS_Y8;
    } else if (!strcasecmp(pixel_type_string, "YV411")) {
      vi.pixel_type = VideoInfo::CS_YV411;
*/
    } else if (!strcasecmp(pixel_type_string, "RGB24")) {
      vi.pixel_type = VideoInfo::CS_BGR24;
    } else if (!strcasecmp(pixel_type_string, "RGB32")) {
      vi.pixel_type = VideoInfo::CS_BGR32;
    } else {
      env->ThrowError("BlankClip: pixel_type must be \"RGB32\", \"RGB24\", \"YV12\""/*, \"YV24\", \"YV16\", \"Y8\", \"YV411\"*/" or \"YUY2\"");
    }
  }
  else {
    vi.pixel_type = vi_default.pixel_type;
  }

  if (!vi.pixel_type)
    vi.pixel_type = VideoInfo::CS_BGR32;


  double n = args[5].AsFloat(double(vi_default.fps_numerator));

  if (args[5].Defined() && !args[6].Defined()) {
    unsigned d = 1;
    while (n < 16777216 && d < 16777216) { n*=2; d*=2; }
    vi.SetFPS(int(n+0.5), d);
  } else {
    vi.SetFPS(int(n+0.5), args[6].AsInt(vi_default.fps_denominator));
  }

  vi.image_type = vi_default.image_type; // Copy any field sense from template

  vi.audio_samples_per_second = args[7].AsInt(vi_default.audio_samples_per_second);

  if (args[8].IsBool())
    vi.nchannels = args[8].AsBool() ? 2 : 1; // stereo=True
  else if (args[8].IsInt())
    vi.nchannels = args[8].AsInt();          // channels=2
  else
    vi.nchannels = vi_default.nchannels;

  if (args[9].IsBool())
    vi.sample_type = args[9].AsBool() ? SAMPLE_INT16 : SAMPLE_FLOAT; // sixteen_bit=True
  else if (args[9].IsString()) {
    const char* sample_type_string = args[9].AsString();
    if        (!strcasecmp(sample_type_string, "8bit" )) {  // sample_type="8Bit"
      vi.sample_type = SAMPLE_INT8;
    } else if (!strcasecmp(sample_type_string, "16bit")) {  // sample_type="16Bit"
      vi.sample_type = SAMPLE_INT16;
    } else if (!strcasecmp(sample_type_string, "24bit")) {  // sample_type="24Bit"
      vi.sample_type = SAMPLE_INT24;
    } else if (!strcasecmp(sample_type_string, "32bit")) {  // sample_type="32Bit"
      vi.sample_type = SAMPLE_INT32;
    } else if (!strcasecmp(sample_type_string, "float")) {  // sample_type="Float"
      vi.sample_type = SAMPLE_FLOAT;
    } else {
      env->ThrowError("BlankClip: sample_type must be \"8bit\", \"16bit\", \"24bit\", \"32bit\" or \"float\"");
    }
  } else
    vi.sample_type = vi_default.sample_type;

  vi.width++; // cheat HasVideo() call for Audio Only clips
  vi.num_audio_samples = vi.AudioSamplesFromFrames(vi.num_frames);
  vi.width--;

  int color = args[10].AsInt(0);
  int mode = COLOR_MODE_RGB;
  if (args[11].Defined()) {
    if (color != 0) // Not quite 100% test
      env->ThrowError("BlankClip: color and color_yuv are mutually exclusive");
    if (!vi.IsYUV())
      env->ThrowError("BlankClip: color_yuv only valid for YUV color spaces");
    color = args[11].AsInt();
    mode=COLOR_MODE_YUV;
    if ((unsigned)color > 0xffffff)
      env->ThrowError("BlankClip: color_yuv must be between 0 and %d($ffffff)", 0xffffff);
  }

  return new StaticImage(vi, CreateBlankFrame(vi, color, mode, env), parity);
}


/********************************************************************
********************************************************************/

// in text-overlay.cpp
extern void ApplyMessage(PVideoFrame* frame, const VideoInfo& vi,
  const char* message, int size, int textcolor, int halocolor, int bgcolor,
  IScriptEnvironment* env);

extern bool GetTextBoundingBox(const char* text, const char* fontname,
  int size, bool bold, bool italic, int align, int* width, int* height);

int find_number_of_newlines(const char* message, unsigned int & longestSubLineLength)
{
    int nOcccurences = 0;
    std::string strMessage = message;
    
    size_t found = 0;
    while (1)
    {
        found = strMessage.find_first_of("\n", found);
        if(std::string::npos == found)
            break;

        nOcccurences++;
        if(found > longestSubLineLength)
            longestSubLineLength = found;
        strMessage.erase(0, found + 1);
    }
    
    if(strMessage.length() > longestSubLineLength)
        longestSubLineLength = strMessage.length();
    
    return nOcccurences;
}

#define SCREEN_DIMENSION_ALIGNMENT  (16)
void fit_font_with_desired_video_dimensions(const char* message, int & fontSize, int & width, int & height, bool shrink)
{
    int nTextLen = strlen(message);
    
    unsigned int nLongestSubLineLength = 0;
    int nNewLineCharsFound = find_number_of_newlines(message, nLongestSubLineLength);
    
    if (-1 == width)
    {
        // we need to determine the window width so that entire text fits in without being wrapped around
        int nCharWidth = 0;
        GetApproximateCharacterWidth("Arial", fontSize, 0, 0, nCharWidth);
        int nSideMarginWidth = nCharWidth;

        if(0 == nNewLineCharsFound)
        {
            width = nTextLen*nCharWidth + 2*nSideMarginWidth;
        }
        else
        {
            int nFudgeFactor = 10*nCharWidth; // unexplained why is this needed, libpango breaks the text when there is newline in unusual manner
            width = nLongestSubLineLength*nCharWidth + 2*nSideMarginWidth + nFudgeFactor;
        }
        width = ((width + SCREEN_DIMENSION_ALIGNMENT)/SCREEN_DIMENSION_ALIGNMENT)*SCREEN_DIMENSION_ALIGNMENT;
        
        if (-1 == height || shrink)
        {
            if(0 == nNewLineCharsFound)
            {
                int nVerticalMargin = fontSize;
                height = fontSize + 2*nVerticalMargin;
            }
            else
            {                
                int nLines = nNewLineCharsFound + 1;
                
                int nMarginBetweenLines = 3;
                int nVerticalMargin     = fontSize;
                height                  = (2*nLines - 1)*fontSize +          // text lines
                                          (nLines - 1)*nMarginBetweenLines + // margins in between lines
                                          2*nVerticalMargin;
            }
            height = ((height + SCREEN_DIMENSION_ALIGNMENT)/SCREEN_DIMENSION_ALIGNMENT)*SCREEN_DIMENSION_ALIGNMENT;
        }
        // else there is no need to touch the height - leave it as it is
    }
    else
    {
        // basic question - can the message fit the existing screen ?
        int nMinRequiredHeight = 0;
        for (fontSize = 24; fontSize >= 9; fontSize--)
        {
            int nCharWidth = 0;
            GetApproximateCharacterWidth("Arial", fontSize, 0, 0, nCharWidth);
            int nSideMarginWidth = nCharWidth;
            
            int nCharsPerLine = (width - 2*nSideMarginWidth)/nCharWidth;
            
            int nLines = nTextLen/nCharsPerLine;
            if(nTextLen % nCharsPerLine)
                nLines++;
            
            int nMarginBetweenLines = 1;
            int nVerticalMargin     = fontSize;
            nMinRequiredHeight = (2*nLines - 1)*fontSize +          // text lines
                                 (nLines - 1)*nMarginBetweenLines + // margins in between lines
                                 2*nVerticalMargin;            
            if(-1 == height)
                break;
            else if(nMinRequiredHeight <= height)
            {
                if(shrink)
                {
                    height = nMinRequiredHeight;
                    height = ((height + SCREEN_DIMENSION_ALIGNMENT)/SCREEN_DIMENSION_ALIGNMENT)*SCREEN_DIMENSION_ALIGNMENT;
                }

                return; // with determined font size
            }
        }
        
        //
        // If we arrived here, it means that even with the smallest possible font we can't fit in the message into the window, 
        // or that video height was not specified (i.e. set to -1)
        //
        height = nMinRequiredHeight;
        height = ((height + SCREEN_DIMENSION_ALIGNMENT)/SCREEN_DIMENSION_ALIGNMENT)*SCREEN_DIMENSION_ALIGNMENT;
        return ;
    }
}

PClip Create_MessageClip(const char* message, int width, int height, int pixel_type, bool shrink,
                         int textcolor, int halocolor, int bgcolor, IScriptEnvironment* env) {
// builtinfunctions
#if 0 // original avisynth code
  for (size = 24*8; /*size>=9*8*/; size-=4) {
    int text_width, text_height;
    GetTextBoundingBox(message, "Arial", size, true, false, TA_TOP | TA_CENTER, &text_width, &text_height);
    text_width = ((text_width>>3)+8+3) & -4;
    text_height = ((text_height>>3)+8+1) & -2;
    if (size<=9*8 || ((width<=0 || text_width<=width) && (height<=0 || text_height<=height))) {
      if (width <= 0 || (shrink && width>text_width))
        width = text_width;
      if (height <= 0 || (shrink && height>text_height))
        height = text_height;
      break;
    }
  }
#else
  int fontHeight = 24; // to start with
  fit_font_with_desired_video_dimensions(message, fontHeight, width, height, shrink);
  
#endif
  VideoInfo vi;
  memset(&vi, 0, sizeof(vi));
  vi.width = width;
  vi.height = height;
  vi.pixel_type = pixel_type;
  vi.fps_numerator = 24;
  vi.fps_denominator = 1;
  vi.num_frames = 240;

  PVideoFrame frame = CreateBlankFrame(vi, bgcolor, COLOR_MODE_RGB, env);
  ApplyMessage(&frame, vi, message, fontHeight, textcolor, halocolor, bgcolor, env);
  return new StaticImage(vi, frame, false);
};


AVSValue __cdecl Create_MessageClip(AVSValue args, void*, IScriptEnvironment* env) {
  return Create_MessageClip(args[0].AsString(), args[1].AsInt(-1),
      args[2].AsInt(-1), VideoInfo::CS_BGR24, args[3].AsBool(false),
      args[4].AsInt(0xFFFFFF), args[5].AsInt(0), args[6].AsInt(0), env);
}


/********************************************************************
********************************************************************/
ColorBars::ColorBars(int w, int h, const char* pixel_type, IScriptEnvironment* env) {
    memset(&vi, 0, sizeof(VideoInfo));
    vi.width = w;
    vi.height = h;
    vi.fps_numerator = 30000;
    vi.fps_denominator = 1001;
    vi.num_frames = 107892;   // 1 hour
    if (strcasecmp(pixel_type, "RGB32") == 0) {
        vi.pixel_type = VideoInfo::CS_BGR32;
    }
	else if (strcasecmp(pixel_type, "YUY2") == 0) { // YUY2
        vi.pixel_type = VideoInfo::CS_YUY2;
		if (w & 1)
          env->ThrowError("ColorBars: YUY2 width must be even!");
    }
	else if (strcasecmp(pixel_type, "YV12") == 0) { // YV12
		vi.pixel_type = VideoInfo::CS_YV12;
		if ((w & 1) || (h & 1))
		env->ThrowError("ColorBars: YV12 both height and width must be even!");
	}
	else {
      env->ThrowError("ColorBars: pixel_type must be \"RGB32\", \"YUY2\" or \"YV12\"");
    }
    vi.sample_type = SAMPLE_FLOAT;
    vi.nchannels = 2;
    vi.audio_samples_per_second = 48000;
    vi.num_audio_samples=vi.AudioSamplesFromFrames(vi.num_frames);

    frame = env->NewVideoFrame(vi);
    unsigned* p = (unsigned*)frame->GetWritePtr();
    const int pitch = frame->GetPitch()/4;

    int y = 0;
    
	// Rec. ITU-R BT.801-1
	if (vi.IsRGB32()) {
		// note we go bottom->top
		static const int bottom_quarter[] =
// RGB[16..235]     -I     white        +Q     Black     -4ire     Black     +4ire     Black
			{ 0x003a62, 0xebebeb, 0x4b0f7e, 0x101010,  0x070707, 0x101010, 0x191919,  0x101010 }; // Qlum=Ilum=13.4%
		for (; y < h/4; ++y) {
			int x = 0;
			for (int i=0; i<4; ++i) {
				for (; x < (w*(i+1)*5+14)/28; ++x)
					p[x] = bottom_quarter[i];
			}
			for (int j=4; j<7; ++j) {
				for (; x < (w*(j+12)+10)/21; ++x)
					p[x] = bottom_quarter[j];
			}
			for (; x < w; ++x)
				p[x] = bottom_quarter[7];
			p += pitch;
		}
		
		static const int two_thirds_to_three_quarters[] =
// RGB[16..235]   Blue     Black  Magenta      Black      Cyan     Black    LtGrey
			{ 0x1010b4, 0x101010, 0xb410b4, 0x101010, 0x10b4b4, 0x101010, 0xb4b4b4 };
		for (; y < h/3; ++y) {
			int x = 0;
			for (int i=0; i<7; ++i) {
				for (; x < (w*(i+1)+3)/7; ++x)
					p[x] = two_thirds_to_three_quarters[i];
			}
			p += pitch;
		}
		
		static const int top_two_thirds[] =
// RGB[16..235] LtGrey    Yellow      Cyan     Green   Magenta       Red      Blue
			{ 0xb4b4b4, 0xb4b410, 0x10b4b4, 0x10b410, 0xb410b4, 0xb41010, 0x1010b4 };
		for (; y < h; ++y) {
			int x = 0;
			for (int i=0; i<7; ++i) {
				for (; x < (w*(i+1)+3)/7; ++x)
					p[x] = top_two_thirds[i];
			}
			p += pitch;
		}
	}
	else if (vi.IsYUY2()) {
		static const int top_two_thirds[] =
//                LtGrey      Yellow        Cyan       Green     Magenta         Red        Blue
			{ 0x80b480b4, 0x8ea22ca2, 0x2c839c83, 0x3a704870, 0xc654b854, 0xd4416441, 0x7223d423 }; //VYUY
		w >>= 1;
		for (; y*3 < h*2; ++y) {
			int x = 0;
			for (int i=0; i<7; i++) {
				for (; x < (w*(i+1)+3)/7; ++x)
					p[x] = top_two_thirds[i];
			}
			p += pitch;
		}

		static const int two_thirds_to_three_quarters[] =
//                 Blue        Black     Magenta       Black        Cyan       Black      LtGrey
			{ 0x7223d423, 0x80108010, 0xc654b854, 0x80108010, 0x2c839c83, 0x80108010, 0x80b480b4 }; //VYUY
		for (; y*4 < h*3; ++y) {
			int x = 0;
			for (int i=0; i<7; i++) {
				for (; x < (w*(i+1)+3)/7; ++x)
					p[x] = two_thirds_to_three_quarters[i];
			}
			p += pitch;
		}

		static const int bottom_quarter[] =
//                    -I       white          +Q       Black       -4ire       Black       +4ire       Black
			{ 0x5f109e10, 0x80eb80eb, 0x9510ae10, 0x80108010, 0x80078007, 0x80108010, 0x80198019, 0x80108010 }; //VYUY
		for (; y < h; ++y) {
			int x = 0;
			for (int i=0; i<4; ++i) {
				for (; x < (w*(i+1)*5+14)/28; ++x)
					p[x] = bottom_quarter[i];
			}
			for (int j=4; j<7; ++j) {
				for (; x < (w*(j+12)+10)/21; ++x)
					p[x] = bottom_quarter[j];
			}
			for (; x < w; ++x)
				p[x] = bottom_quarter[7];
			p += pitch;
		}
	}
	else if (vi.IsYV12()) {
		unsigned short* pY = (unsigned short*)frame->GetWritePtr(PLANAR_Y);
		BYTE* pU = (BYTE*)frame->GetWritePtr(PLANAR_U);
		BYTE* pV = (BYTE*)frame->GetWritePtr(PLANAR_V);
		const int pitchY  = frame->GetPitch(PLANAR_Y)>>1;
		const int pitchUV = frame->GetPitch(PLANAR_U);
//                                                        LtGrey  Yellow    Cyan   Green Magenta     Red    Blue
		static const unsigned short top_two_thirdsY[] = { 0xb4b4, 0xa2a2, 0x8383, 0x7070, 0x5454, 0x4141, 0x2323 };
		static const BYTE top_two_thirdsU[] =           {   0x80,   0x2c,   0x9c,   0x48,   0xb8,   0x64,   0xd4 };
		static const BYTE top_two_thirdsV[] =           {   0x80,   0x8e,   0x2c,   0x3a,   0xc6,   0xd4,   0x72 };
		w >>= 1;
		h >>= 1;
		for (; y*3 < h*2; ++y) {
			int x = 0;
			for (int i=0; i<7; i++) {
				for (; x < (w*(i+1)+3)/7; ++x) {
					pY[x] = pY[x+pitchY] = top_two_thirdsY[i];
					pU[x] = top_two_thirdsU[i];
					pV[x] = top_two_thirdsV[i];
				}
			}
			pY += pitchY*2; pU += pitchUV; pV += pitchUV;
		} //                                                              Blue   Black Magenta   Black    Cyan   Black  LtGrey
		static const unsigned short two_thirds_to_three_quartersY[] = { 0x2323, 0x1010, 0x5454, 0x1010, 0x8383, 0x1010, 0xb4b4 };
		static const BYTE two_thirds_to_three_quartersU[] =           {   0xd4,   0x80,   0xb8,   0x80,   0x9c,   0x80,   0x80 };
		static const BYTE two_thirds_to_three_quartersV[] =           {   0x72,   0x80,   0xc6,   0x80,   0x2c,   0x80,   0x80 };
		for (; y*4 < h*3; ++y) {
			int x = 0;
			for (int i=0; i<7; i++) {
				for (; x < (w*(i+1)+3)/7; ++x) {
					pY[x] = pY[x+pitchY] = two_thirds_to_three_quartersY[i];
					pU[x] = two_thirds_to_three_quartersU[i];
					pV[x] = two_thirds_to_three_quartersV[i];
				}
			}
			pY += pitchY*2; pU += pitchUV; pV += pitchUV;
		} //                                                  -I   white      +Q   Black   -4ire   Black   +4ire   Black
		static const unsigned short bottom_quarterY[] = { 0x1010, 0xebeb, 0x1010, 0x1010, 0x0707, 0x1010, 0x1919, 0x1010 };
		static const BYTE bottom_quarterU[] =           {   0x9e,   0x80,   0xae,   0x80,   0x80,   0x80,   0x80,   0x80 };
		static const BYTE bottom_quarterV[] =           {   0x5f,   0x80,   0x95,   0x80,   0x80,   0x80,   0x80,   0x80 };
		for (; y < h; ++y) {
			int x = 0;
			for (int i=0; i<4; ++i) {
				for (; x < (w*(i+1)*5+14)/28; ++x) {
					pY[x] = pY[x+pitchY] = bottom_quarterY[i];
					pU[x] = bottom_quarterU[i];
					pV[x] = bottom_quarterV[i];
				}
			}
			for (int j=4; j<7; ++j) {
				for (; x < (w*(j+12)+10)/21; ++x) {
					pY[x] = pY[x+pitchY] = bottom_quarterY[j];
					pU[x] = bottom_quarterU[j];
					pV[x] = bottom_quarterV[j];
				}
			}
			for (; x < w; ++x) {
				pY[x] = pY[x+pitchY] = bottom_quarterY[7];
				pU[x] = bottom_quarterU[7];
				pV[x] = bottom_quarterV[7];
			}
			pY += pitchY*2; pU += pitchUV; pV += pitchUV;
		}
	}

	// Generate Audio buffer
	{
	  unsigned x=vi.audio_samples_per_second, y=Hz;
	  while (y) {     // find gcd
		unsigned t = x%y; x = y; y = t;
	  }
	  nsamples = vi.audio_samples_per_second/x; // 1200
	  const unsigned ncycles  = Hz/x; // 11

	  audio = new SFLOAT[nsamples];

	  if (!audio)
		env->ThrowError("ColorBars: insufficient memory");

	  const double add_per_sample=ncycles/(double)nsamples;
	  double second_offset=0.0;
	  for (unsigned int i=0;i<nsamples;i++) {
		  audio[i] = sin(3.1415926535897932384626433832795*2.0*second_offset);
		  second_offset+=add_per_sample;
	  }
	}
  }

ColorBars::~ColorBars() {
    if (audio) delete audio;
};

void __stdcall ColorBars::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) {
#if 1
    const int d_mod = vi.audio_samples_per_second*2;
    float* samples = (float*)buf;

	unsigned int j = start % nsamples;
    for (unsigned int i=0;i<count;i++) {
	  samples[i*2]=audio[j];
	  if (((start+i)%d_mod)>vi.audio_samples_per_second) {
		samples[i*2+1]=audio[j];
	  } else {
		samples[i*2+1]=0;
	  }
	  if (++j >= nsamples) j = 0;
	}
#else
    __int64 Hz=440;
    // Calculate what start equates in cycles.
    // This is the number of cycles (rounded down) that has already been taken.
    __int64 startcycle = (start*Hz) /  vi.audio_samples_per_second;  
    
    // Move offset down - this is to avoid float rounding errors
    int start_offset = (int)(start - ((startcycle * vi.audio_samples_per_second) / Hz));

    double add_per_sample=Hz/(double)vi.audio_samples_per_second;
    double second_offset=((double)start_offset*add_per_sample);
    int d_mod=vi.audio_samples_per_second*2;
    float* samples = (float*)buf;

    for (int i=0;i<count;i++) {
        samples[i*2]=sinf(3.1415926535897932384626433832795f*2.0f*(float)second_offset);
        if (((start+i)%d_mod)>vi.audio_samples_per_second) {
          samples[i*2+1]=samples[i*2];
        } else {
          samples[i*2+1]=0;
        }
        second_offset+=add_per_sample;
    }
#endif
}

AVSValue __cdecl ColorBars::Create(AVSValue args, void*, IScriptEnvironment* env) {
    return new ColorBars(args[0].AsInt(640), args[1].AsInt(480), args[2].AsString("RGB32"), env);
};


/********************************************************************
********************************************************************/



/********************************************************************
********************************************************************/

#if 0
class QuickTimeSource : public IClip {
public:
  QuickTimeSource() {
//    extern void foo();
//    foo();
  }
  PVideoFrame GetFrame(int n, IScriptEnvironment* env) {}
  void GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) {}
  const VideoInfo& GetVideoInfo() {}
  bool GetParity(int n) { return false; }

  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env) {
    return new QuickTimeSource;
  }
};
#endif

#if 0
/********************************************************************
********************************************************************/

AVSValue __cdecl Create_SegmentedSource(AVSValue args, void* use_directshow, IScriptEnvironment* env) {
  bool bAudio = !use_directshow && args[1].AsBool(true);
  const char* pixel_type;
  const char* fourCC;
  const int inv_args_count = args.ArraySize();
  AVSValue inv_args[9];
  if (!use_directshow) {
    pixel_type = args[2].AsString("");
    fourCC = args[3].AsString("");
  }
  else {
    for (int i=1; i<inv_args_count ;i++)
      inv_args[i] = args[i];
  }
  args = args[0];
  PClip result = 0;
  const char* error_msg=0;
  for (int i = 0; i < args.ArraySize(); ++i) {
    char basename[260];
    strcpy(basename, args[i].AsString());
    char* extension = strrchr(basename, '.');
    if (extension)
      *extension++ = 0;
    else
      extension = (char*)"";
    for (int j = 0; j < 100; ++j) {
      char filename[260];
      sprintf(filename, "%s.%02d.%s", basename, j, extension);
      if (GetFileAttributes(filename) != (DWORD)-1) {   // check if file exists
          PClip clip;
        try {
          if (use_directshow) {
            inv_args[0] = filename;
            clip = env->Invoke("DirectShowSource",AVSValue(inv_args, inv_args_count)).AsClip();
          } else {
            clip =  (IClip*)(new AVISource(filename, bAudio, pixel_type, fourCC, 0, env));
          }
          result = !result ? clip : new_Splice(result, clip, false, env);
        } catch (AvisynthError e) {
          error_msg=e.msg;
        }
      }
    }
  }
  if (!result) {
    if (!error_msg) {
      env->ThrowError("Segmented%sSource: no files found!", use_directshow ? "DirectShow" : "AVI");
    } else {
      env->ThrowError("Segmented%sSource: decompressor returned error:\n%s!", use_directshow ? "DirectShow" : "AVI",error_msg);
    }
  }
  return result;
}
#endif

Tone::Tone(float _length, double _freq, int _samplerate, int _ch, const char* _type, float _level, IScriptEnvironment* env)
	: freq(_freq), samplerate(_samplerate), ch(_ch), add_per_sample(_freq/_samplerate), level(_level) 
{
    memset(&vi, 0, sizeof(VideoInfo));
    vi.sample_type = SAMPLE_FLOAT;
    vi.nchannels = _ch;
    vi.audio_samples_per_second = _samplerate;
    vi.num_audio_samples=(__int64)(_length*(float)vi.audio_samples_per_second);

    if (!strcasecmp(_type, "Sine"))
      s = new SineGenerator();
    else if (!strcasecmp(_type, "Noise"))
      s = new NoiseGenerator();
    else if (!strcasecmp(_type, "Square"))
      s = new SquareGenerator();
    else if (!strcasecmp(_type, "Triangle"))
      s = new TriangleGenerator();
    else if (!strcasecmp(_type, "Sawtooth"))
      s = new SawtoothGenerator();
    else if (!strcasecmp(_type, "Silence"))
      s = new SampleGenerator();
    else
      env->ThrowError("Tone: Type was not recognized!");
}

void __stdcall Tone::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) {

    // Where in the cycle are we in?
    const double cycle = (freq * start) / samplerate;
    double period_place = cycle - floor(cycle);

    SFLOAT* samples = (SFLOAT*)buf;

    for (int i=0;i<count;i++) {
      SFLOAT v = s->getValueAt(period_place) * level;
      for (int o=0;o<ch;o++) {
        samples[o+i*ch] = v;
      }
      period_place += add_per_sample;
      if (period_place >= 1.0)
        period_place -= floor(period_place);
    }
  }

 AVSValue __cdecl Tone::Create(AVSValue args, void*, IScriptEnvironment* env) {
	try {	// HIDE DAMN SEH COMPILER BUG!!!
	    return new Tone(args[0].AsFloat(10.0), args[1].AsFloat(440), args[2].AsInt(48000),
		                args[3].AsInt(2), args[4].AsString("Sine"), args[5].AsFloat(1.0), env);
	}
	catch (...) { throw; }
}

AVSValue __cdecl Create_Version(AVSValue args, void*, IScriptEnvironment* env) {
  return Create_MessageClip(_AVS_COPYRIGHT, -1, -1, VideoInfo::CS_BGR24, false, 0xECF2BF, 0, 0x404040, env);
}


AVSFunction Source_filters[] = {
#if 0    
  { "AVISource", "s+[audio]b[pixel_type]s[fourCC]s", AVISource::Create, (void*) AVISource::MODE_NORMAL },
  { "AVIFileSource", "s+[audio]b[pixel_type]s[fourCC]s", AVISource::Create, (void*) AVISource::MODE_AVIFILE },
  { "WAVSource", "s+", AVISource::Create, (void*) AVISource::MODE_WAV },
  { "OpenDMLSource", "s+[audio]b[pixel_type]s[fourCC]s", AVISource::Create, (void*) AVISource::MODE_OPENDML },
  { "SegmentedAVISource", "s+[audio]b[pixel_type]s[fourCC]s", Create_SegmentedSource, (void*)0 },
  { "SegmentedDirectShowSource",
// args               0      1      2       3       4            5          6         7            8
                     "s+[fps]f[seek]b[audio]b[video]b[convertfps]b[seekzero]b[timeout]i[pixel_type]s",
                     Create_SegmentedSource, (void*)1 },
// args             0         1       2        3            4     5                 6            7        8             9       10          11     12
#endif
  { "BlankClip", "[]c*[length]i[width]i[height]i[pixel_type]s[fps]f[fps_denominator]i[audio_rate]i[stereo]b[sixteen_bit]b[color]i[color_yuv]i[clip]c", Create_BlankClip },
  { "BlankClip", "[]c*[length]i[width]i[height]i[pixel_type]s[fps]f[fps_denominator]i[audio_rate]i[channels]i[sample_type]s[color]i[color_yuv]i[clip]c", Create_BlankClip },
  { "Blackness", "[]c*[length]i[width]i[height]i[pixel_type]s[fps]f[fps_denominator]i[audio_rate]i[stereo]b[sixteen_bit]b[color]i[color_yuv]i[clip]c", Create_BlankClip },
  { "Blackness", "[]c*[length]i[width]i[height]i[pixel_type]s[fps]f[fps_denominator]i[audio_rate]i[channels]i[sample_type]s[color]i[color_yuv]i[clip]c", Create_BlankClip },
  { "MessageClip", "s[width]i[height]i[shrink]b[text_color]i[halo_color]i[bg_color]i", Create_MessageClip },
  { "ColorBars", "[width]i[height]i[pixel_type]s", ColorBars::Create },
  { "Tone", "[length]f[frequency]f[samplerate]i[channels]i[type]s[level]f", Tone::Create },
  { "Version", "", Create_Version },
  { 0,0,0 }
};

}; // namespace avxsynth
