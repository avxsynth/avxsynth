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

#ifndef __Text_overlay_H__
#define __Text_overlay_H__

#include <internal.h>
#include <convert/convert.h>

namespace avxsynth {
	
/********************************************************************
********************************************************************/


class ShowFrameNumber : public GenericVideoFilter 
/**
  * Class to display frame number on a video clip
 **/
{  
public:
  ShowFrameNumber(PClip _child, bool _scroll, int _offset, int _x, int _y, const char _fontname[], int _size,
			int _textcolor, int _halocolor, int font_width, int font_angle, IScriptEnvironment* env);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);

  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);

private:
  const char* const fontname;
  const int textcolor, halocolor;
  const bool scroll;
  const int offset;
  const int size, x, y;
};



class ShowSMPTE : public GenericVideoFilter 
/**
  * Class to display SMPTE codes on a video clip
 **/
{
public:
  ShowSMPTE(PClip _child, double _rate, const char* _offset, int _offset_f, int _x, int _y, const char _fontname[], int _size,
			int _textcolor, int _halocolor, int font_width, int font_angle, IScriptEnvironment* env);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);

  static AVSValue __cdecl CreateSMTPE(AVSValue args, void*, IScriptEnvironment* env);
  static AVSValue __cdecl CreateTime(AVSValue args, void*, IScriptEnvironment* env);

private:
  const char* const fontname;
  const int textcolor, halocolor, size;
  int rate;
  int offset_f;
  const int x, y;
  bool dropframe;
};




class Subtitle : public GenericVideoFilter 
/**
  * Subtitle creation class
 **/
{
public:
  Subtitle( PClip _child, const char _text[], int _x, int _y, int _firstframe, int _lastframe, 
            const char _fontname[], int _size, int _textcolor, int _halocolor, int _align, 
            int _spc, bool _multiline, int _lsp, int _font_width, int _font_angle, bool _interlaced );
  virtual ~Subtitle(void);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  
  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);  

private:
  const int x, y, firstframe, lastframe, size, lsp, font_width, font_angle;
  const bool multiline, interlaced;
  const int textcolor, halocolor, align, spc;
  const char* const fontname;
  const char* const text;
};


class FilterInfo : public GenericVideoFilter 
/**
  * FilterInfo creation class
 **/
{
public:
  FilterInfo( PClip _child, VideoInfo const& viIn);
  virtual ~FilterInfo(void);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  
  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);  

private:
  HDC hdcAntialias;
  VideoInfo const& viIn;
  const char* const fontname;
  const int textcolor, halocolor;
};


class Compare : public GenericVideoFilter
/**
  * Compare two clips frame by frame and display fidelity measurements (with optionnal logging to file)
 **/
{
public:
  Compare(PClip _child1, PClip _child2, const char* channels, const char *fname, bool _show_graph, IScriptEnvironment* env);
  ~Compare();
  static AVSValue __cdecl Create(AVSValue args, void* , IScriptEnvironment* env);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
private:
  const int fontsize, textcolor, halocolor;
  PClip child2;
  DWORD mask;
  int masked_bytes;
  FILE* log;
  int* psnrs;
  bool show_graph;
  double PSNR_min, PSNR_tot, PSNR_max;
  double MAD_min, MAD_tot, MAD_max;
  double MD_min, MD_tot, MD_max;
  double bytecount_overall, SSD_overall;
  int framecount;
  int planar_plane;
  void Compare_ISSE(DWORD mask, int incr,
	                const BYTE * f1ptr, int pitch1, 
				    const BYTE * f2ptr, int pitch2,
					int rowsize, int height,
				    int &SAD_sum, int &SD_sum, int &pos_D,  int &neg_D, double &SSD_sum);

};



/**** Helper functions ****/

void ApplyMessage( PVideoFrame* frame, const VideoInfo& vi, const char* message, int size, 
                   int textcolor, int halocolor, int bgcolor, IScriptEnvironment* env );


}; // namespace avxsynth

#endif  // __Text_overlay_H__
