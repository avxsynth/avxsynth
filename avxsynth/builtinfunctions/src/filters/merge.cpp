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


// Avisynth filter: YUV merge / Swap planes
// by Klaus Post (kp@interact.dk)
// adapted by Richard Berg (avisynth-dev@richardberg.net)
// iSSE code by Ian Brabham


#include "common/include/stdafx.h"

#include "merge.h"

namespace avxsynth {


__declspec(align(8)) static __int64 I1=0x00ff00ff00ff00ff;  // Luma mask
__declspec(align(8)) static __int64 I2=0xff00ff00ff00ff00;  // Chroma mask
__declspec(align(8)) static __int64 rounder = 0x0000400000004000;  // (0.5)<<15 in each dword

/********************************************************************
***** Declare index of new filters for Avisynth's filter engine *****
********************************************************************/
#if 0  // Set to 1 to expose test harness
AVSFunction Merge_filters[] = {
  { "Merge", "cc[weight]f[test]i", MergeAll::Create },  // src, src2, weight
  { "MergeChroma", "cc[chromaweight]f[test]i", MergeChroma::Create },  // src, chroma src, weight
  { "MergeLuma", "cc[lumaweight]f[test]i", MergeLuma::Create },      // src, luma src, weight
  { 0 }
};
#define TEST(off, on) !!(test & on) || !(test & off) &&
#define TESTARG(n) args[n].AsInt(0) 
#else
AVSFunction Merge_filters[] = {
  { "Merge", "cc[weight]f", MergeAll::Create },  // src, src2, weight
  { "MergeChroma", "cc[chromaweight]f", MergeChroma::Create },  // src, chroma src, weight
  { "MergeLuma", "cc[lumaweight]f", MergeLuma::Create },      // src, luma src, weight
  { 0 }
};
#define TEST(off, on)
#define TESTARG(n) 0
#endif




/****************************
******   Merge Chroma   *****
****************************/

MergeChroma::MergeChroma(PClip _child, PClip _clip, float _weight, int _test, IScriptEnvironment* env)
  : GenericVideoFilter(_child), clip(_clip), weight(_weight), test(_test)
{
  const VideoInfo& vi2 = clip->GetVideoInfo();

  if (!vi.IsYUV() || !vi2.IsYUV())
    env->ThrowError("MergeChroma: YUV data only (no RGB); use ConvertToYUY2 or ConvertToYV12");

  if (!(vi.IsSameColorspace(vi2)))
    env->ThrowError("MergeChroma: YUV images must have same data type.");

  if (vi.width!=vi2.width || vi.height!=vi2.height)
    env->ThrowError("MergeChroma: Images must have same width and height!");

  if (weight<0.0f) weight=0.0f;
  if (weight>1.0f) weight=1.0f;
}


PVideoFrame __stdcall MergeChroma::GetFrame(int n, IScriptEnvironment* env)
{
  PVideoFrame src = child->GetFrame(n, env);

  if (weight<0.0001f) return src;

  PVideoFrame chroma = clip->GetFrame(n, env);

  const int h = src->GetHeight();
  const int w = src->GetRowSize()>>1; // width in pixels

  if (weight<0.9999f) {
    if (vi.IsYUY2()) {
      env->MakeWritable(&src);
      unsigned int* srcp = (unsigned int*)src->GetWritePtr();
      unsigned int* chromap = (unsigned int*)chroma->GetReadPtr();

      const int isrc_pitch = (src->GetPitch())>>2;  // int pitch (one pitch=two pixels)
      const int ichroma_pitch = (chroma->GetPitch())>>2;  // Ints

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
      ((TEST(4, 8) (env->GetCPUFlags() & CPUF_MMX)) ? mmx_weigh_chroma : weigh_chroma)
        (srcp,chromap,isrc_pitch,ichroma_pitch,w,h,(int)(weight*32768.0f),32768-(int)(weight*32768.0f));
#else   
    weigh_chroma(srcp,chromap,isrc_pitch,ichroma_pitch,w,h,(int)(weight*32768.0f),32768-(int)(weight*32768.0f));
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
        
    } else {  // Planar
      env->MakeWritable(&src);
      src->GetWritePtr(PLANAR_Y); //Must be requested

      BYTE* srcpU = (BYTE*)src->GetWritePtr(PLANAR_U);
      BYTE* chromapU = (BYTE*)chroma->GetReadPtr(PLANAR_U);
      BYTE* srcpV = (BYTE*)src->GetWritePtr(PLANAR_V);
      BYTE* chromapV = (BYTE*)chroma->GetReadPtr(PLANAR_V);

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE      
      if ((env->GetCPUFlags() & CPUF_INTEGER_SSE) && (weight>0.4999f) && (weight<0.5001f)) {
        isse_avg_plane(srcpU,chromapU,src->GetPitch(PLANAR_U),chroma->GetPitch(PLANAR_U),src->GetRowSize(PLANAR_U_ALIGNED),src->GetHeight(PLANAR_U));
        isse_avg_plane(srcpV,chromapV,src->GetPitch(PLANAR_U),chroma->GetPitch(PLANAR_U),src->GetRowSize(PLANAR_V_ALIGNED),src->GetHeight(PLANAR_U));
      } else if (TEST(4, 8) (env->GetCPUFlags() & CPUF_MMX)) {
        mmx_weigh_plane(srcpU,chromapU,src->GetPitch(PLANAR_U),chroma->GetPitch(PLANAR_U),src->GetRowSize(PLANAR_U_ALIGNED),src->GetHeight(PLANAR_U),(int)(weight*32767.0f),32767-(int)(weight*32767.0f));
        mmx_weigh_plane(srcpV,chromapV,src->GetPitch(PLANAR_U),chroma->GetPitch(PLANAR_U),src->GetRowSize(PLANAR_V_ALIGNED),src->GetHeight(PLANAR_U),(int)(weight*32767.0f),32767-(int)(weight*32767.0f));
      }
      else 
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE      
      {
        weigh_plane(srcpU,chromapU,src->GetPitch(PLANAR_U),chroma->GetPitch(PLANAR_U),src->GetRowSize(PLANAR_U_ALIGNED),src->GetHeight(PLANAR_U),(int)(weight*65535.0f),65535-(int)(weight*65535.0f));
        weigh_plane(srcpV,chromapV,src->GetPitch(PLANAR_U),chroma->GetPitch(PLANAR_U),src->GetRowSize(PLANAR_V_ALIGNED),src->GetHeight(PLANAR_U),(int)(weight*65535.0f),65535-(int)(weight*65535.0f));
      }
    }
  } else { // weight == 1.0
    if (vi.IsYUY2()) {
      unsigned int* srcp = (unsigned int*)src->GetReadPtr();
      env->MakeWritable(&chroma);
      unsigned int* chromap = (unsigned int*)chroma->GetWritePtr();

      const int isrc_pitch = (src->GetPitch())>>2;  // int pitch (one pitch=two pixels)
      const int ichroma_pitch = (chroma->GetPitch())>>2;  // Ints

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE            
      ((TEST(4, 8) (env->GetCPUFlags() & CPUF_MMX)) ? mmx_merge_luma : merge_luma)(chromap,srcp,ichroma_pitch,isrc_pitch,w,h);  // Just swap luma/chroma
#else
      merge_luma(chromap,srcp,ichroma_pitch,isrc_pitch,w,h);  // Just swap luma/chroma
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
      return chroma;
    } else {
      if (TEST(1, 2) src->IsWritable()) {
        src->GetWritePtr(PLANAR_Y); //Must be requested
        env->BitBlt(src->GetWritePtr(PLANAR_U),src->GetPitch(PLANAR_U),chroma->GetReadPtr(PLANAR_U),chroma->GetPitch(PLANAR_U),chroma->GetRowSize(PLANAR_U),chroma->GetHeight(PLANAR_U));
        env->BitBlt(src->GetWritePtr(PLANAR_V),src->GetPitch(PLANAR_V),chroma->GetReadPtr(PLANAR_V),chroma->GetPitch(PLANAR_V),chroma->GetRowSize(PLANAR_V),chroma->GetHeight(PLANAR_V));
      }
      else { // avoid the cost of 2 chroma blits
        PVideoFrame dst = env->NewVideoFrame(vi);
        
        env->BitBlt(dst->GetWritePtr(PLANAR_Y),dst->GetPitch(PLANAR_Y),src->GetReadPtr(PLANAR_Y),src->GetPitch(PLANAR_Y),src->GetRowSize(PLANAR_Y),src->GetHeight(PLANAR_Y));
        env->BitBlt(dst->GetWritePtr(PLANAR_U),dst->GetPitch(PLANAR_U),chroma->GetReadPtr(PLANAR_U),chroma->GetPitch(PLANAR_U),chroma->GetRowSize(PLANAR_U),chroma->GetHeight(PLANAR_U));
        env->BitBlt(dst->GetWritePtr(PLANAR_V),dst->GetPitch(PLANAR_V),chroma->GetReadPtr(PLANAR_V),chroma->GetPitch(PLANAR_V),chroma->GetRowSize(PLANAR_V),chroma->GetHeight(PLANAR_V));
        return dst;
      }
    }
  }
  return src;
}


AVSValue __cdecl MergeChroma::Create(AVSValue args, void* user_data, IScriptEnvironment* env)
{
  return new MergeChroma(args[0].AsClip(), args[1].AsClip(), args[2].AsFloat(1.0f), TESTARG(3), env);
}


/**************************
******   Merge Luma   *****
**************************/


MergeLuma::MergeLuma(PClip _child, PClip _clip, float _weight, int _test, IScriptEnvironment* env)
  : GenericVideoFilter(_child), clip(_clip), weight(_weight), test(_test)
{
  const VideoInfo& vi2 = clip->GetVideoInfo();

  if (!vi.IsYUV() || !vi2.IsYUV())
    env->ThrowError("MergeLuma: YUV data only (no RGB); use ConvertToYUY2 or ConvertToYV12");

  if (!vi.IsSameColorspace(vi2)) {  // Since this is luma we allow all planar formats to be merged.
    if (vi.IsPlanar() == vi2.IsPlanar()) {
      env->ThrowError("MergeLuma: YUV data is not same type. YUY2 and planar images doesn't mix.");
    }
  }

  if (vi.width!=vi2.width || vi.height!=vi2.height)
    env->ThrowError("MergeLuma: Images must have same width and height!");

  if (weight<0.0f) weight=0.0f;
  if (weight>1.0f) weight=1.0f;
}


PVideoFrame __stdcall MergeLuma::GetFrame(int n, IScriptEnvironment* env)
{
  PVideoFrame src = child->GetFrame(n, env);

  if (weight<0.0001f) return src;

  PVideoFrame luma = clip->GetFrame(n, env);

  if (vi.IsYUY2()) {
    env->MakeWritable(&src);
    unsigned int* srcp = (unsigned int*)src->GetWritePtr();
    unsigned int* lumap = (unsigned int*)luma->GetReadPtr();

    const int isrc_pitch = (src->GetPitch())>>2;  // int pitch (one pitch=two pixels)
    const int iluma_pitch = (luma->GetPitch())>>2;  // Ints

    const int h = src->GetHeight();
    const int w = src->GetRowSize()>>1; // width in pixels

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE          
    if (weight<0.9999f)
      ((TEST(4, 8) (env->GetCPUFlags() & CPUF_MMX)) ? mmx_weigh_luma : weigh_luma)
               (srcp,lumap,isrc_pitch,iluma_pitch,w,h,(int)(weight*32768.0f),32768-(int)(weight*32768.0f));
    else
      ((TEST(4, 8) (env->GetCPUFlags() & CPUF_MMX)) ? mmx_merge_luma : merge_luma)(srcp,lumap,isrc_pitch,iluma_pitch,w,h);
#else
    if (weight<0.9999f)
      weigh_luma(srcp,lumap,isrc_pitch,iluma_pitch,w,h,(int)(weight*32768.0f),32768-(int)(weight*32768.0f));
    else
      merge_luma(srcp,lumap,isrc_pitch,iluma_pitch,w,h);      
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
    return src;
  }  // Planar
  if (weight>0.9999f) {
    const VideoInfo& vi2 = clip->GetVideoInfo();
    if (TEST(1, 2) luma->IsWritable() && vi.IsSameColorspace(vi2)) {
      if (luma->GetRowSize(PLANAR_U)) {
        luma->GetWritePtr(PLANAR_Y); //Must be requested BUT only if we actually do something
        env->BitBlt(luma->GetWritePtr(PLANAR_U),luma->GetPitch(PLANAR_U),src->GetReadPtr(PLANAR_U),src->GetPitch(PLANAR_U),src->GetRowSize(PLANAR_U),src->GetHeight(PLANAR_U));
        env->BitBlt(luma->GetWritePtr(PLANAR_V),luma->GetPitch(PLANAR_V),src->GetReadPtr(PLANAR_V),src->GetPitch(PLANAR_V),src->GetRowSize(PLANAR_V),src->GetHeight(PLANAR_V));
      }
      return luma;
    }
    else { // avoid the cost of 2 chroma blits
      PVideoFrame dst = env->NewVideoFrame(vi);
      
      env->BitBlt(dst->GetWritePtr(PLANAR_Y),dst->GetPitch(PLANAR_Y),luma->GetReadPtr(PLANAR_Y),luma->GetPitch(PLANAR_Y),luma->GetRowSize(PLANAR_Y),luma->GetHeight(PLANAR_Y));
      if (src->GetRowSize(PLANAR_U) && dst->GetRowSize(PLANAR_U)) {
        env->BitBlt(dst->GetWritePtr(PLANAR_U),dst->GetPitch(PLANAR_U),src->GetReadPtr(PLANAR_U),src->GetPitch(PLANAR_U),src->GetRowSize(PLANAR_U),src->GetHeight(PLANAR_U));
        env->BitBlt(dst->GetWritePtr(PLANAR_V),dst->GetPitch(PLANAR_V),src->GetReadPtr(PLANAR_V),src->GetPitch(PLANAR_V),src->GetRowSize(PLANAR_V),src->GetHeight(PLANAR_V));
      }
      return dst;
    }
  } else { // weight <= 0.9999f
    env->MakeWritable(&src);
    BYTE* srcpY = (BYTE*)src->GetWritePtr(PLANAR_Y);
    BYTE* lumapY = (BYTE*)luma->GetReadPtr(PLANAR_Y);

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE                
    if ((TEST(4, 8) (env->GetCPUFlags() & CPUF_INTEGER_SSE)) && (weight>0.4999f) && (weight<0.5001f)) {
      isse_avg_plane(srcpY,lumapY,src->GetPitch(PLANAR_Y),luma->GetPitch(PLANAR_Y),src->GetRowSize(PLANAR_Y_ALIGNED),src->GetHeight(PLANAR_Y));
    }
    else if (TEST(4, 8) (env->GetCPUFlags() & CPUF_MMX)) {
      mmx_weigh_plane(srcpY,lumapY,src->GetPitch(PLANAR_Y),luma->GetPitch(PLANAR_Y),src->GetRowSize(PLANAR_Y_ALIGNED),src->GetHeight(PLANAR_Y),(int)(weight*32767.0f),32767-(int)(weight*32767.0f));
    }
    else 
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
    {
      weigh_plane(srcpY,lumapY,src->GetPitch(PLANAR_Y),luma->GetPitch(PLANAR_Y),src->GetRowSize(PLANAR_Y_ALIGNED),src->GetHeight(PLANAR_Y),(int)(weight*65535.0f),65535-(int)(weight*65535.0f));
    }
  }

  return src;
}


AVSValue __cdecl MergeLuma::Create(AVSValue args, void* user_data, IScriptEnvironment* env)
{
  return new MergeLuma(args[0].AsClip(), args[1].AsClip(), args[2].AsFloat(1.0f), TESTARG(3), env);
}



/*************************
******   Merge All   *****
*************************/


MergeAll::MergeAll(PClip _child, PClip _clip, float _weight, int _test, IScriptEnvironment* env)
  : GenericVideoFilter(_child), clip(_clip), weight(_weight), test(_test)
{
  const VideoInfo& vi2 = clip->GetVideoInfo();

  if (!vi.IsSameColorspace(vi2))
    env->ThrowError("Merge: Pixel types are not the same. Both must be the same.");

  if (vi.width!=vi2.width || vi.height!=vi2.height)
    env->ThrowError("Merge: Images must have same width and height!");

  if (weight<0.0f) weight=0.0f;
  if (weight>1.0f) weight=1.0f;
}


PVideoFrame __stdcall MergeAll::GetFrame(int n, IScriptEnvironment* env)
{
  if (weight<0.0001f) return child->GetFrame(n, env);
  if (weight>0.9999f) return clip->GetFrame(n, env);

  PVideoFrame src  = child->GetFrame(n, env);
  PVideoFrame src2 =  clip->GetFrame(n, env);

  env->MakeWritable(&src);
  BYTE* srcp  = (BYTE*)src->GetWritePtr();
  BYTE* srcp2 = (BYTE*)src2->GetReadPtr();

  const int src_pitch = src->GetPitch();
  const int src_rowsize = src->GetRowSize();
  int src_rowsize4 = (src_rowsize + 3) & -4;
  if (src_rowsize4 > src_pitch) src_rowsize4 = src_pitch;
  int src_rowsize8 = (src_rowsize + 7) & -8;
  if (src_rowsize8 > src_pitch) src_rowsize8 = src_pitch;

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE                  
  if (TEST(16, 32) (env->GetCPUFlags() & CPUF_INTEGER_SSE) && ((src_rowsize4 & 3)==0) && (weight>0.4999f) && (weight<0.5001f)) {
    isse_avg_plane(srcp, srcp2, src_pitch, src2->GetPitch(), src_rowsize4, src->GetHeight());
  }
  else if (TEST(4, 8) (env->GetCPUFlags() & CPUF_MMX) && ((src_rowsize8 & 7)==0)) {
	const int iweight = (int)(weight*32767.0f);
	const int invweight = 32767-iweight;
    mmx_weigh_plane(srcp, srcp2, src_pitch, src2->GetPitch(), src_rowsize8, src->GetHeight(), iweight, invweight);
  }
  else 
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE   
  {
	const int iweight = (int)(weight*65535.0f);
	const int invweight = 65535-iweight;
    weigh_plane(srcp, srcp2, src_pitch, src2->GetPitch(), src_rowsize, src->GetHeight(), iweight, invweight);
  }

  if (vi.IsPlanar()) {
    BYTE* srcpU  = (BYTE*)src->GetWritePtr(PLANAR_U);
    BYTE* srcpV  = (BYTE*)src->GetWritePtr(PLANAR_V);
    BYTE* srcp2U = (BYTE*)src2->GetReadPtr(PLANAR_U);
    BYTE* srcp2V = (BYTE*)src2->GetReadPtr(PLANAR_V);
 
    const int src_pitch = src->GetPitch(PLANAR_U);
    const int src_rowsize = src->GetRowSize(PLANAR_U);
    src_rowsize4 = (src_rowsize + 3) & -4;
    if (src_rowsize4 > src_pitch) src_rowsize4 = src_pitch;
    src_rowsize8 = (src_rowsize + 7) & -8;
    if (src_rowsize8 > src_pitch) src_rowsize8 = src_pitch;
 
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE                    
    if ((TEST(4, 8) (env->GetCPUFlags() & CPUF_INTEGER_SSE) && ((src_rowsize4 & 3)==0)) && (weight>0.4999f) && (weight<0.5001f)) {
      isse_avg_plane(srcpV, srcp2V, src_pitch, src2->GetPitch(PLANAR_U), src_rowsize4, src->GetHeight(PLANAR_U));
      isse_avg_plane(srcpU, srcp2U, src_pitch, src2->GetPitch(PLANAR_V), src_rowsize4, src->GetHeight(PLANAR_V));
    }
    else if (TEST(4, 8) (env->GetCPUFlags() & CPUF_MMX) && ((src_rowsize8 & 7)==0)) {
      const int iweight = (int)(weight*32767.0f);
      const int invweight = 32767-iweight;
      mmx_weigh_plane(srcpV, srcp2V, src_pitch, src2->GetPitch(PLANAR_U), src_rowsize8, src->GetHeight(PLANAR_U), iweight, invweight);
      mmx_weigh_plane(srcpU, srcp2U, src_pitch, src2->GetPitch(PLANAR_V), src_rowsize8, src->GetHeight(PLANAR_V), iweight, invweight);
    }
    else 
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE   
    {
	  const int iweight = (int)(weight*65535.0f);
	  const int invweight = 65535-iweight;
      weigh_plane(srcpV, srcp2V, src_pitch, src2->GetPitch(PLANAR_U), src_rowsize, src->GetHeight(PLANAR_U), iweight, invweight);
      weigh_plane(srcpU, srcp2U, src_pitch, src2->GetPitch(PLANAR_V), src_rowsize, src->GetHeight(PLANAR_V), iweight, invweight);
    }
  }

  return src;
}


AVSValue __cdecl MergeAll::Create(AVSValue args, void* user_data, IScriptEnvironment* env)
{
  return new MergeAll(args[0].AsClip(), args[1].AsClip(), args[2].AsFloat(0.5f), TESTARG(3), env);
}





/****************************
******    C routines    *****
****************************/


void merge_luma(unsigned int *src, unsigned int *luma, int pitch, int luma_pitch,int width, int height ) {

  int lwidth=width>>1;

  for (int y=0;y<height;y++) {
    unsigned char *lum=(unsigned char*)luma;
    unsigned char *dst=(unsigned char*)src;
    for (int x=0;x<lwidth;x++) {
      dst[x*4]   = lum[x*4];
      dst[x*4+2] = lum[x*4+2];
    }
    src+=pitch;
    luma+=luma_pitch;
  } // end for y
}


void weigh_luma(unsigned int *src,unsigned int *luma, int pitch, int luma_pitch,int width, int height, int weight, int invweight) {

  int lwidth=width>>1;

  for (int y=0;y<height;y++) {
    unsigned char *lum=(unsigned char*)luma;
    unsigned char *dst=(unsigned char*)src;
    for (int x=0;x<lwidth;x++) {
      dst[x*4]   = (lum[x*4]   * weight + dst[x*4]   * invweight + 16384) >> 15;
      dst[x*4+2] = (lum[x*4+2] * weight + dst[x*4+2] * invweight + 16384) >> 15;
    }
    src+=pitch;
    luma+=luma_pitch;
  } // end for y
}


void weigh_chroma(unsigned int *src,unsigned int *chroma, int pitch, int chroma_pitch,int width, int height, int weight, int invweight) {

  int lwidth=width>>1;

  for (int y=0;y<height;y++) {
    unsigned char *chm=(unsigned char*)chroma;
    unsigned char *dst=(unsigned char*)src;
    for (int x=0;x<lwidth;x++) {
      dst[x*4+1] = (chm[x*4+1] * weight + dst[x*4+1] * invweight + 16384) >> 15;
      dst[x*4+3] = (chm[x*4+3] * weight + dst[x*4+3] * invweight + 16384) >> 15;
    }
    src+=pitch;
    chroma+=chroma_pitch;
  } // end for y
}


void weigh_plane(BYTE *p1, const BYTE *p2, int p1_pitch, int p2_pitch,int rowsize, int height, int weight, int invweight) {

  for (int y=0;y<height;y++) {
    for (int x=0;x<rowsize;x++) {
      p1[x] = (p1[x]*invweight + p2[x]*weight + 32768) >> 16;
    }
    p2+=p2_pitch;
    p1+=p1_pitch;
  }
}


#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
/****************************
******   MMX routines   *****
****************************/

void mmx_merge_luma( unsigned int *src, unsigned int *luma, int pitch,
                     int luma_pitch,int width, int height )
{
  // [V][Y2][U][Y1]

  int row_size = width * 2;
  int row_even = row_size & -8;
  int lwidth_bytes = row_size & -16;	// bytes handled by the MMX loop

  __asm {
    movq mm7,[I1]     ; Luma
    movq mm6,[I2]     ; Chroma
  }
  for (int y=0;y<height;y++) {

    // eax=src
    // ebx=luma
    // ecx=src/luma offset

  __asm {
    push ebx // bloody compiler forgets to save ebx!!
    mov eax,src
    xor ecx,ecx
    mov ebx,luma
    align 16
goloop:
    cmp       ecx,[lwidth_bytes]	; Is eax(i) greater than endx
    jge       outloop		; Jump out of loop if true

    ; Processes 8 pixels at the time
    movq mm0,[eax+ecx]		; chroma 4 pixels
     movq mm1,[eax+ecx+8]  ; chroma next 4 pixels
    pand mm0,mm6
     movq mm2,[ebx+ecx]  ; load luma 4 pixels
    pand mm1,mm6
     movq mm3,[ebx+ecx+8]  ; load luma next 4 pixels
    pand mm2,mm7
     pand mm3,mm7
    por mm0,mm2
     por mm1,mm3
    movq [eax+ecx],mm0
     movq [eax+ecx+8],mm1
    add ecx,16   // 16 bytes per pass = 8 pixels = 2 quadwords
     jmp goloop
outloop:
    ; processes remaining pixels pair
    cmp ecx,[row_even]
    jge outeven

    movq mm0,[eax+ecx]		; chroma 4 pixels
     movq mm2,[ebx+ecx]  ; load luma 4 pixels
    pand mm0,mm6
     pand mm2,mm7
    add ecx,8
     por mm0,mm2
     movq [eax+ecx-8],mm0
outeven:
    ; processes remaining pixel
    cmp ecx,[row_size]
    jge exitloop

    movd mm0,[eax+ecx]		; chroma 2 pixels
     movd mm2,[ebx+ecx]  ; load luma 2 pixels
    pand mm0,mm6
     pand mm2,mm7
     por mm0,mm2
     movd [eax+ecx],mm0
exitloop:
    pop ebx
    }

    src += pitch;
    luma += luma_pitch;
  } // end for y
  __asm {emms};
}




void mmx_weigh_luma(unsigned int *src,unsigned int *luma, int pitch,
                    int luma_pitch,int width, int height, int weight, int invweight)
{
  int row_size = width * 2;
  int lwidth_bytes = row_size & -8;	// bytes handled by the main loop
  // weight LLLL llll LLLL llll

  __asm {
		movq mm7,[I1]     ; Luma
		movq mm6,[I2]     ; Chroma
		movd mm5,[invweight]
		punpcklwd mm5,[weight]
		punpckldq mm5,mm5 ; Weight = invweight | (weight<<16) | (invweight<<32) | (weight<<48);
		movq mm4,[rounder]

  }
	for (int y=0;y<height;y++) {

		// eax=src
		// ebx=luma
		// ecx=src/luma offset

	__asm {
		push ebx // bloody compiler forgets to save ebx!!
		mov eax,src
		xor ecx,ecx
		mov ebx,luma
		cmp       ecx,[lwidth_bytes]	; Is eax(i) greater than endx
		jge       outloop		; Jump out of loop if true
		movq mm3,[eax+ecx]		; original 4 pixels   (cc)
		align 16
goloop:
		; Processes 4 pixels at the time
		 movq mm2,[ebx+ecx]    ; load 4 pixels
		movq mm1,mm3          ; move original pixel into mm3
		 punpckhwd mm3,mm2     ; Interleave upper pixels in mm3 | mm3= CCLL ccll CCLL ccll
		movq mm0,mm1
		 punpcklwd mm1,mm2     ; Interleave lower pixels in mm1 | mm1= CCLL ccll CCLL ccll
		pand mm3,mm7					; mm3= 00LL 00ll 00LL 00ll
		 pand mm1,mm7
		pmaddwd mm3,mm5				; Mult with weights and add. Latency 2 cycles - mult unit cannot be used
		 pand mm0,mm6					; mm0= cc00 cc00 cc00 cc00
		pmaddwd mm1,mm5
		 paddd mm3,mm4					; round to nearest
		paddd mm1,mm4					; round to nearest
		 psrld mm3,15					; Divide with total weight (=15bits) mm3 = 0000 00LL 0000 00LL
		psrld mm1,15					; Divide with total weight (=15bits) mm1 = 0000 00LL 0000 00LL
		 add ecx,8   // 8 bytes per pass = 4 pixels = 1 quadword
		packssdw mm1, mm3			; mm1 = 00LL 00LL 00LL 00LL
		 cmp ecx,[lwidth_bytes]	; Is eax(i) greater than endx
		por mm1,mm0
		 movq mm3,[eax+ecx]		; original 4 pixels   (cc)
		movq [eax+ecx-8],mm1
		 jnge goloop						; fall out of loop if true

outloop:
		// processes remaining pixels here
		cmp ecx,[row_size]
		jge	exitloop
		movd mm1,[eax+ecx]			; original 2 pixels
		 movd mm2,[ebx+ecx]			; luma 2 pixels
		movq mm0,mm1
		 punpcklwd mm1,mm2				; mm1= CCLL ccll CCLL ccll
		pand mm0,mm6						; mm0= 0000 0000 cc00 cc00
		 pand mm1,mm7						; mm1= 00LL 00ll 00LL 00ll
		 pmaddwd mm1,mm5
		 paddd mm1,mm4						; round to nearest
		 psrld mm1,15						; mm1= 0000 00LL 0000 00LL
		 packssdw mm1,mm1				; mm1= 00LL 00LL 00LL 00LL
		 por mm1,mm0							; mm0 finished
		 movd [eax+ecx],mm1
		// no loop since there is at most 2 remaining pixels
exitloop:
		pop ebx
		}
		src += pitch;
		luma += luma_pitch;
	} // end for y
  __asm {emms};
}




void mmx_weigh_chroma( unsigned int *src,unsigned int *chroma, int pitch,
                     int chroma_pitch,int width, int height, int weight, int invweight )
{

  int row_size = width * 2;
  int lwidth_bytes = row_size & -8;	// bytes handled by the main loop

  __asm {
		movq mm7,[I1]     ; Luma
		movd mm5,[invweight]
		punpcklwd mm5,[weight]
		punpckldq mm5,mm5 ; Weight = invweight | (weight<<16) | (invweight<<32) | (weight<<48);
		movq mm4,[rounder]

  }
	for (int y=0;y<height;y++) {

		// eax=src
		// ebx=luma
		// ecx=src/luma offset

	__asm {
		push ebx // bloody compiler forgets to save ebx!!
		mov eax,src
		xor ecx,ecx
		mov ebx,chroma
		cmp ecx,[lwidth_bytes]	; Is eax(i) greater than endx
		jge       outloop		; Jump out of loop if true
		; Processes 4 pixels at the time
		movq mm1,[eax+ecx]		; original 4 pixels   (cc)
		 movq mm2,[ebx+ecx]    ; load 4 pixels
		align 16
goloop:
		movq mm3,mm1
		 punpcklwd mm1,mm2     ; Interleave lower pixels in mm1 | mm1= CCLL ccll CCLL ccll
		movq mm0,mm3          ; move original pixel into mm3
		 psrlw mm1,8
		punpckhwd mm3,mm2     ; Interleave upper pixels in mm3 | mm3= CCLL ccll CCLL ccll
		 pmaddwd mm1,mm5
		psrlw mm3,8						; mm3= 00CC 00cc 00CC 00cc
		 paddd mm1,mm4					; round to nearest
		pmaddwd mm3,mm5				; Mult with weights and add. Latency 2 cycles - mult unit cannot be used
		 psrld mm1,15					; Divide with total weight (=15bits) mm1 = 0000 00CC 0000 00CC
		paddd mm3,mm4					; round to nearest
		 pand mm0,mm7					; mm0= 00ll 00ll 00ll 00ll
		psrld mm3,15					; Divide with total weight (=15bits) mm3 = 0000 00CC 0000 00CC
		 add ecx,8   // 8 bytes per pass = 4 pixels = 1 quadword
		packssdw mm1, mm3			; mm1 = 00CC 00CC 00CC 00CC
		 cmp ecx,[lwidth_bytes]	; Is eax(i) greater than endx
		psllw mm1,8
		 movq mm2,[ebx+ecx]    ; load 4 pixels
		por mm0,mm1
		 movq mm1,[eax+ecx]		; original 4 pixels   (cc)
		movq [eax+ecx-8],mm0
		 jnge goloop      ; fall out of loop if true

outloop:
		// processes remaining pixels here
		cmp ecx,[row_size]
		jge	exitloop
		movd mm0,[eax+ecx]			; original 2 pixels
		movd mm2,[ebx+ecx]			; luma 2 pixels
		movq mm1,mm0
		punpcklwd mm1,mm2				; mm1= CCLL ccll CCLL ccll
		psrlw mm1,8							; mm1= 00CC 00cc 00CC 00cc
		pmaddwd mm1,mm5
		pand mm0,mm7						; mm0= 0000 0000 00ll 00ll
		paddd mm1,mm4						; round to nearest
		psrld mm1,15						; mm1= 0000 00CC 0000 00CC
		packssdw mm1,mm1
		psllw mm1,8							; mm1= CC00 CC00 CC00 CC00
		por mm0,mm1							; mm0 finished
		movd [eax+ecx],mm0
		// no loop since there is at most 2 remaining pixels
exitloop:
		pop ebx
		}
		src += pitch;
		chroma += chroma_pitch;
	} // end for y
  __asm {emms};
}

/*******************
 * Blends two planes.
 * A weight between the two planes are given.
 * Has excelent pairing,
 * and has very little memory usage.
 * Processes eight pixels per loop, so rowsize must be mod 8.
 * Thanks to ARDA for squeezing out a bit more performance.
 *
 * Weights must be multipled by 32767
 * Returns the blended plane in p1;
 * (c) 2002, 2004 by sh0dan, IanB.
 ********/

void mmx_weigh_plane(BYTE *p1, const BYTE *p2, int p1_pitch, int p2_pitch,int rowsize, int height, int weight, int invweight) {

// mm0 mm1 mm2 mm3 mm4 mm5 mm6 mm7
  __asm {
      push ebx // bloody compiler forgets to save ebx!!
      movq       mm5,[rounder]
      pxor       mm6,mm6
      movd       mm7,[weight]
      punpcklwd  mm7,[invweight]
      punpckldq  mm7,mm7 ; Weight = weight | (invweight<<16) | (weight<<32) | (invweight<<48);
      mov        ebx,[rowsize]
      mov        esi,[p1]
      mov        edi,[p2]
      xor        ecx, ecx  // Height
      mov        edx,[height]
      test       ebx, ebx
      jz         outy
  
      align      16
yloopback:
      xor        eax, eax
      cmp        ecx, edx
      jge        outy

      align 16
testloop:
      movq        mm0,[edi+eax]  // y7y6 y5y4 y3y2 y1y0 img2
       movq       mm1,[esi+eax]  // Y7Y6 Y5Y4 Y3Y2 Y1Y0 IMG1
      movq        mm2,mm0
       punpcklbw  mm0,mm1        // Y3y3 Y2y2 Y1y1 Y0y0
      punpckhbw   mm2,mm1        // Y7y7 Y6y6 Y5y5 Y4y4
       movq       mm1,mm0        // Y3y3 Y2y2 Y1y1 Y0y0
      punpcklbw   mm0,mm6        // 00Y1 00y1 00Y0 00y0
       movq       mm3,mm2        // Y7y7 Y6y6 Y5y5 Y4y4
      pmaddwd     mm0,mm7        // Multiply pixels by weights.  pixel = img1*weight + img2*invweight (twice)
       punpckhbw  mm1,mm6        // 00Y3 00y3 00Y2 00y2
      paddd       mm0,mm5        // Add rounder
       pmaddwd    mm1,mm7        // Multiply pixels by weights.  pixel = img1*weight + img2*invweight (twice)
      punpcklbw   mm2,mm6        // 00Y5 00y5 00Y4 00y4
       paddd      mm1,mm5        // Add rounder                         
      psrld       mm0,15         // Shift down, so there is no fraction.
       pmaddwd    mm2,mm7        // Multiply pixels by weights.  pixel = img1*weight + img2*invweight (twice)
      punpckhbw   mm3,mm6        // 00Y7 00y7 00Y6 00y6
       paddd      mm2,mm5        // Add rounder
      pmaddwd     mm3,mm7        // Multiply pixels by weights.  pixel = img1*weight + img2*invweight (twice)
       psrld      mm1,15         // Shift down, so there is no fraction.
      paddd       mm3,mm5        // Add rounder                         
       psrld      mm2,15         // Shift down, so there is no fraction.
      psrld       mm3,15         // Shift down, so there is no fraction.
       packssdw   mm0,mm1        // 00Y3 00Y2 00Y1 00Y0
      packssdw    mm2,mm3        // 00Y7 00Y6 00Y5 00Y4
       add        eax,8
      packuswb    mm0,mm2        // Y7Y6 Y5Y4 Y3Y2 Y1Y0
       cmp        ebx, eax
      movq        [esi+eax-8],mm0
       jg         testloop

      inc         ecx
       add        esi,[p1_pitch];
      add         edi,[p2_pitch];
       jmp        yloopback
outy:
      emms
      pop ebx
  } // end asm
}


/*******************
 * Average two planes.
 * Processes 16 pixels per loop, rowsize must be mod 4.
 *
 * Returns the blended plane in p1;
 * (c) 2005 by IanB.
 ********/

void isse_avg_plane(BYTE *p1, const BYTE *p2, int p1_pitch, int p2_pitch,int rowsize, int height) {

// mm0 mm1
  __asm {
      push ebx // bloody compiler forgets to save ebx!!
      mov        ebx,[rowsize]
      mov        esi,[p1]
      mov        edi,[p2]
      xor        ecx, ecx  // Height
      mov        edx,[height]
      test       ebx, ebx
      jz         outy
  
      align      16
yloopback:
      mov        eax, 16
      cmp        ecx, edx
      jge        outy

      cmp        ebx, eax
      jl         twelve
      align 16
testloop:
      movq        mm0,[edi+eax-16]  // y7y6 y5y4 y3y2 y1y0 img2
       movq       mm1,[edi+eax- 8]  // yFyE yDyC yByA y9y8 img2
      pavgb       mm0,[esi+eax-16]  // Y7Y6 Y5Y4 Y3Y2 Y1Y0 IMG1
       pavgb      mm1,[esi+eax- 8]  // YfYe YdYc YbYa Y9Y8 IMG1
      movq        [esi+eax-16],mm0
       movq       [esi+eax- 8],mm1
      add         eax,16
      cmp         ebx, eax
      jge         testloop
      align 16
twelve:
	  test        ebx, 8
	  jz          four
      movq        mm0,[edi+eax-16]  // y7y6 y5y4 y3y2 y1y0 img2
      pavgb       mm0,[esi+eax-16]  // Y7Y6 Y5Y4 Y3Y2 Y1Y0 IMG1
      movq        [esi+eax-16],mm0
      add         eax,8
      align 16
four:
	  test        ebx, 4
	  jz          zero
      movd        mm0,[edi+eax-16]  // ____ ____ y3y2 y1y0 img2
      movd        mm1,[esi+eax-16]  // ____ ____ Y3Y2 Y1Y0 IMG1
      pavgb       mm0,mm1
      movd        [esi+eax-16],mm0
      align 16
zero:
      inc         ecx
      add         esi,[p1_pitch];
      add         edi,[p2_pitch];
      jmp         yloopback
outy:
      emms
      pop ebx
  } // end asm
}

#endif  // ENABLE_INLINE_ASSEMBLY_MMX_SSE

}; // namespace avxsynth

