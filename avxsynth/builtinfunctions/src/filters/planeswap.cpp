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


// Avisynth filter:  Swap planes
// by Klaus Post
// adapted by Richard Berg (avisynth-dev@richardberg.net)
// iSSE code by Ian Brabham


#include "common/include/stdafx.h"

#include "planeswap.h"

namespace avxsynth {


/********************************************************************
***** Declare index of new filters for Avisynth's filter engine *****
********************************************************************/
#if 0  // Set to 1 to expose test harness
AVSFunction Swap_filters[] = {
  {  "SwapUV","c[test]i", Swap::CreateUV },
  {  "UToY","c[test]i", Swap::CreateUToY },
  {  "VToY","c[test]i", Swap::CreateVToY },
  {  "YToUV","cc[test]i", Swap::CreateYToUV },
  {  "YToUV","ccc[test]i", Swap::CreateYToYUV },
  { 0 }
};
#define TEST(off, on) !!(test & on) || !(test & off) &&
#define TESTARG(n) args[n].AsInt(0) 
#else
AVSFunction Swap_filters[] = {
  {  "SwapUV","c", Swap::CreateUV },
  {  "UToY","c", Swap::CreateUToY },
  {  "VToY","c", Swap::CreateVToY },
  {  "YToUV","cc", Swap::CreateYToUV },
  {  "YToUV","ccc", Swap::CreateYToYUV },
  { 0 }
};
#define TEST(off, on)
#define TESTARG(n) 0
#endif


/**************************************
 *  Swap - swaps UV on planar maps
 **************************************/


AVSValue __cdecl Swap::CreateUV(AVSValue args, void* user_data, IScriptEnvironment* env) {
  return new Swap(args[0].AsClip(), args[0].AsClip(), args[0].AsClip(), SwapUV, TESTARG(1), env);
}

AVSValue __cdecl Swap::CreateUToY(AVSValue args, void* user_data, IScriptEnvironment* env) {
  return new Swap(args[0].AsClip(), args[0].AsClip(), args[0].AsClip(), UToY, TESTARG(1), env);
}

AVSValue __cdecl Swap::CreateVToY(AVSValue args, void* user_data, IScriptEnvironment* env) {
  return new Swap(args[0].AsClip(), args[0].AsClip(), args[0].AsClip(), VToY, TESTARG(1), env);
}

AVSValue __cdecl Swap::CreateYToUV(AVSValue args, void* user_data, IScriptEnvironment* env) {
  return new Swap(args[0].AsClip(), args[1].AsClip(), NULL , YToUV, TESTARG(2), env);
}

AVSValue __cdecl Swap::CreateYToYUV(AVSValue args, void* user_data, IScriptEnvironment* env) {
  return new Swap(args[0].AsClip(), args[1].AsClip(), args[2].AsClip(), YToUV, TESTARG(3), env);
}

Swap::Swap(PClip _child, PClip _clip, PClip _clipY, int _mode, int _test, IScriptEnvironment* env)
  : GenericVideoFilter(_child), clip(_clip), clipY(_clipY), mode(_mode), test(_test) {
    VideoInfo vi2=clip->GetVideoInfo();
    VideoInfo vi3;

  const char *myname;

  switch (mode) {
  case SwapUV: myname="SwapUV"; break;
  case UToY:   myname="UToY";   break;
  case VToY:   myname="VToY";   break;
  case YToUV:  myname="YToUV";  break;
  default:     myname="Plane swapper"; break;
  }

  if (clipY)
    vi3=clipY->GetVideoInfo();

  if (!vi.IsYUV() || !vi2.IsYUV())
    env->ThrowError("%s: YUV data only!", myname);

  switch (mode) {
  case SwapUV:
    break;
  case UToY:
  case VToY:
    if (vi.IsYV12()) vi.height >>=1;
    if (vi.IsYUY2() || vi.IsYV12()) vi.width >>=1;
    break;
  case YToUV:
    if (!(vi.IsYV12() == vi2.IsYV12() || vi.IsYUY2() == vi2.IsYUY2())) {
       env->ThrowError("YToUV: Clips must be the same colorspace!");
    }
    if (vi.height!=vi2.height)
      env->ThrowError("YToUV: Clips do not have the same height!");
    if (vi.width!=vi2.width)
      env->ThrowError("YToUV: Clips do not have the same width!");
    if (clipY) {
      if (!(vi3.IsYV12() == vi.IsYV12() || vi.IsYUY2() == vi3.IsYUY2()))
        env->ThrowError("YToUV: Y clip must be be same colorspace as the UV clips!");

      if ((vi3.width/2)!=vi.width)
        env->ThrowError("YToUV: Y clip does not have the double width of the UV clips!");

      if (vi.IsYV12() && (vi3.height/2)!=vi.height)
        env->ThrowError("YToUV: Y clip does not have the double height of the UV clips! (YV12 mode)");

      if (vi.IsYUY2() && vi3.height!=vi.height)
        env->ThrowError("YToUV: Y clip does not have the same height of the UV clips! (YUY2 mode)");
    }

    if (!vi.IsYUY2()) vi.height <<=1;
    vi.width <<=1;
    break;
  }
}

PVideoFrame __stdcall Swap::GetFrame(int n, IScriptEnvironment* env) {
  PVideoFrame src = child->GetFrame(n, env);

  if (mode==SwapUV) {  // SwapUV
    if (vi.IsPlanar()) {
	  // Abuse subframe to flip the UV plane pointers -- extremely fast but a bit naughty!
	  const int uvoffset = src->GetReadPtr(PLANAR_V) - src->GetReadPtr(PLANAR_U); // very naughty - don't do this at home!!

	  return env->SubframePlanar(src,0, src->GetPitch(PLANAR_Y), src->GetRowSize(PLANAR_Y), src->GetHeight(PLANAR_Y),
	                       uvoffset, -uvoffset, src->GetPitch(PLANAR_V));

    } else if (vi.IsYUY2()) { // YUY2
	  if (TEST(1, 2) src->IsWritable()) {
		BYTE* srcp = src->GetWritePtr();
		for (int y=0; y<vi.height; y++) {
		  for (int x = 0; x < src->GetRowSize(); x+=4) {
			const BYTE t = srcp[x+3]; // This is surprisingly fast,
			srcp[x+3] = srcp[x+1];    // faster than any MMX/SSE code
			srcp[x+1] = t;            // I could write.
		  }
		  srcp += src->GetPitch();
		}
		return src;
	  }
	  else { // avoid the cost of a frame blit, we have to parse the frame anyway
		const BYTE* srcp = src->GetReadPtr();
		PVideoFrame dst = env->NewVideoFrame(vi);
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
		if (TEST(4, 8) (env->GetCPUFlags() & CPUF_INTEGER_SSE)) { // need pshufw
		  BYTE* dstp = dst->GetWritePtr();
		  int srcpitch = src->GetPitch();
		  int dstpitch = dst->GetPitch();
		  int height = vi.height;
		  int rowsize4 = dst->GetRowSize();
		  int rowsize8 = rowsize4 & -8;
		  int rowsize16 = rowsize4 & -16;

		  __asm {
		  pcmpeqb   mm7,mm7				; 0xffffffffffffffff
		  psrlw     mm7,8				; 0x00ff00ff00ff00ff

		  mov       ecx,[height]
		  xor       eax,eax
		  test      ecx,ecx
		  mov       edx,[rowsize16]
		  jz        done

		  mov       esi,[srcp]
		  mov       edi,[dstp]
		  align     16
yloop:
		  cmp       eax,edx
		  jge       twoleft

		  align     16					; Process 8 pixels(16 bytes) per loop
xloop:
		  movq      mm0,[esi+eax]		; VYUY HI VYUY LO
		   movq      mm1,[esi+eax+8]	; vyuy hi vyuy lo
		  movq      mm2,mm0
		   punpckhbw mm0,mm1			; vVyY uUyY hi HI
		  movq      mm3,mm7
		   punpcklbw mm2,mm1			; vVyY uUyY lo LO
		  movq      mm1,mm7
		   pshufw    mm0,mm0,01101100b	; uUyY vVyY hi HI
		  add       eax,16
		   pshufw    mm2,mm2,01101100b	; uUyY vVyY lo LO
		  pand      mm1,mm0				; 0U0Y 0V0Y HI
		   psrlw     mm0,8					; 0u0y 0v0y hi
		  pand      mm3,mm2				; 0U0Y 0V0Y LO
		   psrlw     mm2,8					; 0u0y 0v0y lo
		  packuswb  mm3,mm1				; UYVY HI UYVY LO
		   cmp       eax,edx
		  packuswb  mm2,mm0				; uyvy hi uyvy lo
		   movq      [edi+eax-16],mm3
		  movq      [edi+eax-8],mm2
		   jl        xloop
		  align     16
twoleft:
		  cmp       eax,[rowsize8]
		  jge       oneleft

		  movq      mm0,[esi+eax]		; VYUY HI VYUY LO
		   pxor      mm1,mm1
		  movq      mm2,mm0
		   punpckhbw mm0,mm1			; _V_Y _U_Y HI
		  punpcklbw mm2,mm1				; _V_Y _U_Y LO
		   pshufw    mm0,mm0,01101100b	; _U_Y _V_Y HI
		  pshufw    mm2,mm2,01101100b	; _U_Y _V_Y LO
		   add       eax,8
		  packuswb  mm2,mm0				; UYVY HI UYVY LO
		  movq      [edi+eax-8],mm2
		  align     16
oneleft:
		  cmp       eax,[rowsize4]
		  jge       noneleft

		  movd      mm0,[esi+eax]		; ____ HI VYUY LO
		   pxor      mm1,mm1
		   punpcklbw mm0,mm1			; _V_Y _U_Y LO
		   pshufw    mm0,mm0,01101100b	; _U_Y _V_Y LO
		   packuswb  mm0,mm1			; ____ HI UYVY LO
		   movd      [edi+eax],mm0
		  align     16
noneleft:
		  add       esi,[srcpitch]
		   add       edi,[dstpitch]
		  xor       eax,eax
		   dec       ecx
		  jnz       yloop
done:
		  emms
		  }
		}
		else 
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE            
        {
		  short* dstp = (short*)dst->GetWritePtr();
		  const int srcpitch = src->GetPitch();
		  const int dstpitch = dst->GetPitch()>>1;
		  const int endx = dst->GetRowSize()>>1;
		  for (int y=0; y<vi.height; y++) {
			for (int x = 0; x < endx; x+=2) {
			  // The compiler generates very good code for this construct 
			  // using ah, al & ax register variants to very good effect.
			  dstp[x+0] = (srcp[x*2+3] << 8) | srcp[x*2+0];
			  dstp[x+1] = (srcp[x*2+1] << 8) | srcp[x*2+2];
			}
			srcp += srcpitch;
			dstp += dstpitch;
		  }
		}
		return dst;
      }
    }
  }


  if (mode==UToY || mode ==VToY) {  // U to Y or V to Y
    PVideoFrame dst = env->NewVideoFrame(vi);

    if (vi.IsYUY2()) {  // YUY2 interleaved
      const BYTE* srcp = src->GetReadPtr();
      short* dstp = (short*)dst->GetWritePtr();
      const int srcpitch = src->GetPitch();
      const int dstpitch = dst->GetPitch()>>1;
      const int endx = dst->GetRowSize()>>1;
      if (mode==UToY) {
        for (int y=0; y<vi.height; y++) {
          for (int x = 0; x < endx; x+=2) {
            dstp[x  ] = 0x8000 | srcp[x*4+1];
            dstp[x+1] = 0x8000 | srcp[x*4+5];
          }
          srcp += srcpitch;
          dstp += dstpitch;
        }
      }
      else if (mode==VToY) {
        for (int y=0; y<vi.height; y++) {
          for (int x = 0; x < endx; x+=2) {
            dstp[x  ] = 0x8000 | srcp[x*4+3];
            dstp[x+1] = 0x8000 | srcp[x*4+7];
          }
          srcp += srcpitch;
          dstp += dstpitch;
        }
      }
      return dst;
    }

    // Planar

    if (mode==UToY) {  // U To Y
      env->BitBlt(dst->GetWritePtr(PLANAR_Y),dst->GetPitch(PLANAR_Y),src->GetReadPtr(PLANAR_U),src->GetPitch(PLANAR_U),dst->GetRowSize(),dst->GetHeight());
    }
    else if (mode==VToY) {
      env->BitBlt(dst->GetWritePtr(PLANAR_Y),dst->GetPitch(PLANAR_Y),src->GetReadPtr(PLANAR_V),src->GetPitch(PLANAR_V),dst->GetRowSize(),dst->GetHeight());
    }

    // Clear chroma

    const int pitch = dst->GetPitch(PLANAR_U)/4;
    const int myx = dst->GetRowSize(PLANAR_U_ALIGNED)/4;
    const int myy = dst->GetHeight(PLANAR_U);

    int *srcpUV = (int*)dst->GetWritePtr(PLANAR_U);
    for (int y=0; y<myy; y++) {
      for (int x=0; x<myx; x++) {
        srcpUV[x] = 0x80808080;  // mod 8
      }
      srcpUV += pitch;
    }

    srcpUV = (int*)dst->GetWritePtr(PLANAR_V);
    for (int y=0; y<myy; ++y) {
      for (int x=0; x<myx; x++) {
        srcpUV[x] = 0x80808080;  // mod 8
      }
      srcpUV += pitch;
    }
    return dst;
  }

  if (mode==YToUV) {  // Merge UV  U=child V=clip, Y = Yclip (optional)

    PVideoFrame dst = env->NewVideoFrame(vi);

    if (vi.IsYUY2()) {
      const BYTE* srcpU = src->GetReadPtr();
      const int srcUpitch = src->GetPitch();

      PVideoFrame srcV = clip->GetFrame(n, env);
      const BYTE* srcpV = srcV->GetReadPtr();
      const int srcVpitch = srcV->GetPitch();

      short* dstp = (short*)dst->GetWritePtr();
      const int endx = dst->GetRowSize()>>1;
      const int dstpitch = dst->GetPitch()>>1;

      if (clipY) {
        PVideoFrame srcY = clipY->GetFrame(n, env);
        const BYTE* srcpY = srcY->GetReadPtr();
        const int srcYpitch = srcY->GetPitch();

        for (int y=0; y<vi.height; y++) {
          for (int x = 0; x < endx; x+=2) {
            dstp[x+0] = (srcpU[x] << 8) | srcpY[x*2+0];
            dstp[x+1] = (srcpV[x] << 8) | srcpY[x*2+2];
          }
          srcpY += srcYpitch;
          srcpU += srcUpitch;
          srcpV += srcVpitch;
          dstp += dstpitch;
        }
      }
      else {
        for (int y=0; y<vi.height; y++) {
          for (int x = 0; x < endx; x+=2) {
            dstp[x+0] = (srcpU[x] << 8) | 0x7e;  // Luma = 126
            dstp[x+1] = (srcpV[x] << 8) | 0x7e;
          }
          srcpU += srcUpitch;
          srcpV += srcVpitch;
          dstp += dstpitch;
        }
      }
      return dst;
    }

    env->BitBlt(dst->GetWritePtr(PLANAR_U),dst->GetPitch(PLANAR_U),src->GetReadPtr(PLANAR_Y),src->GetPitch(PLANAR_Y),src->GetRowSize(PLANAR_Y),src->GetHeight(PLANAR_Y));

    src = clip->GetFrame(n, env);
    env->BitBlt(dst->GetWritePtr(PLANAR_V),dst->GetPitch(PLANAR_V),src->GetReadPtr(PLANAR_Y),src->GetPitch(PLANAR_Y),src->GetRowSize(PLANAR_Y),src->GetHeight(PLANAR_Y));

    if (!clipY) {
      // Luma = 126
      const int pitch = dst->GetPitch(PLANAR_Y)/4;
      int *dstpY = (int*)dst->GetWritePtr(PLANAR_Y);
      const int myx = dst->GetRowSize(PLANAR_Y_ALIGNED)/4;
      const int myy = dst->GetHeight(PLANAR_Y);
      for (int y=0; y<myy; y++) {
        for (int x=0; x<myx; x++) {
          dstpY[x] = 0x7e7e7e7e;  // mod 4
        }
        dstpY += pitch;
      }
    } else {
      src = clipY->GetFrame(n, env);
      env->BitBlt(dst->GetWritePtr(PLANAR_Y),dst->GetPitch(PLANAR_Y),src->GetReadPtr(PLANAR_Y),src->GetPitch(PLANAR_Y),src->GetRowSize(PLANAR_Y),src->GetHeight(PLANAR_Y));
    }
    return dst;
  }
  return src;
}

}; // namespace avxsynth


