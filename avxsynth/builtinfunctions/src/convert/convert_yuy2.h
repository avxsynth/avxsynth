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

#ifndef __Convert_YUY2_H__
#define __Convert_YUY2_H__

#include "common/include/internal.h"
#include "convert_yv12.h"
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
#include "../core/softwire_helpers.h" // doesn't exist in source tree
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE

namespace avxsynth {
	
class ConvertToYUY2 : public GenericVideoFilter
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE      
, public CodeGenerator
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
/**
  * Class for conversions to YUY2
 **/
{
public:
  ConvertToYUY2(PClip _child, bool _dupl, bool _interlaced, const char *matrix, IScriptEnvironment* env);
  ~ConvertToYUY2();
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);

  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);

private:
  const bool interlaced;

protected:
  
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
  void GenerateAssembly(bool rgb24, bool dupl, bool sub, int w, IScriptEnvironment* env);
  void mmx_ConvertRGBtoYUY2(const BYTE *src,BYTE *dst,int src_pitch, int dst_pitch, int h);
  DynamicAssembledCode assembly;
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
  
  const int src_cs;  // Source colorspace
  int theMatrix;
  enum {Rec601=0, Rec709=1, PC_601=2, PC_709=3 };	// Note! convert_yuy2.cpp assumes these values

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE  
  // Variables for dynamic code.
  const BYTE* dyn_src;
  BYTE* dyn_dst;

  // These must be set BEFORE creating the generator, and CANNOT be changed at runtime!
  const __int64* dyn_cybgr;
  const __int64* dyn_fpix_mul;
  const int* dyn_fraction;
  const int* dyn_y1y2_mult;
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
};

class ConvertBackToYUY2 : public ConvertToYUY2
/**
  * Class for conversions to YUY2 (With Chroma copy)
 **/
{
public:
  ConvertBackToYUY2(PClip _child, const char *matrix, IScriptEnvironment* env);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);

  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);

private:
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE  
  void mmxYV24toYUY2(const unsigned char *py, const unsigned char *pu, const unsigned char *pv,
                     unsigned char *dst, int pitch1Y, int pitch1UV, int pitch2, int width, int height);
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
};

}; // namespace avxsynth

#endif // __Convert_YUY2_H__
