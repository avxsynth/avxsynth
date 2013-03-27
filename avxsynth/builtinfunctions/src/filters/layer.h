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




// Avisynth filter: Layer 
// by "poptones" (poptones@myrealbox.com)


#ifndef __Layer_H__
#define __Layer_H__

#include "common/include/internal.h"

namespace avxsynth {
	
/********************************************************************
********************************************************************/
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE      

class Mask : public IClip 
/**
  * Class for overlaying a mask clip on a video clip
 **/
{ 
public:
  Mask(PClip _child1, PClip _child2, IScriptEnvironment* env);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);

  inline virtual void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env)
    { child1->GetAudio(buf, start, count, env); }
  inline virtual const VideoInfo& __stdcall GetVideoInfo() 
    { return vi; }
  inline virtual bool __stdcall GetParity(int n) 
    { return child1->GetParity(n); }

  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);
  void __stdcall SetCacheHints(int cachehints,size_t frame_range) { };

private:
  const PClip child1, child2;
  VideoInfo vi;
  int mask_frames;
  bool YUYmask;

};



class ColorKeyMask : public GenericVideoFilter
/**
  * Class for setting a mask on a video clip based on a color key
**/
{
public:
  ColorKeyMask(PClip _child, int _color, int _tolB, int _tolG, int _tolR, IScriptEnvironment *env);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment *env);

  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);

private:
  const int color, tolB, tolG, tolR;
};

#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE


class ResetMask : public GenericVideoFilter
/**
  * Class to set the mask to all-opaque
**/
{
public:
  ResetMask(PClip _child, IScriptEnvironment* env);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);

  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);
};


#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE      

class Invert : public GenericVideoFilter
/**
  * Class to invert selected RGBA channels
**/
{
public:
  Invert(PClip _child, const char * _channels, IScriptEnvironment* env);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);

  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);
private:
  void ConvertFrame(BYTE* frame, int pitch, int rowsize, int height, int mask);
  const char * channels;
};
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE


class ShowChannel : public GenericVideoFilter
/**
  * Class to set the RGB components to the alpha mask
**/
{
public:
  ShowChannel(PClip _child, const char * _pixel_type, int _channel, IScriptEnvironment* env);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);

  static AVSValue __cdecl Create(AVSValue args, void* channel, IScriptEnvironment* env);
private:
  const int channel;
  const int input_type;
};




class MergeRGB : public GenericVideoFilter
/**
  * Class to load the RGB components from specified clips
**/
{
public:
  MergeRGB(PClip _child, PClip _blue, PClip _green, PClip _red, PClip _alpha,
           const char * _pixel_type, IScriptEnvironment* env);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);

  static AVSValue __cdecl Create(AVSValue args, void* mode, IScriptEnvironment* env);
private:
  const PClip blue, green, red, alpha;
  const VideoInfo &viB, &viG, &viR, &viA;
  const char * myname;
};



#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE      

class Layer: public IClip 
/**
  * Class for layering two clips on each other, combined by various functions
 **/ 
{ 
public:
  Layer( PClip _child1, PClip _child2, const char _op[], int _lev, int _x, int _y, 
         int _t, bool _chroma, IScriptEnvironment* env );
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);

  inline virtual void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) 
    { child1->GetAudio(buf, start, count, env); }
  inline virtual const VideoInfo& __stdcall GetVideoInfo() 
    { return vi; }
  inline virtual bool __stdcall GetParity(int n) 
    { return child1->GetParity(n); }
  void __stdcall SetCacheHints(int cachehints,size_t frame_range) { };

  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);

private:
  const PClip child1, child2;
  const int levelB, T;
  const bool chroma;
  int map1[256], map2[256], ydest, xdest, ysrc, xsrc, ofsX, ofsY, ycount, xcount, overlay_frames;
  const  char* Op;
  VideoInfo vi;

};
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE



class Subtract : public IClip 
/**
  * Class for subtracting one clip from another
 **/
{
public:
  Subtract(PClip _child1, PClip _child2, IScriptEnvironment* env);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);

  inline virtual void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) 
    { child1->GetAudio(buf, start, count, env);  }
  inline virtual const VideoInfo& __stdcall GetVideoInfo() 
    { return vi; }
  inline virtual bool __stdcall GetParity(int n) 
    { return child1->GetParity(n); }
  void __stdcall SetCacheHints(int cachehints,size_t frame_range) { };

  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);

private:
  const PClip child1, child2;
  VideoInfo vi;

// Common to all instances
  static bool DiffFlag;
  static BYTE Diff[513];

};

}; // namespace avxsynth

#endif  // __Layer_H__
