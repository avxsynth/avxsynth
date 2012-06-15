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

#include <text-overlay.h>

using namespace std;

#include "windowsPorts.h"
#include "AvxString.h"
#include "TextLayout.h"
#include "TextConfig.h"
#include "AvxTextRender.h"
#include "avxlog.h"

#ifdef MODULE_NAME
#undef MODULE_NAME
#endif

#define MODULE_NAME core::text-overlay

#include "avxlog.h"

namespace avxsynth {

#define COLOR_COMPONENT_B(x)	((x >> 16) & 0xFF)
#define COLOR_COMPONENT_G(x)	((x >>  8) & 0xFF)
#define COLOR_COMPONENT_R(x)	(x & 0xFF)

/********************************************************************
***** Declare index of new filters for Avisynth's filter engine *****
********************************************************************/

AVSFunction Text_filters[] = {
  { "ShowFrameNumber",
	"c[scroll]b[offset]i[x]i[y]i[font]s[size]f[text_color]i[halo_color]i[font_width]f[font_angle]f",
	ShowFrameNumber::Create },

  { "ShowSMPTE",
	"c[fps]f[offset]s[offset_f]i[x]i[y]i[font]s[size]f[text_color]i[halo_color]i[font_width]f[font_angle]f",
	ShowSMPTE::CreateSMTPE },

  { "ShowTime",
	"c[offset_f]i[x]i[y]i[font]s[size]f[text_color]i[halo_color]i[font_width]f[font_angle]f",
	ShowSMPTE::CreateTime },

  { "Info", "c", FilterInfo::Create },  // clip

  { "Subtitle",
	"cs[x]i[y]i[first_frame]i[last_frame]i[font]s[size]f[text_color]i[halo_color]i"
	"[align]i[spc]i[lsp]i[font_width]f[font_angle]f[interlaced]b", 
    Subtitle::Create },       // see docs!

  { "Compare",
	"cc[channels]s[logfile]s[show_graph]b",
	Compare::Create },

  { 0 }
};


void convertColorFormatToRGB24(PClip & clip, VideoInfo const& vi, IScriptEnvironment* env)
{
  if(!vi.IsRGB24())
    clip = env->Invoke("ConvertToRGB24", AVSValue(clip)).AsClip();
}

void convertColorFormatBackToOriginal(PClip & input, VideoInfo const& vi, IScriptEnvironment* env)
{
  if(vi.IsYUY2())
    input = env->Invoke("ConvertToYUY2", AVSValue(input)).AsClip();
  else if(vi.IsYV12())
    input = env->Invoke("ConvertToYV12", AVSValue(input)).AsClip();
  else if(vi.IsRGB32())
    input = env->Invoke("ConvertToRGB32", AVSValue(input)).AsClip();
}

/*************************************
 *******   Show Frame Number    ******
 ************************************/

ShowFrameNumber::ShowFrameNumber(PClip _child, bool _scroll, int _offset, int _x, int _y, const char _fontname[],
					 int _size, int _textcolor, int _halocolor, int font_width, int font_angle, IScriptEnvironment* env)
// GenericVideoFilter(_child), antialiaser(vi.width, vi.height, "Arial", 192),
 : GenericVideoFilter(_child), 
   fontname(_fontname), textcolor(_textcolor), halocolor(_halocolor),			   
   scroll(_scroll), offset(_offset), size(_size), x(_x), y(_y)
{
  if ((x==-1) ^ (y==-1))
	env->ThrowError("ShowFrameNumber: both x and y position must be specified");
}

#define FRAME_NUMBER_CHARACTERS (5)
#define FRAME_NUMBER_PRINT_FORMAT   "%05d"

PVideoFrame ShowFrameNumber::GetFrame(int n, IScriptEnvironment* env) {
  PVideoFrame frame = child->GetFrame(n, env);
  n+=offset;
  if (n < 0) return frame;
  
  env->MakeWritable(&frame);

  AvxTextRender::FrameBuffer trd(frame->GetWritePtr(), vi.width, vi.height, frame->GetPitch());
  TextConfig txtConfig(fontname, size/8, 0.75, textcolor, halocolor);

  int nCharWidth;
  GetApproximateCharacterWidth(fontname, size/8, 0, 0, nCharWidth);
  
  int nLeftCoordinate = this->x;
  if(-1 == nLeftCoordinate)
  {
    // progressive frames and/or bottom field will be displayed on the right side, top field on the left
    bool bDisplayOnRightSide= vi.IsFieldBased() ? child->GetParity(n) : true;
    int nSideMargin         = nCharWidth;
    nLeftCoordinate         = bDisplayOnRightSide ? vi.width - nSideMargin - FRAME_NUMBER_CHARACTERS*nCharWidth : this->x + nSideMargin;
  }

  int nTopCoordinate = this->y; 
  if(-1 == nTopCoordinate)
  {
    nTopCoordinate   = vi.height - 2*size/8;
  }
  
  TextLayout txtLayout(TextLayout::Rect(nLeftCoordinate, nTopCoordinate, vi.width, vi.height), TextLayout::VCenter, TextLayout::HCenter);

  char text[16];
  sprintf(text, FRAME_NUMBER_PRINT_FORMAT, n);
  
  try
  {
    if(-1 == this->x && -1 == this->y && scroll)
        AvxTextRender::RenderText(text, trd, txtConfig, txtLayout, AvxTextRender::RenderOptions_Scroll_SFN, n - offset);
    else
        AvxTextRender::RenderText(text, trd, txtConfig, txtLayout);
  }
  catch(AvxException &e)
  {
    env->ThrowError(Utf8String(e.GetMsg()).c_str());
  }
  return frame;
}


AVSValue __cdecl ShowFrameNumber::Create(AVSValue args, void*, IScriptEnvironment* env) 
{
  AVXLOG_INFO("ShowFrameNumber::%s", __FUNCTION__);

  PClip clip = args[0].AsClip();
  
  VideoInfo vi = clip->GetVideoInfo();
  convertColorFormatToRGB24(clip, vi, env);
  
  bool scroll = args[1].AsBool(false);
  const int offset = args[2].AsInt(0);
  const int x = args[3].AsInt(-1);
  const int y = args[4].AsInt(-1);
  const char* font = args[5].AsString("Arial");
  const int size = int(args[6].AsFloat(24)*8+0.5);
  const int text_color = args[7].AsInt(0xFFFF00);
  const int halo_color = args[8].AsInt(0);
  const int font_width = int(args[9].AsFloat(0)*8+0.5);
  const int font_angle = int(args[10].AsFloat(0)*10+0.5);
  PClip texted = new ShowFrameNumber(clip, scroll, offset, x, y, font, size, text_color, halo_color, font_width, font_angle, env);
  
  convertColorFormatBackToOriginal(texted, vi, env);
  return texted;
}








/***********************************
 *******   Show SMPTE code    ******
 **********************************/

ShowSMPTE::ShowSMPTE(PClip _child, double _rate, const char* offset, int _offset_f, int _x, int _y, const char _fontname[],
					 int _size, int _textcolor, int _halocolor, int font_width, int font_angle, IScriptEnvironment* env)
  : GenericVideoFilter(_child), 
  fontname(_fontname), textcolor(_textcolor), halocolor(_halocolor), size(_size),
	x(_x), y(_y)
{
  int off_f, off_sec, off_min, off_hour;

  if (_rate == 24) {
    rate = 24;
    dropframe = false;
  } 
  else if (_rate > 23.975 && _rate < 23.977) { // Pulldown drop frame rate
    rate = 24;
    dropframe = true;
  } 
  else if (_rate == 25) {
    rate = 25;
    dropframe = false;
  } 
  else if (_rate == 30) {
    rate = 30;
    dropframe = false;
  } 
  else if (_rate > 29.969 && _rate < 29.971) {
    rate = 30;
    dropframe = true;
  } 
  else if (_rate == 0) {
    rate = 0; // Display hh:mm:ss.mmm
    dropframe = false;
  }
  else {
    env->ThrowError("ShowSMPTE: rate argument must be 23.976, 24, 25, 29.97 or 30");
  }

  if (offset) {
	if (strlen(offset)!=11 || offset[2] != ':' || offset[5] != ':' || offset[8] != ':')
	  env->ThrowError("ShowSMPTE:  offset should be of the form \"00:00:00:00\" ");
	if (!isdigit(offset[0]) || !isdigit(offset[1]) || !isdigit(offset[3]) || !isdigit(offset[4])
	 || !isdigit(offset[6]) || !isdigit(offset[7]) || !isdigit(offset[9]) || !isdigit(offset[10]))
	  env->ThrowError("ShowSMPTE:  offset should be of the form \"00:00:00:00\" ");

	off_hour = atoi(offset);

	off_min = atoi(offset+3);
	if (off_min > 59)
	  env->ThrowError("ShowSMPTE:  make sure that the number of minutes in the offset is in the range 0..59");

	off_sec = atoi(offset+6);
	if (off_sec > 59)
	  env->ThrowError("ShowSMPTE:  make sure that the number of seconds in the offset is in the range 0..59");

	off_f = atoi(offset+9);
	if (off_f >= rate)
	  env->ThrowError("ShowSMPTE:  make sure that the number of frames in the offset is in the range 0..%d", rate-1);

	offset_f = off_f + rate*(off_sec + 60*off_min + 3600*off_hour);
	if (dropframe) {
	  if (rate == 30) {
		int c = 0;
		c = off_min + 60*off_hour;  // number of drop events
		c -= c/10; // less non-drop events on 10 minutes
		c *=2; // drop 2 frames per drop event
		offset_f -= c;
	  }
	  else if (rate == 24) {
//  Need to cogitate with the guys about this
//  gotta drop 86.3 counts per hour. So until
//  a proper formula is found, just wing it!
		offset_f -= 2 * ((offset_f+1001)/2002);
	  }
	}
  }
  else {
	offset_f = _offset_f;
  }
}


PVideoFrame __stdcall ShowSMPTE::GetFrame(int n, IScriptEnvironment* env) 
{
  PVideoFrame frame = child->GetFrame(n, env);
  n+=offset_f;
  if (n < 0) return frame;

  env->MakeWritable(&frame);

  if (dropframe) {
    if (rate == 30) {
	// at 10:00, 20:00, 30:00, etc. nothing should happen if offset=0
	  int high = n / 17982;
	  int low = n % 17982;
	  if (low>=2)
		low += 2 * ((low-2) / 1798);
	  n = high * 18000 + low;
	}
	else if (rate == 24) {
//  Needs some cogitating
	  n += 2 * ((n+1001)/2002);
	}
  }

  char text[17];

  if (rate > 0) {
    int frames = n % rate;
    int sec = n/rate;
    int min = sec/60;
    int hour = sec/3600;

    sprintf(text, "%02d:%02d:%02d:%02d", hour, min%60, sec%60, frames);
  }
  else {
    int ms = (int)(((__int64)n * vi.fps_denominator * 1000 / vi.fps_numerator)%1000);
    int sec = (int)((__int64)n * vi.fps_denominator / vi.fps_numerator);
    int min = sec/60;
    int hour = sec/3600;

    sprintf(text, "%02d:%02d:%02d.%03d", hour, min%60, sec%60, ms);
  }

  int nLeftCoordinate = this->x;
  if(-1 == nLeftCoordinate)
  {
      nLeftCoordinate = vi.width/2;
      int nCharWidth;
      GetApproximateCharacterWidth(fontname, size/8, 0, 0, nCharWidth);
      nLeftCoordinate -= strlen(text)*nCharWidth/2;
  }
  int nTopCoordinate = this->y;
  if(-1 == nTopCoordinate)
  {
      nTopCoordinate = vi.height - 3*(size/8)/2;
  }
  AvxTextRender::FrameBuffer trd(frame->GetWritePtr(), vi.width, vi.height, frame->GetPitch());
  TextConfig txtConfig(fontname, size/8, 0.75, textcolor, halocolor);
  TextLayout txtLayout(TextLayout::Rect(nLeftCoordinate, nTopCoordinate, vi.width, vi.height), TextLayout::VCenter, TextLayout::HCenter);
  
  try
  {
    AvxTextRender::RenderText(text, trd, txtConfig, txtLayout);
  }
  catch(AvxException &e)
  {
    env->ThrowError(Utf8String(e.GetMsg()).c_str());
  }
  return frame;
}

AVSValue __cdecl ShowSMPTE::CreateSMTPE(AVSValue args, void*, IScriptEnvironment* env)
{
  AVXLOG_INFO("ShowSMPTE::%s", __FUNCTION__);
    
  PClip clip = args[0].AsClip();
  
  VideoInfo vi = clip->GetVideoInfo();
  convertColorFormatToRGB24(clip, vi, env);
  
  double def_rate = (double)args[0].AsClip()->GetVideoInfo().fps_numerator / (double)args[0].AsClip()->GetVideoInfo().fps_denominator;
  double dfrate = args[1].AsFloat(def_rate);
  const char* offset = args[2].AsString(0);
  const int offset_f = args[3].AsInt(0);
  const int xreal = args[0].AsClip()->GetVideoInfo().width;
  const int x = args[4].AsInt(-1);
  const int yreal = args[0].AsClip()->GetVideoInfo().height;
  const int y = args[5].AsInt(-1);
  const char* font = args[6].AsString("Arial");
  const int size = int(args[7].AsFloat(24)*8+0.5);
  const int text_color = args[8].AsInt(0xFFFF00);
  const int halo_color = args[9].AsInt(0);
  const int font_width = int(args[10].AsFloat(0)*8+0.5);
  const int font_angle = int(args[11].AsFloat(0)*10+0.5);
  PClip texted = new ShowSMPTE(clip, dfrate, offset, offset_f, x, y, font, size, text_color, halo_color, font_width, font_angle, env);
   
  convertColorFormatBackToOriginal(texted, vi, env);
  return texted;
}

AVSValue __cdecl ShowSMPTE::CreateTime(AVSValue args, void*, IScriptEnvironment* env)
{
  PClip clip = args[0].AsClip();
  
  VideoInfo vi = clip->GetVideoInfo();
  convertColorFormatToRGB24(clip, vi, env);
  
  const int offset_f = args[1].AsInt(0);
  const int xreal = args[0].AsClip()->GetVideoInfo().width;
  const int x = args[2].AsInt(-1);
  const int yreal = args[0].AsClip()->GetVideoInfo().height;
  const int y = args[3].AsInt(-1);
  const char* font = args[4].AsString("Arial");
  const int size = int(args[5].AsFloat(24)*8+0.5);
  const int text_color = args[6].AsInt(0xFFFF00);
  const int halo_color = args[7].AsInt(0);
  const int font_width = int(args[8].AsFloat(0)*8+0.5);
  const int font_angle = int(args[9].AsFloat(0)*10+0.5);
  PClip texted = new ShowSMPTE(clip, 0.0, NULL, offset_f, x, y, font, size, text_color, halo_color, font_width, font_angle, env);
     
  convertColorFormatBackToOriginal(texted, vi, env);
  return texted;
}






/***********************************
 *******   Subtitle Filter    ******
 **********************************/


Subtitle::Subtitle( PClip _child, const char _text[], int _x, int _y, int _firstframe, 
                    int _lastframe, const char _fontname[], int _size, int _textcolor, 
                    int _halocolor, int _align, int _spc, bool _multiline, int _lsp,
					int _font_width, int _font_angle, bool _interlaced )
 : GenericVideoFilter(_child), 
   x(_x), y(_y), 
   firstframe(_firstframe), lastframe(_lastframe), 
   size(_size),
   lsp(_lsp), 
   font_width(_font_width), font_angle(_font_angle), 
   multiline(_multiline), 
   interlaced(_interlaced), 
   textcolor(vi.IsYUV() ? RGB2YUV(_textcolor) : _textcolor),
   halocolor(vi.IsYUV() ? RGB2YUV(_halocolor) : _halocolor), 
   align(_align), 
   spc(_spc),
   fontname(_fontname),
   text(_text)
{
}



Subtitle::~Subtitle(void) 
{
}



PVideoFrame Subtitle::GetFrame(int n, IScriptEnvironment* env) 
{
  PVideoFrame frame = child->GetFrame(n, env);

  if (n >= firstframe && n <= lastframe) {
    env->MakeWritable(&frame);

    AvxTextRender::FrameBuffer trd(frame->GetWritePtr(), vi.width, vi.height, frame->GetPitch());
    TextConfig txtConfig(fontname, size/8, 0.75, textcolor, halocolor);
    TextLayout txtLayout(TextLayout::Rect(x,y,vi.width, vi.height), TextLayout::VCenter, TextLayout::Left);    

    try
    {
        AvxTextRender::RenderText(text, trd, txtConfig, txtLayout);
    }
    catch(AvxException &e)
    {
        env->ThrowError(Utf8String(e.GetMsg()).c_str());
    }
  }
  
  return frame;
}

AVSValue __cdecl Subtitle::Create(AVSValue args, void*, IScriptEnvironment* env) 
{
    AVXLOG_INFO("Subtitle::%s", __FUNCTION__);
    
    PClip clip = args[0].AsClip();
    
    VideoInfo vi = clip->GetVideoInfo();
    convertColorFormatToRGB24(clip, vi, env);
    
    const char* text = args[1].AsString();
    const int first_frame = args[4].AsInt(0);
    const int last_frame = args[5].AsInt(clip->GetVideoInfo().num_frames-1);
    const char* font = args[6].AsString("Arial");
    const int size = int(args[7].AsFloat(18)*8+0.5);
    const int text_color = args[8].AsInt(0xFFFF00);
    const int halo_color = args[9].AsInt(0);
    const int align = args[10].AsInt(args[2].AsInt(8)==-1?2:7);
    const int spc = args[11].AsInt(0);
    const bool multiline = args[12].Defined();
    const int lsp = args[12].AsInt(0);
	const int font_width = int(args[13].AsFloat(0)*8+0.5);
	const int font_angle = int(args[14].AsFloat(0)*10+0.5);
	const bool interlaced = args[15].AsBool(false);

    int defx, defy;
    switch (align) {
	 case 1: case 4: case 7: defx = 8; break;
     case 2: case 5: case 8: defx = -1; break;
     case 3: case 6: case 9: defx = clip->GetVideoInfo().width-8; break;
     default: defx = 8; break; }
    switch (align) {
     case 1: case 2: case 3: defy = clip->GetVideoInfo().height-2; break;
     case 4: case 5: case 6: defy = -1; break;
	 case 7: case 8: case 9: defy = 0; break;
     default: defy = (size+4)/8; break; }

    const int x = args[2].AsInt(defx);
    const int y = args[3].AsInt(defy);

    if ((align < 1) || (align > 9))
     env->ThrowError("Subtitle: Align values are 1 - 9 mapped to your numeric pad");

    PClip texted = new Subtitle(clip, text, x, y, first_frame, last_frame, font, size, text_color,
	                    halo_color, align, spc, multiline, lsp, font_width, font_angle, interlaced);
      
    convertColorFormatBackToOriginal(texted, vi, env);
    return texted;    
}


inline int CalcFontSize(int w, int h)
{
  enum { minFS=8, FS=128, minH=224, minW=388 };

  const int ws = (w < minW) ? (FS*w)/minW : FS;
  const int hs = (h < minH) ? (FS*h)/minH : FS;
  const int fs = (ws < hs) ? ws : hs;
  return ( (fs < minFS) ? minFS : fs );
}


/***********************************
 *******   FilterInfo Filter    ******
 **********************************/


FilterInfo::FilterInfo( PClip _child, VideoInfo const& _viIn)
: GenericVideoFilter(_child), viIn(_viIn)
  , fontname("Courier New")
  , textcolor(vi.IsYUV() ? 0xD21092 : 0xFFFF00)
  , halocolor(vi.IsYUV() ? 0x108080 : 0)
{
}



FilterInfo::~FilterInfo(void) 
{
}

const char* t_YV12="YV12";
const char* t_YUY2="YUY2";
const char* t_RGB32="RGB32";
const char* t_RGB24="RGB24";
/* For 2.6
const char* t_YV24="YV24";
const char* t_Y8="Y8";
const char* t_YV16="YV16";
const char* t_Y41P="YUV 411 Planar";
*/
const char* t_INT8="Integer 8 bit";
const char* t_INT16="Integer 16 bit";
const char* t_INT24="Integer 24 bit";
const char* t_INT32="Integer 32 bit";
const char* t_FLOAT32="Float 32 bit";
const char* t_YES="YES";
const char* t_NO="NO";
const char* t_NONE="NONE";
const char* t_TFF ="Top Field First            ";
const char* t_BFF ="Bottom Field First         ";
const char* t_ATFF="Assumed Top Field First    ";
const char* t_ABFF="Assumed Bottom Field First ";
const char* t_STFF="Top Field (Separated)      ";
const char* t_SBFF="Bottom Field (Separated)   ";


string GetCpuMsg(IScriptEnvironment * env)
{
  int flags = env->GetCPUFlags();
  stringstream ss;  

  if (flags & CPUF_FPU)
    ss << "x87  ";
  if (flags & CPUF_MMX)
    ss << "MMX  ";
  if (flags & CPUF_INTEGER_SSE)
    ss << "ISSE  ";
  if (flags & CPUF_SSE)
    ss << "SSE  ";
  if (flags & CPUF_SSE2)
    ss << "SSE2 ";
  if (flags & CPUF_SSE3)
    ss << "SSE3 ";
  if (flags & CPUF_3DNOW)
    ss << "3DNOW ";
  if (flags & CPUF_3DNOW_EXT)
    ss << "3DNOW_EXT";

  return ss.str();
}


PVideoFrame FilterInfo::GetFrame(int n, IScriptEnvironment* env) 
{
    PVideoFrame frame = child->GetFrame(n, env);
  
    const char* c_space;
    const char* s_type = t_NONE;
    const char* s_parity;
    if (viIn.IsRGB24()) c_space=t_RGB24;
    if (viIn.IsRGB32()) c_space=t_RGB32;
    if (viIn.IsYV12()) c_space=t_YV12;
    if (viIn.IsYUY2()) c_space=t_YUY2;
/* For 2.6
    if (viIn.IsYV24()) c_space=t_YV24;
    if (viIn.IsY8()) c_space=t_Y8;
    if (viIn.IsYV16()) c_space=t_YV16;
    if (viIn.IsYV411()) c_space=t_Y41P;
*/

    if (viIn.SampleType()==SAMPLE_INT8) s_type=t_INT8;
    if (viIn.SampleType()==SAMPLE_INT16) s_type=t_INT16;
    if (viIn.SampleType()==SAMPLE_INT24) s_type=t_INT24;
    if (viIn.SampleType()==SAMPLE_INT32) s_type=t_INT32;
    if (viIn.SampleType()==SAMPLE_FLOAT) s_type=t_FLOAT32;
    if (viIn.IsFieldBased()) {
      if (child->GetParity(n)) {
        s_parity = t_STFF;
      } else {
        s_parity = t_SBFF;
      }
    } else {
      if (child->GetParity(n)) {
        s_parity = viIn.IsTFF() ? t_ATFF : t_TFF;
      } else {
        s_parity = viIn.IsBFF() ? t_ABFF : t_BFF;
      }
    }
    char text[512];
	int tlen;
    RECT r= { 32, 16, min(3440,viIn.width*8), 900*2 };
    
    int vLenInMsecs = (int)(1000.0 * (double)viIn.num_frames * (double)viIn.fps_denominator / (double)viIn.fps_numerator);
    int cPosInMsecs = (int)(1000.0 * (double)n * (double)viIn.fps_denominator / (double)viIn.fps_numerator);

    tlen = snprintf(text, sizeof(text),
      "Frame: %8u of %-8u\n"                                //  28
      "Time: %02d:%02d:%02d:%03d of %02d:%02d:%02d:%03d\n"  //  35
      "ColorSpace: %s\n"                                    //  18=13+5
      "Width:%4u pixels, Height:%4u pixels.\n"              //  39
      "Frames per second: %7.4f (%u/%u)\n"                  //  51=31+20
      "FieldBased (Separated) Video: %s\n"                  //  35=32+3
      "Parity: %s\n"                                        //  35=9+26
      "Video Pitch: %5u bytes.\n"                           //  25
      "Has Audio: %s\n"                                     //  15=12+3
      , n, viIn.num_frames
      , (cPosInMsecs/(60*60*1000)), (cPosInMsecs/(60*1000))%60 ,(cPosInMsecs/1000)%60, cPosInMsecs%1000,
        (vLenInMsecs/(60*60*1000)), (vLenInMsecs/(60*1000))%60 ,(vLenInMsecs/1000)%60, vLenInMsecs%1000 
      , c_space
      , viIn.width, viIn.height
      , (float)viIn.fps_numerator/(float)viIn.fps_denominator, viIn.fps_numerator, viIn.fps_denominator
      , viIn.IsFieldBased() ? t_YES : t_NO
      , s_parity
      , frame->GetPitch()
      , viIn.HasAudio() ? t_YES : t_NO
    );
    if (viIn.HasAudio()) {
      int aLenInMsecs = (int)(1000.0 * (double)viIn.num_audio_samples / (double)viIn.audio_samples_per_second);
	  tlen += snprintf(text+tlen, sizeof(text)-tlen,
		"Audio Channels: %-8u\n"                              //  25
		"Sample Type: %s\n"                                   //  28=14+14
		"Samples Per Second: %5d\n"                           //  26
		"Audio length: %ld samples. %02d:%02d:%02d:%03d\n"  //  57=37+20
		, viIn.AudioChannels()
		, s_type
		, viIn.audio_samples_per_second
		, viIn.num_audio_samples,
		  (aLenInMsecs/(60*60*1000)), (aLenInMsecs/(60*1000))%60, (aLenInMsecs/1000)%60, aLenInMsecs%1000
	  );
    }
    else {
	  strcpy(text+tlen,"\n");
	  tlen += 1;
    }
    tlen += snprintf(text+tlen, sizeof(text)-tlen,
      "CPU detected: %s\n"                                  //  60=15+45
      , GetCpuMsg(env).c_str()                              // 442
    );
    
    env->MakeWritable(&frame);
    
    AvxTextRender::FrameBuffer trd(frame->GetWritePtr(), vi.width, vi.height, frame->GetPitch());
    TextConfig txtConfig("Arial", CalcFontSize(vi.width, vi.height)/8, 0.75, textcolor, halocolor);    
    TextLayout txtLayout(TextLayout::Rect(r.left, r.top, r.right, r.bottom), TextLayout::VCenter, TextLayout::Left);    
 
    try
    {
        AvxTextRender::RenderText(text, trd, txtConfig, txtLayout);
    }
    catch(AvxException &e)
    {
        env->ThrowError(Utf8String(e.GetMsg()).c_str());
    }

    return frame;
}

AVSValue __cdecl FilterInfo::Create(AVSValue args, void*, IScriptEnvironment* env) 
{
    AVXLOG_INFO("FilterInfo::%s", __FUNCTION__);

    PClip clip = args[0].AsClip();
    
    VideoInfo const& vi = clip->GetVideoInfo();
    convertColorFormatToRGB24(clip, vi, env);
    
    PClip texted = new FilterInfo(clip, vi);
    
    convertColorFormatBackToOriginal(texted, vi, env);
    return texted;        
}










/************************************
 *******    Compare Filter    *******
 ***********************************/


Compare::Compare(PClip _child1, PClip _child2, const char* channels, const char *fname, bool _show_graph, IScriptEnvironment* env)
  : GenericVideoFilter(_child1),
    fontsize(128), textcolor(vi.IsYUV() ? 0xD21092 : 0xFFFF00), halocolor(vi.IsYUV() ? 0x108080 : 0),
    child2(_child2),
    log(NULL),
    show_graph(_show_graph),
    framecount(0)
{
  const VideoInfo& vi2 = child2->GetVideoInfo();
  psnrs = 0;

  if (!vi.IsSameColorspace(vi2))
    env->ThrowError("Compare: Clips are not same colorspace.");

  if (vi.width != vi2.width || vi.height != vi2.height)
    env->ThrowError("Compare: Clips must have same size.");

  if (!(vi.IsRGB24() || vi.IsYUY2() || vi.IsRGB32() || vi.IsPlanar()))
    env->ThrowError("Compare: Clips have unknown pixel format. RGB24, RGB32, YUY2 and YUV Planar supported.");

  if (channels[0] == 0) {
    if (vi.IsRGB())
      channels = "RGB";
    else if (vi.IsYUV())
      channels = "YUV";
    else env->ThrowError("Compare: Clips have unknown colorspace. RGB and YUV supported.");
  }

  planar_plane = 0;
  mask = 0;
  for (int i = 0; i < (int)strlen(channels); i++) {
    if (vi.IsRGB()) {
      switch (channels[i]) {
      case 'b':
      case 'B': mask |= 0x000000ff; break;
      case 'g':
      case 'G': mask |= 0x0000ff00; break;
      case 'r':
      case 'R': mask |= 0x00ff0000; break;
      case 'a':
      case 'A': mask |= 0xff000000; if (vi.IsRGB32()) break; // else fall thru
      default: env->ThrowError("Compare: invalid channel: %c", channels[i]);
      }
      if (vi.IsRGB24()) mask &= 0x00ffffff;   // no alpha channel in RGB24
	} else if (vi.IsPlanar()) {
      switch (channels[i]) {
      case 'y':
      case 'Y': mask |= 0xffffffff; planar_plane |= PLANAR_Y; break;
      case 'u':
      case 'U': mask |= 0xffffffff; planar_plane |= PLANAR_U; break;
      case 'v':
      case 'V': mask |= 0xffffffff; planar_plane |= PLANAR_V; break;
      default: env->ThrowError("Compare: invalid channel: %c", channels[i]);
      }
    } else {  // YUY2
      switch (channels[i]) {
      case 'y':
      case 'Y': mask |= 0x00ff00ff; break;
      case 'u':
      case 'U': mask |= 0x0000ff00; break;
      case 'v':
      case 'V': mask |= 0xff000000; break;
      default: env->ThrowError("Compare: invalid channel: %c", channels[i]);
      }
    }
  }

  masked_bytes = 0;
  for (DWORD temp = mask; temp != 0; temp >>=8)
    masked_bytes += (temp & 1);

  if (fname[0] != 0) {
    log = fopen(fname, "wt");
    if (log) {
      AVXLOG_INFO("Comparing channel(s) %s\n\n",channels);
      AVXLOG_INFO("%s", "           Mean               Max    Max             \n");
      AVXLOG_INFO("%s", "         Absolute     Mean    Pos.   Neg.            \n");
      AVXLOG_INFO("%s", " Frame     Dev.       Dev.    Dev.   Dev.  PSNR (dB) \n");
      AVXLOG_INFO("%s", "-----------------------------------------------------\n");
    } else
      env->ThrowError("Compare: unable to create file %s", fname);
  } else {
    psnrs = new int[vi.num_frames];
    if (psnrs)
      for (int i = 0; i < vi.num_frames; i++)
        psnrs[i] = 0;
  }

}


Compare::~Compare()
{
  if (log) {
    AVXLOG_INFO("\n\n\nTotal frames processed: %d\n\n", framecount);
    AVXLOG_INFO("%s", "                           Minimum   Average   Maximum\n");
    AVXLOG_INFO("Mean Absolute Deviation: %9.4f %9.4f %9.4f\n", MAD_min, MAD_tot/framecount, MAD_max);
    AVXLOG_INFO("         Mean Deviation: %+9.4f %+9.4f %+9.4f\n", MD_min, MD_tot/framecount, MD_max);
    AVXLOG_INFO("                   PSNR: %9.4f %9.4f %9.4f\n", PSNR_min, PSNR_tot/framecount, PSNR_max);
    double PSNR_overall = 10.0 * log10(bytecount_overall * 255.0 * 255.0 / SSD_overall);
    AVXLOG_INFO("           Overall PSNR: %9.4f\n", PSNR_overall);
    fclose(log);
  }
  if (psnrs) delete[] psnrs;
}


AVSValue __cdecl Compare::Create(AVSValue args, void*, IScriptEnvironment *env)
{
  PClip clip = args[0].AsClip();
  PClip clip1 = args[1].AsClip();
  
  VideoInfo vi = clip->GetVideoInfo();
  convertColorFormatToRGB24(clip, vi, env);
  convertColorFormatToRGB24(clip1, vi, env);
  
  PClip texted = new Compare(clip,     // clip
                             clip1,     // base clip
                             args[2].AsString(""),   // channels
                             args[3].AsString(""),   // logfile
                             args[4].AsBool(true),   // show_graph
                             env);
  convertColorFormatBackToOriginal(texted, vi, env);
  return texted;
}

void Compare::Compare_ISSE(DWORD mask, int incr,
                           const BYTE * f1ptr, int pitch1, 
                           const BYTE * f2ptr, int pitch2,
                           int rowsize, int height,
                           int &SAD_sum, int &SD_sum, int &pos_D,  int &neg_D, double &SSD_sum)
{ 
    // rowsize multiple of 8 for YUV Planar, RGB32 and YUY2; 6 for RGB24
    // incr must be 3 for RGB24 and 4 for others
    // SAD_sum, SD_sum, SSD_sum are incremented (must be properly initialized)
    int SAD = 0, SD = 0;
    const int incr2 = incr * 2;

    int64_t iSSD   = 0;
    int64_t mask64 = (int64_t)mask << ((incr == 3) ? 24: 32) | mask;
    uint64_t pos_D8 = 0, neg_D8 = 0;
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
    __asm {
      mov     esi, f1ptr
      mov     edi, f2ptr
      add     esi, rowsize
      add     edi, rowsize
      xor     eax, eax      ; sum of squared differences low
      xor     edx, edx      ; sum of squared differences high
      pxor    mm7, mm7      ; sum of absolute differences
      pxor    mm6, mm6      ; zero
      pxor    mm5, mm5      ; sum of differences
comp_loopy:
      mov     ecx, rowsize
      neg     ecx
      pxor    mm4, mm4      ; sum of squared differences (row_SSD)
comp_loopx:
      movq    mm0, [esi+ecx]
      movq    mm1, [edi+ecx]
      pand    mm0, mask64
      pand    mm1, mask64
      movq    mm2, mm0
      psubusb   mm0, mm1
      psubusb   mm1, mm2

      ; maximum positive and negative differences
      movq    mm3, pos_D8
      movq    mm2, neg_D8
      pmaxub    mm3, mm0
      pmaxub    mm2, mm1
      movq    pos_D8, mm3
      movq    neg_D8, mm2

       movq   mm2, mm0      ; SSD calculations are indented
      psadbw    mm0, mm6
       por    mm2, mm1
      psadbw    mm1, mm6
       movq   mm3, mm2
       punpcklbw  mm2, mm6
       punpckhbw  mm3, mm6
       pmaddwd  mm2, mm2
      paddd   mm7, mm0
      paddd   mm5, mm0
       pmaddwd  mm3, mm3
      paddd   mm7, mm1
       paddd    mm4, mm2
      psubd   mm5, mm1
      add     ecx, incr2
       paddd    mm4, mm3      ; keep two counts at once
      jne     comp_loopx

      add     esi, pitch1
      add     edi, pitch2
      movq    mm3, mm4
      punpckhdq mm4, mm6
      paddd   mm3, mm4
      movd    ecx, mm3
      add     eax, ecx
      adc     edx, 0
      dec     height
      jne     comp_loopy

      movd    SAD, mm7
      movd    SD, mm5
      mov     DWORD PTR [iSSD], eax
      mov     DWORD PTR [iSSD+4], edx
      emms
    }
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
    SSD_sum += (double)iSSD;
    for (int i=0; i<incr2; i++) {
      pos_D = max(pos_D, (int)(pos_D8 & 0xff));
      neg_D = max(neg_D, (int)(neg_D8 & 0xff));
      pos_D8 >>= 8;
      neg_D8 >>= 8;
    }
    neg_D = -neg_D;
    SAD_sum += SAD;
    SD_sum  += SD;
}


PVideoFrame __stdcall Compare::GetFrame(int n, IScriptEnvironment* env)
{
  PVideoFrame f1 = child->GetFrame(n, env);
  PVideoFrame f2 = child2->GetFrame(n, env);

  int SD = 0;
  int SAD = 0;
  int pos_D = 0;
  int neg_D = 0;
  double SSD = 0;
  int row_SSD;

  int bytecount = 0;
  const int incr = vi.IsRGB24() ? 3 : 4;

  if (vi.IsRGB24() || vi.IsYUY2() || vi.IsRGB32()) {

    const BYTE* f1ptr = f1->GetReadPtr();
    const BYTE* f2ptr = f2->GetReadPtr();
    const int pitch1 = f1->GetPitch();
    const int pitch2 = f2->GetPitch();
    const int rowsize = f1->GetRowSize();
    const int height = f1->GetHeight();

    bytecount = rowsize * height * masked_bytes / 4;

    if (((rowsize & 7) && !vi.IsRGB24()) ||       // rowsize must be a multiple or 8 for RGB32 and YUY2
       ((rowsize % 6) && vi.IsRGB24()) ||          // or 6 for RGB24
       !(env->GetCPUFlags() & CPUF_INTEGER_SSE)) { // to use the ISSE routine
      for (int y = 0; y < height; y++) {
        row_SSD = 0;
        for (int x = 0; x < rowsize; x += incr) {
          DWORD p1 = *(DWORD *)(f1ptr + x) & mask;
          DWORD p2 = *(DWORD *)(f2ptr + x) & mask;
          int d0 = (p1 & 0xff) - (p2 & 0xff);
          int d1 = ((p1 >> 8) & 0xff) - ((p2 & 0xff00) >> 8);
          int d2 = ((p1 >>16) & 0xff) - ((p2 & 0xff0000) >> 16);
          int d3 = (p1 >> 24) - (p2 >> 24);
          SD += d0 + d1 + d2 + d3;
          SAD += abs(d0) + abs(d1) + abs(d2) + abs(d3);
          row_SSD += d0 * d0 + d1 * d1 + d2 * d2 + d3 * d3;
          pos_D = max(max(max(max(pos_D,d0),d1),d2),d3);
          neg_D = min(min(min(min(neg_D,d0),d1),d2),d3);
        }
        SSD += row_SSD;
        f1ptr += pitch1;
        f2ptr += pitch2;
      }
    } else {        // ISSE version; rowsize multiple of 8 for RGB32 and YUY2; 6 for RGB24
      Compare_ISSE(mask, incr, f1ptr, pitch1, f2ptr, pitch2, rowsize, height,
                   SAD, SD, pos_D, neg_D, SSD);
    }
  }
  else { // Planar
  
    int planes[3] = {PLANAR_Y, PLANAR_U, PLANAR_V};
    for (int p=0; p<3; p++) {
      const int plane = planes[p];

	  if (planar_plane & plane) {

        const BYTE* f1ptr = f1->GetReadPtr(plane);
        const BYTE* f2ptr = f2->GetReadPtr(plane);
        const int pitch1 = f1->GetPitch(plane);
        const int pitch2 = f2->GetPitch(plane);
        const int rowsize = f1->GetRowSize(plane);
        const int height = f1->GetHeight(plane);

        bytecount += rowsize * height;

        if ((rowsize & 7) || !(env->GetCPUFlags() & CPUF_INTEGER_SSE)) { 
          // rowsize must be a multiple 8 to use the ISSE routine
          for (int y = 0; y < height; y++) {
            row_SSD = 0;
            for (int x = 0; x < rowsize; x += 1) {
              int p1 = *(f1ptr + x);
              int p2 = *(f2ptr + x);
              int d0 = p1 - p2;
              SD += d0;
              SAD += abs(d0);
              row_SSD += d0 * d0;
              pos_D = max(pos_D,d0);
              neg_D = min(neg_D,d0);
            }
            SSD += row_SSD;
            f1ptr += pitch1;
            f2ptr += pitch2;
          }
        }
        else {
         Compare_ISSE(mask, incr, f1ptr, pitch1, f2ptr, pitch2, rowsize, height,
                      SAD, SD, pos_D, neg_D, SSD);
        }
      }
    }
  }

  double MAD = (double)SAD / bytecount;
  double MD = (double)SD / bytecount;
  if (SSD == 0.0) SSD = 1.0;
  double PSNR = 10.0 * log10(bytecount * 255.0 * 255.0 / SSD);

  framecount++;
  if (framecount == 1) {
    MAD_min = MAD_tot = MAD_max = MAD;
    MD_min = MD_tot = MD_max = MD;
    PSNR_min = PSNR_tot = PSNR_max = PSNR;
    bytecount_overall = double(bytecount);
    SSD_overall = SSD;
  } else {
    MAD_min = min(MAD_min, MAD);
    MAD_tot += MAD;
    MAD_max = max(MAD_max, MAD);
    MD_min = min(MD_min, MD);
    MD_tot += MD;
    MD_max = max(MD_max, MD);
    PSNR_min = min(PSNR_min, PSNR);
    PSNR_tot += PSNR;
    PSNR_max = max(PSNR_max, PSNR);
    bytecount_overall += double(bytecount);
    SSD_overall += SSD;  
  }

  if (log)
  {
      AVXLOG_INFO("%6u  %8.4f  %+9.4f  %3d    %3d    %8.4f\n", n, MAD, MD, pos_D, neg_D, PSNR);
  }
  else {
    env->MakeWritable(&f1);
    BYTE* dstp = f1->GetWritePtr();
    int dst_pitch = f1->GetPitch();

	char text[400];
	RECT r= { 32, 16, min(3440,vi.width*8), 768+128 };
	double PSNR_overall = 10.0 * log10(bytecount_overall * 255.0 * 255.0 / SSD_overall);
	snprintf(text, sizeof(text), 
		"       Frame:  %-8u(   min  /   avg  /   max  )\n"
		"Mean Abs Dev:%8.4f  (%7.3f /%7.3f /%7.3f )\n"
		"    Mean Dev:%+8.4f  (%+7.3f /%+7.3f /%+7.3f )\n"
		" Max Pos Dev:%4d  \n"
		" Max Neg Dev:%4d  \n"
		"        PSNR:%6.2f dB ( %6.2f / %6.2f / %6.2f )\n"
		"Overall PSNR:%6.2f dB\n", 
		n,
		MAD, MAD_min, MAD_tot / framecount, MD_max,
		MD, MD_min, MD_tot / framecount, MD_max,
		pos_D,
		neg_D,
		PSNR, PSNR_min, PSNR_tot / framecount, PSNR_max,
		PSNR_overall
	);
	  
    AvxTextRender::FrameBuffer trd(f1->GetWritePtr(), vi.width, vi.height, f1->GetPitch());
	TextConfig txtConfig("Courier New",  fontsize/8, 0.75, textcolor, halocolor);
    TextLayout txtLayout(TextLayout::Rect(vi.width/8, vi.height/2, vi.width-4*8, vi.height-4*8), TextLayout::VCenter, TextLayout::Left);
	  
    try
    {
        AvxTextRender::RenderText(text, trd, txtConfig, txtLayout);
    }
    catch(AvxException &e)
    {
        env->ThrowError(Utf8String(e.GetMsg()).c_str());
    }

    if (show_graph) {
      // original idea by Marc_FD
      psnrs[n] = min((int)(PSNR + 0.5), 100);
      if (vi.height > 196) {
        if (vi.IsYUY2()) {
          dstp += (vi.height - 1) * dst_pitch;
          for (int y = 0; y <= 100; y++) {            
            for (int x = max(0, vi.width - n - 1); x < vi.width; x++) {
              if (y <= psnrs[n - vi.width + 1 + x]) {
                if (y <= psnrs[n - vi.width + 1 + x] - 2) {
                  dstp[x << 1] = 16;                // Y
                  dstp[((x & -1) << 1) + 1] = 0x80; // U
                  dstp[((x & -1) << 1) + 3] = 0x80; // V
                } else {
                  dstp[x << 1] = 235;               // Y
                  dstp[((x & -1) << 1) + 1] = 0x80; // U
                  dstp[((x & -1) << 1) + 3] = 0x80; // V
                }
              }
            } // for x
            dstp -= dst_pitch;
          } // for y
        }
		else if (vi.IsPlanar()) {
          dstp += (vi.height - 1) * dst_pitch;
          for (int y = 0; y <= 100; y++) {            
            for (int x = max(0, vi.width - n - 1); x < vi.width; x++) {
              if (y <= psnrs[n - vi.width + 1 + x]) {
                if (y <= psnrs[n - vi.width + 1 + x] - 2) {
                  dstp[x] = 16;                // Y
                } else {
                  dstp[x] = 235;               // Y
                }
              }
            } // for x
            dstp -= dst_pitch;
          } // for y
        } else {  // RGB
          for (int y = 0; y <= 100; y++) {
            for (int x = max(0, vi.width - n - 1); x < vi.width; x++) {
              if (y <= psnrs[n - vi.width + 1 + x]) {
                const int xx = x * incr;
                if (y <= psnrs[n - vi.width + 1 + x] -2) {
                  dstp[xx] = 0x00;        // B
                  dstp[xx + 1] = 0x00;    // G
                  dstp[xx + 2] = 0x00;    // R
                } else {
                  dstp[xx] = 0xFF;        // B
                  dstp[xx + 1] = 0xFF;    // G
                  dstp[xx + 2] = 0xFF;    // R
                }
              }
            } // for x
            dstp += dst_pitch;
          } // for y
        } // RGB
      } // height > 100
    } // show_graph
  } // no logfile

  return f1;
}


/************************************
 *******   Helper Functions    ******
 ***********************************/

void ApplyMessage( PVideoFrame* frame, const VideoInfo& vi, const char* message, int size, 
                   int textcolor, int halocolor, int bgcolor, IScriptEnvironment* env ) 
{    
  AVXLOG_INFO("ApplyMessage::%s", __FUNCTION__);
    
  if (vi.IsYUV()) {
    textcolor = RGB2YUV(textcolor);
    halocolor = RGB2YUV(halocolor);
  }
  
  AvxTextRender::FrameBuffer trd((*frame)->GetWritePtr(), vi.width, vi.height, (*frame)->GetPitch());
  TextConfig txtConfig("Arial", size, 0.75, textcolor, halocolor);
  TextLayout txtLayout(TextLayout::Rect(4*8, 4*8, vi.width - 4*8, vi.height-4*8), TextLayout::VCenter, TextLayout::Left);
  
  try
  {
    AvxTextRender::RenderText(message, trd, txtConfig, txtLayout, AvxTextRender::RenderOptions_ResizeToFit);
  }
  catch(AvxException &e)
  {
      env->ThrowError(Utf8String(e.GetMsg()).c_str());
  }
}

void GetApproximateCharacterWidth(const char* pStrFontFamily, int nFontSize, int textcolor, int halocolor, int& nCharWidth)
{
    TextConfig txtConfig(pStrFontFamily, nFontSize, 0, textcolor, halocolor);
    
    AvxTextRender::GetApproximateCharacterWidth(txtConfig, nCharWidth);
}

}; // namespace avxsynth
