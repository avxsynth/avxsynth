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

#include "resize.h"


namespace avxsynth {


/********************************************************************
***** Declare index of new filters for Avisynth's filter engine *****
********************************************************************/

AVSFunction Resize_filters[] = {
  { "VerticalReduceBy2", "c", VerticalReduceBy2::Create },        // src clip
  { "HorizontalReduceBy2", "c", HorizontalReduceBy2::Create },    // src clip
  { "ReduceBy2", "c", Create_ReduceBy2 },                         // src clip
  { 0 }
};





/*************************************
 ******* Vertical 2:1 Reduction ******
 ************************************/


VerticalReduceBy2::VerticalReduceBy2(PClip _child, IScriptEnvironment* env)
 : GenericVideoFilter(_child)
{
  original_height = vi.height;
  vi.height >>= 1;
  if (vi.height<3) {
    env->ThrowError("VerticalReduceBy2: Image too small to be reduced by 2.");    
  }
}


PVideoFrame VerticalReduceBy2::GetFrame(int n, IScriptEnvironment* env) {
  PVideoFrame src = child->GetFrame(n, env);
  PVideoFrame dst = env->NewVideoFrame(vi);
  int src_pitch = src->GetPitch();
  int dst_pitch = dst->GetPitch();
  int row_size = src->GetRowSize();
  BYTE* dstp = dst->GetWritePtr();
  const BYTE* srcp = src->GetReadPtr();

  if (vi.IsPlanar()) {
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE        
    if ((env->GetCPUFlags() & CPUF_MMX) && (row_size&3)==0) 
        mmx_process(srcp,src_pitch, row_size, dstp,dst_pitch,dst->GetHeight(PLANAR_Y));    
    else
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
        c_process(srcp, src_pitch, src->GetHeight(PLANAR_Y), row_size, dstp, dst_pitch, dst->GetHeight(PLANAR_Y));   
    
    if (src->GetRowSize(PLANAR_V)) {
      src_pitch = src->GetPitch(PLANAR_V);
      dst_pitch = dst->GetPitch(PLANAR_V);
      row_size = src->GetRowSize(PLANAR_V_ALIGNED);
      dstp = dst->GetWritePtr(PLANAR_V);
      srcp = src->GetReadPtr(PLANAR_V);
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE   
      if ((env->GetCPUFlags() & CPUF_MMX) && (row_size&3)==0)
        mmx_process(srcp,src_pitch, row_size, dstp,dst_pitch,dst->GetHeight(PLANAR_V));
      else
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
        c_process(srcp, src_pitch, src->GetHeight(PLANAR_V), row_size, dstp, dst_pitch, dst->GetHeight(PLANAR_V));   
      
      src_pitch = src->GetPitch(PLANAR_U);
      dst_pitch = dst->GetPitch(PLANAR_U);
      row_size = src->GetRowSize(PLANAR_U_ALIGNED);
      dstp = dst->GetWritePtr(PLANAR_U);
      srcp = src->GetReadPtr(PLANAR_U);
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE    
      if ((env->GetCPUFlags() & CPUF_MMX) && (row_size&3)==0)
        mmx_process(srcp,src_pitch, row_size, dstp,dst_pitch,dst->GetHeight(PLANAR_U));
      else
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
        c_process(srcp, src_pitch, src->GetHeight(PLANAR_U), row_size, dstp, dst_pitch, dst->GetHeight(PLANAR_U));
    }
    return dst;

  }

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE  
  if ((env->GetCPUFlags() & CPUF_MMX)) {
    if ((row_size&3)==0) {  // row width divideable with 4 (one dword per loop)
      mmx_process(srcp,src_pitch, row_size, dstp,dst_pitch,vi.height);
      return dst;
    }
  }
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE  
    
    
  c_process(src->GetReadPtr(), src_pitch, original_height, row_size, dstp, dst_pitch, vi.height);

  return dst;
}


void VerticalReduceBy2::c_process(const BYTE* srcp, int src_pitch, int src_height, int row_size, BYTE* dstp, int dst_pitch, int height) 

{    
  for (int y=0; y<height; ++y) {
    const BYTE* line0 = srcp + (y*2)*src_pitch;
    const BYTE* line1 = line0 + src_pitch;
    const BYTE* line2 = (y*2 < src_height-2) ? (line1 + src_pitch) : line0;
    for (int x=0; x<row_size; ++x)
      dstp[x] = (line0[x] + 2*line1[x] + line2[x] + 2) >> 2;
    dstp += dst_pitch;
  }    
    
    
}

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE  
/*************************************
 ******* Vertical 2:1 Reduction ******
 ******* MMX Optimized          ******
 ******* by Klaus Post          ******
 ************************************/


#define R_SRC edx
#define R_DST edi
#define R_XOFFSET eax
#define R_YLEFT ebx
#define R_SRC_PITCH ecx
#define R_DST_PITCH esi

void VerticalReduceBy2::mmx_process(const BYTE* srcp, int src_pitch, int row_size, BYTE* dstp, int dst_pitch, int height) {
  height--;
  static const __int64 add_2=0x0002000200020002;

  if ((row_size&3)==0) {  // row width divideable with 4 (one dword per loop)
    __asm {
        push ebx  // avoid compiler bug
        movq mm7,[add_2];
        add [srcp],-4
        mov R_XOFFSET,0
        mov R_SRC,srcp
        mov R_DST,dstp
        mov R_SRC_PITCH,[src_pitch]
        mov R_DST_PITCH,[dst_pitch]
        mov R_YLEFT,[height]
loopback:
      pxor mm1,mm1
        punpckhbw mm0,[R_SRC]  // line0
        punpckhbw mm1,[R_SRC+R_SRC_PITCH]  // line1
        punpckhbw mm2,[R_SRC+R_SRC_PITCH*2]  // line2
        psrlw mm0,8
        psrlw mm1,7
        paddw mm0,mm7
        psrlw mm2,8
        paddw mm0,mm1
        paddw mm0,mm2
        psrlw mm0,2
        packuswb mm0,mm1
        movd [R_DST+R_XOFFSET],mm0
        add R_SRC,4
        add R_XOFFSET,4
        cmp  R_XOFFSET,[row_size]
        jl loopback						; Jump back
        add srcp, R_SRC_PITCH
        mov R_XOFFSET,0
        add srcp, R_SRC_PITCH
        add R_DST,R_DST_PITCH
        mov R_SRC,srcp
        dec R_YLEFT
        jnz loopback
        
        // last line 
loopback_last:
      pxor mm1,mm1
        punpckhbw mm0,[R_SRC]  // line0
        punpckhbw mm1,[R_SRC+R_SRC_PITCH]  // line1
        psrlw mm0,8
        movq mm2,mm1  // dupe line 1
        psrlw mm1,7
        paddw mm0,mm7
        psrlw mm2,8
        paddw mm0,mm1
        paddw mm0,mm2
        psrlw mm0,2
        packuswb mm0,mm1
        movd [R_DST+R_XOFFSET],mm0
        add R_XOFFSET,4
        add R_SRC,4
        cmp  R_XOFFSET,[row_size]
        jl loopback_last						; Jump back
        emms
        pop ebx
    }
  }
}

#undef R_SRC
#undef R_DST
#undef R_XOFFSET
#undef R_YLEFT
#undef R_SRC_PITCH
#undef R_DST_PITCH


#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE


/************************************
 **** Horizontal 2:1 Reduction ******
 ***********************************/

HorizontalReduceBy2::HorizontalReduceBy2(PClip _child, IScriptEnvironment* env)
: GenericVideoFilter(_child), mybuffer(0)
{
  if (vi.IsYUY2() && (vi.width & 3))
    env->ThrowError("HorizontalReduceBy2: YUY2 image width must be even");
  source_width = vi.width;
  vi.width >>= 1;
}
 

PVideoFrame HorizontalReduceBy2::GetFrame(int n, IScriptEnvironment* env) 
{
  PVideoFrame src = child->GetFrame(n, env);
  PVideoFrame dst = env->NewVideoFrame(vi);
  int src_gap = src->GetPitch() - src->GetRowSize();  //aka 'modulo' in VDub filter terminology
  int dst_gap = dst->GetPitch() - dst->GetRowSize();
  const int dst_pitch = dst->GetPitch();

  BYTE* dstp = dst->GetWritePtr();

  if (vi.IsPlanar()) {
    const BYTE* srcp = src->GetReadPtr(PLANAR_Y);
    int yloops=dst->GetHeight(PLANAR_Y);
    int xloops=dst->GetRowSize(PLANAR_Y)-1;
    for (int y = 0; y<yloops; y++) {
      for (int x = 0; x<xloops; x++) {
        dstp[0] = (srcp[0] + 2*srcp[1] + srcp[2] + 2) >> 2;
        dstp ++;
        srcp +=2;
      }      
      dstp[0] = (srcp[0] + srcp[1] +1 ) >> 1;
      dstp += dst_gap+1;
      srcp += src_gap+2;
    }
    srcp = src->GetReadPtr(PLANAR_U);
    dstp = dst->GetWritePtr(PLANAR_U);
    src_gap = src->GetPitch(PLANAR_U) - src->GetRowSize(PLANAR_U);
    dst_gap = dst->GetPitch(PLANAR_U) - dst->GetRowSize(PLANAR_U);
    yloops=dst->GetHeight(PLANAR_U);
    xloops=dst->GetRowSize(PLANAR_U)-1;
    for (int y = 0; y<yloops; y++) {
      for (int x = 0; x<xloops; x++) {
        dstp[0] = (srcp[0] + 2*srcp[1] + srcp[2] + 2) >> 2;
        dstp ++;
        srcp +=2;
      }
      dstp[0] = (srcp[0] + srcp[1] +1 ) >> 1;
      dstp += dst_gap+1;
      srcp += src_gap+2;
    }
    srcp = src->GetReadPtr(PLANAR_V);
    dstp = dst->GetWritePtr(PLANAR_V);
    for (int y = 0; y<yloops; y++) {
      for (int x = 0; x<xloops; x++) {
        dstp[0] = (srcp[0] + 2*srcp[1] + srcp[2] + 2) >> 2;
        dstp ++;
        srcp +=2;
      }
      dstp[0] = (srcp[0] + srcp[1] +1 ) >> 1;
      dstp += dst_gap+1;
      srcp += src_gap+2;
    }
  } else if (vi.IsYUY2()  && (!(vi.width&3))) {
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE              
    if (env->GetCPUFlags() & CPUF_INTEGER_SSE) {
			isse_process_yuy2(src,dstp,dst_pitch);
			return dst;
	}
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
    const BYTE* srcp = src->GetReadPtr();
    for (int y = vi.height; y>0; --y) {
      for (int x = (vi.width>>1)-1; x; --x) {
        dstp[0] = (srcp[0] + 2*srcp[2] + srcp[4] + 2) >> 2;
        dstp[1] = (srcp[1] + 2*srcp[5] + srcp[9] + 2) >> 2;
        dstp[2] = (srcp[4] + 2*srcp[6] + srcp[8] + 2) >> 2;
        dstp[3] = (srcp[3] + 2*srcp[7] + srcp[11] + 2) >> 2;
        dstp += 4;
        srcp += 8;
      }
      dstp[0] = (srcp[0] + 2*srcp[2] + srcp[4] + 2) >> 2;
      dstp[1] = (srcp[1] + srcp[5] + 1) >> 1;
      dstp[2] = (srcp[4] + srcp[6] + 1) >> 1;
      dstp[3] = (srcp[3] + srcp[7] + 1) >> 1;
      dstp += dst_gap+4;
      srcp += src_gap+8;

    }
  } else if (vi.IsRGB24()) {
    const BYTE* srcp = src->GetReadPtr();
    for (int y = vi.height; y>0; --y) {
      for (int x = (source_width-1)>>1; x; --x) {
        dstp[0] = (srcp[0] + 2*srcp[3] + srcp[6] + 2) >> 2;
        dstp[1] = (srcp[1] + 2*srcp[4] + srcp[7] + 2) >> 2;
        dstp[2] = (srcp[2] + 2*srcp[5] + srcp[8] + 2) >> 2;
        dstp += 3;
        srcp += 6;
      }
      if (source_width&1) {
        dstp += dst_gap;
        srcp += src_gap+3;
      } else {
        dstp[0] = (srcp[0] + srcp[3] + 1) >> 1;
        dstp[1] = (srcp[1] + srcp[4] + 1) >> 1;
        dstp[2] = (srcp[2] + srcp[5] + 1) >> 1;
        dstp += dst_gap+3;
        srcp += src_gap+6;
      }
    }
  } else if (vi.IsRGB32()) {  //rgb32
    const BYTE* srcp = src->GetReadPtr();
    for (int y = vi.height; y>0; --y) {
      for (int x = (source_width-1)>>1; x; --x) {
        dstp[0] = (srcp[0] + 2*srcp[4] + srcp[8] + 2) >> 2;
        dstp[1] = (srcp[1] + 2*srcp[5] + srcp[9] + 2) >> 2;
        dstp[2] = (srcp[2] + 2*srcp[6] + srcp[10] + 2) >> 2;
        dstp[3] = (srcp[3] + 2*srcp[7] + srcp[11] + 2) >> 2;
        dstp += 4;
        srcp += 8;
      }
      if (source_width&1) {
        dstp += dst_gap;
        srcp += src_gap+4;
      } else {
        dstp[0] = (srcp[0] + srcp[4] + 1) >> 1;
        dstp[1] = (srcp[1] + srcp[5] + 1) >> 1;
        dstp[2] = (srcp[2] + srcp[6] + 1) >> 1;
        dstp[3] = (srcp[3] + srcp[7] + 1) >> 1;
        dstp += dst_gap+4;
        srcp += src_gap+8;
      }
    }
  }
  return dst;
}


#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE  
/*
0102 0304 0506 0708
*/


/************************************
 **** Horizontal 2:1 Reduction ******
 **** YUY2 Integer SSE Optimized ****
 **** by Klaus Post              ****
 ***********************************/


#define R_SRC esi
#define R_DST edi
#define R_XOFFSET edx
#define R_TEMP1 eax
#define R_TEMP2 ecx

void HorizontalReduceBy2::isse_process_yuy2(PVideoFrame src,BYTE* dstp, int dst_pitch) {
  
  const BYTE* srcp = src->GetReadPtr();
  const int src_pitch = src->GetPitch();
  int row_size = ((src->GetRowSize())>>1)-4;
	
  const int height = vi.height;
  int yleft = height;

  __declspec(align(8)) static const __int64 add_2=0x0002000200020002;
  __declspec(align(8)) static const __int64 zero_mask=0x000000000000ffff;
  __declspec(align(8)) static const __int64 three_mask=0x0000ffff00000000;
  __declspec(align(8)) static const __int64 inv_0_mask=0xffffffffffff0000;
  __declspec(align(8)) static const __int64 inv_3_mask=0xffff0000ffffffff;

/**
 * The matrix is flipped, so the mmx registers are equivalent to
 *  the downward column, and pixels are then added sideways in parallel.
 * The last two pixels are handled seperately (as in the C-code), 
 *  with native code - but it's only the last pixel.
 **/
  if ((row_size&3)==0) {  // row width divideable with 8 (one qword per loop)
    __asm {
      movq mm6,[zero_mask];
      movq mm7,[inv_0_mask];

      mov R_XOFFSET,0
				add [srcp],-4
				prefetchnta [srcp]
				prefetchnta [srcp+64]
        mov R_SRC,srcp
        mov R_DST,dstp
loop_nextline:
				mov R_TEMP2,[row_size]
loopback:
				prefetchnta [R_SRC+64]
        punpckhbw mm0,[R_SRC]  // pixel 1
        punpckhbw mm1,[R_SRC+4]  // pixel 2
        punpckhbw mm2,[R_SRC+8]  // pixel 3
				psrlw mm0,8				;mm0=00VV 00Y2 00UU 00Y1
				psrlw mm1,8				;mm1=00VV 00Y2 00UU 00Y1
				psrlw mm2,8				;mm2=00VV 00Y2 00UU 00Y1
				movq mm3,mm0								; word 3 [4] word 0 in mm1 (ok)
				movq mm4,mm1								; word 0 [2] must be exchanged with word 2 in mm0
				pand mm3,[inv_3_mask]
				pshufw mm5,mm2,11000100b		; word 0 [4] must be exhanged (word0 in mm1) (ok)
				pand mm4,mm7
				pshufw mm1,mm1,0
				pand mm5,mm7
				movq mm2,mm1
        pand mm1,mm6
				pshufw mm0,mm0,10101010b
        pand mm2,[three_mask]
        pand mm0,mm6
				por mm4,mm0
				por mm3,mm2
				psllw mm4,1
				por mm5,mm1
				paddw mm3,mm4
				paddw mm5,[add_2]
				paddw mm3,mm5
        psrlw mm3,2
        packuswb mm3,mm1
        movd [R_DST+R_XOFFSET],mm3

				add R_SRC,8
        add R_XOFFSET,4
        cmp  R_XOFFSET,R_TEMP2
        jl loopback						; Jump back
				// Last two pixels
				// Could be optimized greatly, but it only runs once per line

			mov R_TEMP1,[R_SRC]
			mov R_TEMP2,[R_SRC+2]
			and R_TEMP1,255
			and R_TEMP2,255
			add R_TEMP1,R_TEMP2
			add R_TEMP1,R_TEMP2
			mov R_TEMP2,[R_SRC+4]
			add R_TEMP1,2
			and R_TEMP2,255
			add R_TEMP1,R_TEMP2
			shr R_TEMP1,2
			mov [R_DST+R_XOFFSET],R_TEMP1

			mov R_TEMP1,[R_SRC+1]
			mov R_TEMP2,[R_SRC+5]
			and R_TEMP1,255
			and R_TEMP2,255
			add R_TEMP1,1
			add R_TEMP1,R_TEMP2
			shr R_TEMP1,1
			shl R_TEMP1,8
			or [R_DST+R_XOFFSET],R_TEMP1

			mov R_TEMP1,[R_SRC+4]
			mov R_TEMP2,[R_SRC+6]
			and R_TEMP1,255
			and R_TEMP2,255
			add R_TEMP1,1
			add R_TEMP1,R_TEMP2
			shr R_TEMP1,1
			shl R_TEMP1,16
			or [R_DST+R_XOFFSET],R_TEMP1

			mov R_TEMP1,[R_SRC+3]
			mov R_TEMP2,[R_SRC+7]
			and R_TEMP1,255
			and R_TEMP2,255
			add R_TEMP1,1
			add R_TEMP1,R_TEMP2
			shr R_TEMP1,1
			shl R_TEMP1,24
			or [R_DST+R_XOFFSET],R_TEMP1

				mov R_TEMP2,[src_pitch]
        add srcp, R_TEMP2
        mov R_XOFFSET,0
        add R_DST,[dst_pitch]
        mov R_SRC,srcp

        mov R_TEMP1, yleft
        dec R_TEMP1
        mov yleft, R_TEMP1
        jnz loop_nextline        
        emms
    }
  }
}

#undef R_SRC
#undef R_DST
#undef R_XOFFSET
#undef R_YLEFT
#undef R_SRC_PITCH
#undef R_DST_PITCH

#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE


/**************************************
 *****  ReduceBy2 Factory Method  *****
 *************************************/
 

AVSValue __cdecl Create_ReduceBy2(AVSValue args, void*, IScriptEnvironment* env) 
{
  return new HorizontalReduceBy2(new VerticalReduceBy2(args[0].AsClip(), env),env);
}

}; // namespace avxsynth

