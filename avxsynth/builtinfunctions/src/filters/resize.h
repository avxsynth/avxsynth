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

#ifndef __Resize_H__
#define __Resize_H__

#include "internal.h"

namespace avxsynth {
	
/********************************************************************
********************************************************************/

class VerticalReduceBy2 : public GenericVideoFilter
/** 
  * This class exposes a video filter for reducing a video's height by half.  Input is one clip,
  * output is another.
 **/
{  
public:
  VerticalReduceBy2(PClip _child, IScriptEnvironment* env);
  
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE    
  void mmx_process(PVideoFrame src,BYTE* dstp, int dst_pitch, int plane);
  void mmx_process(const BYTE* srcp, int src_pitch, int row_size, BYTE* dstp, int dst_pitch, int height);
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
  void c_process(const BYTE* srcp, int src_pitch, int src_height, int row_size, BYTE* dstp, int dst_pitch, int height);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env)  { 
    return new VerticalReduceBy2(args[0].AsClip(),env); 
  }

private:
  int original_height;
};


class HorizontalReduceBy2 : public GenericVideoFilter 
/** 
  * This class exposes a video filter for reducing a video's width by half.  Input is one clip,
  * output is another.
 **/
{
public:
  HorizontalReduceBy2(PClip _child, IScriptEnvironment* env);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE    
  void isse_process_yuy2(PVideoFrame src,BYTE* dstp, int dst_pitch);
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE 
  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env) {
    return new HorizontalReduceBy2(args[0].AsClip(), env);
  }

private:
  BYTE *mybuffer;
  int source_width;
};


AVSValue __cdecl Create_ReduceBy2(AVSValue args, void*, IScriptEnvironment* env);

}; // namespace avxsynth
#endif  // __Resize_H__
