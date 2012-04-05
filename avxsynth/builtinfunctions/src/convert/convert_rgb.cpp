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

#include "convert_rgb.h"

namespace avxsynth {
	
/*************************************
 *******   RGB Helper Classes   ******
 *************************************/

RGB24to32::RGB24to32(PClip src)
  : GenericVideoFilter(src)
{
  vi.pixel_type = VideoInfo::CS_BGR32;
}


PVideoFrame __stdcall RGB24to32::GetFrame(int n, IScriptEnvironment* env)
{
  PVideoFrame src = child->GetFrame(n, env);
  PVideoFrame dst = env->NewVideoFrame(vi);
  const BYTE *p = src->GetReadPtr();
  BYTE *q = dst->GetWritePtr();
  const int src_pitch = src->GetPitch();
  const int dst_pitch = dst->GetPitch();

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
  if (env->GetCPUFlags() & CPUF_MMX) {
	int h=vi.height;
	int x_loops=vi.width; // 4 dwords/loop   read 12 bytes, write 16 bytes
	const int x_left=vi.width%4;
	x_loops-=x_left;
	x_loops*=4;
	__declspec(align(8)) static const __int64 oxffooooooffoooooo=0xff000000ff000000;

	__asm {
	push		ebx					; daft compiler assumes this is preserved!!!
    mov			esi,p
    mov			edi,q
    mov			eax,255				; Alpha channel for unaligned stosb
    mov			edx,[x_left]		; Count of unaligned pixels
    movq		mm7,[oxffooooooffoooooo]
    align 16
yloop:
    mov			ebx,0				; src offset
    mov			ecx,0				; dst offset
xloop:
    movq		mm0,[ebx+esi]		; 0000 0000 b1r0 g0b0	get b1 & a0
     movd		mm1,[ebx+esi+4]		; 0000 0000 g2b2 r1g1	get b2 & t1
	movq		mm2,mm0				; 0000 0000 b1r0 g0b0	copy b1
	 punpcklwd	mm1,mm1				; g2b2 g2b2 r1g1 r1g1	b2 in top, t1 in bottom
	psrld		mm2,24				; 0000 0000 0000 00b1	b1 in right spot
	 pslld		mm1,8				; b2g2 b200 g1r1 g100	t1 in right spot
    movd		mm3,[ebx+esi+8]		; 0000 0000 r3g3 b3r2	get a3 & t2
	 por		mm2,mm1				; b2g2 b200 g1r1 g1b1	build a1 in low mm2
	pslld		mm1,8				; g2b2 0000 r1g1 b100	clean up b2
	 psllq		mm3,24				; 00r3 g3b3 r200 0000	a3 in right spot
	psrlq		mm1,40				; 0000 0000 00g2 b200	b2 in right spot
	 punpckldq	mm0,mm2				; g1r1 g1b1 b1r0 g0b0	build a1, a0 in mm0
	por			mm1,mm3				; 00r3 g3b3 r2g2 b200	build a2
	 por		mm0,mm7				; a1r1 g1b1 a0r0 g0b0	add alpha to a1, a0
	psllq		mm1,24				; b3r2 g2b2 0000 0000	a2 in right spot
     movq		[ecx+edi],mm0		; a1r1 g1b1 a0r0 g0b0	store a1, a0
	punpckhdq	mm1,mm3				; 00r3 g3b3 b3r2 g2b2	build a3, a2 in mm1
     add		ecx,16				; bump dst index
	por			mm1,mm7				; a3r3 g3b3 a2r2 g2b2	add alpha to a3, a2
     add		ebx,12				; bump src index
    movq		[ecx+edi-8],mm1		; a3r3 g3b3 a2r2 g2b2	store a3, a2

    cmp			ecx,[x_loops]
    jl			xloop

    cmp			edx,0				; Check unaligned move count
    je			no_copy				; None, do next row
    cmp			edx,2
    je			copy_2				; Convert 2 pixels
    cmp			edx,1
    je			copy_1				; Convert 1 pixel
//copy 3
    add			esi,ebx				; else Convert 3 pixels
    add			edi,ecx
    movsb 							; b
    movsb							; g
    movsb							; r
    stosb							; a

    movsb 							; b
    movsb							; g
    movsb							; r
    stosb							; a

    movsb 							; b
    movsb							; g
    movsb							; r
    stosb							; a
    sub			esi,ebx
    sub			edi,ecx
    sub			esi,9
    sub			edi,12
    jmp			no_copy
    align 16
copy_2:
    add			esi,ebx
    add			edi,ecx
    movsb 							; b
    movsb							; g
    movsb							; r
    stosb							; a

    movsb 							; b
    movsb							; g
    movsb							; r
    stosb							; a
    sub			esi,ebx
    sub			edi,ecx
    sub			esi,6
    sub			edi,8
    jmp			no_copy
    align 16
copy_1:
    add			esi,ebx
    add			edi,ecx
    movsb 							; b
    movsb							; g
    movsb							; r
    stosb							; a
    sub			esi,ebx
    sub			edi,ecx
    sub			esi,3
    sub			edi,4
    align 16
no_copy:
    add			esi,[src_pitch]
    add			edi,[dst_pitch]

    dec			[h]
    jnz			yloop
    emms
	pop			ebx
	}
  }
  else
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
  {
	for (int y = vi.height; y > 0; --y) {
	  for (int x = 0; x < vi.width; ++x) {
		q[x*4+0] = p[x*3+0];
		q[x*4+1] = p[x*3+1];
		q[x*4+2] = p[x*3+2];
		q[x*4+3] = 255;
	  }
	  p += src_pitch;
	  q += dst_pitch;
	}
  }
  return dst;
}




RGB32to24::RGB32to24(PClip src)
: GenericVideoFilter(src)
{
  vi.pixel_type = VideoInfo::CS_BGR24;
}


PVideoFrame __stdcall RGB32to24::GetFrame(int n, IScriptEnvironment* env)
{
  PVideoFrame src = child->GetFrame(n, env);
  PVideoFrame dst = env->NewVideoFrame(vi);
  const BYTE *p = src->GetReadPtr();
  BYTE *q = dst->GetWritePtr();
  const int src_pitch = src->GetPitch();
  const int dst_pitch = dst->GetPitch();

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
  if (env->GetCPUFlags() & CPUF_MMX) {
	const int h=vi.height;
	int x_loops=vi.width; // 4 dwords/loop   read 16 bytes, write 12 bytes
	const int x_left=vi.width%4;
	x_loops-=x_left;
	x_loops*=4;
	__declspec(align(8)) static const __int64 oxooooooooooffffff=0x0000000000ffffff;
	__declspec(align(8)) static const __int64 oxooffffffoooooooo=0x00ffffff00000000;

	__asm {
	push ebx			; daft compiler assumes this is preserved!!!
    mov esi,p
    mov edi,q
    mov eax,[h]
    mov edx,[x_left];
    movq mm6,[oxooooooooooffffff];
    movq mm7,[oxooffffffoooooooo];
    align 16
yloop:
    mov ebx,0  ; src offset
    mov ecx,0  ; dst offset
xloop:
    movq mm0,[ebx+esi]    ; a1r1 g1b1 a0r0 g0b0
    movq mm1,[ebx+esi+8]  ; a3r3 g3b3 a2r2 g2b2

    movq mm2,mm0      ; a1r1 g1b1 a0r0 g0b0
    movq mm3,mm1      ; a3r3 g3b3 a2r2 g2b2

    pand mm0,mm6      ; 0000 0000 00r0 g0b0
    pand mm1,mm6      ; 0000 0000 00r2 g2b2

    pand mm2,mm7      ; 00r1 g1b1 0000 0000
    movq mm4,mm1      ; 0000 0000 00r2 g2b2

    psrlq mm2,8       ; 0000 r1g1 b100 0000
    pand mm3,mm7      ; 00r3 g3b3 0000 0000

    psllq mm4,48      ; g2b2 0000 0000 0000
    por mm0,mm2       ; 0000 r1g1 b1r0 g0b0

    psrlq mm1,16      ; 0000 0000 0000 00r2
    por mm0,mm4       ; g2b2 r1g1 b1r0 g0b0

    psrlq mm3,24      ; 0000 0000 r3g3 b300
    movq [ecx+edi],mm0

    por mm3,mm1       ; 0000 0000 r3g3 b3r2
    add ebx,16
    movd [ecx+edi+8],mm3

    add ecx,12
    cmp ebx,[x_loops]
    jl xloop


    cmp edx,0
    je no_copy
    cmp edx,2
    je copy_2
    cmp edx,1
    je copy_1
//copy 3
    add esi,ebx
    add edi,ecx
    movsb
    movsb
    movsb
    inc esi
    movsb
    movsb
    movsb
    inc esi
    movsb
    movsb
    movsb
    sub esi,ebx
    sub edi,ecx
    sub esi,11
    sub edi,9
    jmp no_copy
    align 16
copy_2:
    add esi,ebx
    add edi,ecx
    movsb
    movsb
    movsb
    inc esi
    movsb
    movsb
    movsb
    sub esi,ebx
    sub edi,ecx
    sub esi,7
    sub edi,6
    jmp no_copy
    align 16
copy_1:
    add esi,ebx
    add edi,ecx
    movsb
    movsb
    movsb
    sub esi,ebx
    sub edi,ecx
    sub esi,3
    sub edi,3
    align 16
no_copy:
    add esi,[src_pitch]
    add edi,[dst_pitch]

    dec eax
    jnz yloop
    emms
	pop ebx
	}
  }
  else
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE       
  {
	for (int y = vi.height; y > 0; --y) {
	  for (int x = 0; x < vi.width; ++x) {
		q[x*3+0] = p[x*4+0];
		q[x*3+1] = p[x*4+1];
		q[x*3+2] = p[x*4+2];
	  }
	  p += src_pitch;
	  q += dst_pitch;
	}
  }
  
  return dst;
}

}; // namespace avxsynth
