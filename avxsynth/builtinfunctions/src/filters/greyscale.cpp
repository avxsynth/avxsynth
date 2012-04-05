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

#include "greyscale.h"

namespace avxsynth {


/*************************************
 *******   Convert to Greyscale ******
 ************************************/

AVSFunction Greyscale_filters[] = {
  { "Greyscale", "c[matrix]s", Greyscale::Create },       // matrix can be "rec601", "rec709" or "Average"
  { "Grayscale", "c[matrix]s", Greyscale::Create },
  { 0 }
};

Greyscale::Greyscale(PClip _child, const char* matrix, IScriptEnvironment* env)
 : GenericVideoFilter(_child)
{
  theMatrix = Rec601;
  if (matrix) {
    if (!vi.IsRGB())
      env->ThrowError("GreyScale: invalid \"matrix\" parameter (RGB data only)");
    if (!lstrcmpi(matrix, "rec709"))
      theMatrix = Rec709;
    else if (!lstrcmpi(matrix, "Average"))
      theMatrix = Average;
    else if (!lstrcmpi(matrix, "rec601"))
      theMatrix = Rec601;
    else
      env->ThrowError("GreyScale: invalid \"matrix\" parameter (must be matrix=\"Rec601\", \"Rec709\" or \"Average\")");
  }
}


PVideoFrame Greyscale::GetFrame(int n, IScriptEnvironment* env)
{
  PVideoFrame frame = child->GetFrame(n, env);
  env->MakeWritable(&frame);
  BYTE* srcp = frame->GetWritePtr();
  int pitch = frame->GetPitch();
  int myy = vi.height;
  int myx = vi.width;

  if (vi.IsPlanar()) {
    pitch = frame->GetPitch(PLANAR_U)/4;
    int *srcpUV = (int*)frame->GetWritePtr(PLANAR_U);
    myx = frame->GetRowSize(PLANAR_U_ALIGNED)/4;
    myy = frame->GetHeight(PLANAR_U);
    for (int y=0; y<myy; y++) {
      for (int x=0; x<myx; x++) {
        srcpUV[x] = 0x80808080;  // mod 8
      }
      srcpUV += pitch;
    }
    pitch = frame->GetPitch(PLANAR_V)/4;
    srcpUV = (int*)frame->GetWritePtr(PLANAR_V);
    myx = frame->GetRowSize(PLANAR_V_ALIGNED)/4;
    myy = frame->GetHeight(PLANAR_V);
    for (int y=0; y<myy; ++y) {
      for (int x=0; x<myx; x++) {
        srcpUV[x] = 0x80808080;  // mod 8
      }
      srcpUV += pitch;
    }
  }
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE                
  else if (vi.IsYUY2() && (env->GetCPUFlags() & CPUF_MMX)) {

	  myx = __min(pitch>>1, (myx+3) & -4);	// Try for mod 8
	  __asm {
		pcmpeqw		mm7,mm7
		pcmpeqw		mm6,mm6
		psrlw		mm7,8					; 0x00ff00ff00ff00ff
		psllw		mm6,15					; 0x8000800080008000
		mov			esi,srcp				; data pointer
		mov			eax,pitch				; pitch
		mov			edx,myy					; height
		mov			edi,myx					; aligned width
		sub			eax,edi
		sub			eax,edi					; modulo = pitch - rowsize
		shr			edi,1					; number of dwords

		align		16
yloop:
		mov			ecx,edi					; number of dwords

		test		esi,0x7					; qword aligned?
		jz			xloop1					; yes

		movd		mm5,[esi]				; process 1 dword
		pand		mm5,mm7					; keep luma
		 add		esi,4					; srcp++
		por			mm5,mm6					; set chroma = 128
		 dec		ecx						; count--
		movd		[esi-4],mm5				; update 2 pixels

		align		16
xloop1:
		cmp			ecx,16					; Try to do 16 pixels per loop
		jl			xlend1

		movq		mm0,[esi+0]				; process 1 qword
		movq		mm1,[esi+8]
		pand		mm0,mm7					; keep luma
		pand		mm1,mm7
		por			mm0,mm6					; set chroma = 128
		por			mm1,mm6
		movq		[esi+0],mm0				; update 2 pixels
		movq		[esi+8],mm1

		movq		mm2,[esi+16]
		movq		mm3,[esi+24]
		pand		mm2,mm7
		pand		mm3,mm7
		por			mm2,mm6
		por			mm3,mm6
		movq		[esi+16],mm2
		movq		[esi+24],mm3

		movq		mm0,[esi+32]
		movq		mm1,[esi+40]
		pand		mm0,mm7
		pand		mm1,mm7
		por			mm0,mm6
		por			mm1,mm6
		movq		[esi+32],mm0
		movq		[esi+40],mm1

		movq		mm2,[esi+48]
		movq		mm3,[esi+56]
		pand		mm2,mm7
		pand		mm3,mm7
		por			mm2,mm6
		por			mm3,mm6
		movq		[esi+48],mm2
		movq		[esi+56],mm3

		sub			ecx,16					; count-=16
		add			esi,64					; srcp+=16

		jmp			xloop1
xlend1:
		cmp			ecx,2
		jl			xlend2

		align		16
xloop3:
		movq		mm0,[esi]				; process 1 qword
		 sub		ecx,2					; count-=2
		pand		mm0,mm7					; keep luma
		 add		esi,8					; srcp+=2
		por			mm0,mm6					; set chroma = 128
		 cmp		ecx,2					; any qwords left
		movq		[esi-8],mm0				; update 4 pixels
		 jge		xloop3					; more qwords ?

		align		16
xlend2:
		cmp			ecx,1					; 1 dword left
		jl			xlend3					; no

		movd		mm0,[esi]				; process 1 dword
		pand		mm0,mm7					; keep luma
		por			mm0,mm6					; set chroma = 128
		movd		[esi-4],mm0				; update 2 pixels
		add			esi,4					; srcp++

xlend3:
		add			esi,eax
		dec			edx
		jnle		yloop
		emms
	  }
  }
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE                 
  else if (vi.IsYUY2()) {
	for (int y=0; y<myy; ++y) {
	  for (int x=0; x<myx; x++)
		srcp[x*2+1] = 128;
	  srcp += pitch;
	}
  }
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE                  
  else if (vi.IsRGB32() && (env->GetCPUFlags() & CPUF_MMX)) {
	const int cyav = int(0.33333*32768+0.5);

	const int cyb = int(0.114*32768+0.5);
	const int cyg = int(0.587*32768+0.5);
	const int cyr = int(0.299*32768+0.5);

	const int cyb709 = int(0.0722*32768+0.5);
	const int cyg709 = int(0.7152*32768+0.5);
	const int cyr709 = int(0.2126*32768+0.5);

	__int64 rgb2lum;

	if (theMatrix == Rec709)
	  rgb2lum = ((__int64)cyr709 << 32) | (cyg709 << 16) | cyb709;
	else if (theMatrix == Average)
	  rgb2lum = ((__int64)cyav << 32) | (cyav << 16) | cyav;
	else
	  rgb2lum = ((__int64)cyr << 32) | (cyg << 16) | cyb;

    __asm {
		mov			edi,srcp
		pcmpeqd		mm1,mm1
		pxor		mm0,mm0
		psrlq		mm1,63				; 0x0000000000000001
		pcmpeqd		mm3,mm3
		psllq		mm1,46				; 0x0000400000000000
		movq		mm2,rgb2lum
		pslld		mm3,24				; 0xff000000ff000000

		xor			ecx,ecx
		mov			eax,myy
		mov			edx,myx

		align		16
rgb2lum_mmxloop:
		movq		mm6,[edi+ecx*4]		; Get 2 pixels
		 movq		mm4,mm3				; duplicate alpha mask
		movq		mm5,mm6				; duplicate pixels
		 pand		mm4,mm6				; extract alpha channel [ha00000la000000]
		punpcklbw	mm6,mm0				; [00ha|00rr|00gg|00bb]		-- low
		 punpckhbw	mm5,mm0	 			;                      		-- high
		pmaddwd		mm6,mm2				; [0*a+cyr*r|cyg*g+cyb*b]		-- low
		 pmaddwd	mm5,mm2				;                         		-- high
		punpckldq	mm7,mm6				; [loDWmm6|junk]				-- low
		 paddd		mm6,mm1				; +=0.5
		paddd		mm5,mm1				; +=0.5
		 paddd		mm6,mm7				; [hiDWmm6+32768+loDWmm6|junk]	-- low
		punpckldq	mm7,mm5				; [loDWmm5|junk]				-- high
		psrlq		mm6,47				; -> 8 bit result				-- low
		 paddd		mm5,mm7				; [hiDWmm5+32768+loDWmm5|junk]	-- high
		punpcklwd	mm6,mm6				; [0000|0000|grey|grey]		-- low
		 psrlq		mm5,47				; -> 8 bit result				-- high
		punpckldq	mm6,mm6				; [grey|grey|grey|grey]		-- low
		 punpcklwd	mm5,mm5				; [0000|0000|grey|grey]		-- high
		 punpckldq	mm5,mm5				; [grey|grey|grey|grey]		-- high
		 packuswb	mm6,mm5				; [hg|hg|hg|hg|lg|lg|lg|lg]
		 psrld		mm6,8				; [00|hg|hg|hg|00|lg|lg|lg]
		add			ecx,2				; loop counter
		 por		mm6,mm4				; [ha|hg|hg|hg|la|lg|lg|lg]
		cmp			ecx,edx				; loop >= myx
		 movq		[edi+ecx*4-8],mm6	; update 2 pixels
		jnge		rgb2lum_mmxloop

		test		edx,1				; Non-mod 2 width
		jz			rgb2lum_even

		movd		mm6,[edi+ecx*4]		; Get 1 pixels
		movq		mm4,mm3				; duplicate alpha mask
		pand		mm4,mm6				; extract alpha channel [xx00000la000000]
		punpcklbw	mm6,mm0				; [00ha|00rr|00gg|00bb]
		pmaddwd		mm6,mm2				; [0*a+cyr*r|cyg*g+cyb*b]
		punpckldq	mm7,mm6				; [loDWmm6|junk]
		paddd		mm6,mm1				; +=0.5
		paddd		mm6,mm7				; [hiDWmm6+32768+loDWmm6|junk]
		psrlq		mm6,47				; -> 8 bit result
		punpcklwd	mm6,mm6				; [0000|0000|grey|grey]
		punpckldq	mm6,mm6				; [grey|grey|grey|grey]
		packuswb	mm6,mm0				; [xx|xx|xx|xx|lg|lg|lg|lg]
		psrld		mm6,8				; [00|xx|xx|xx|00|lg|lg|lg]
		por			mm6,mm4				; [xx|xx|xx|xx|la|lg|lg|lg]
		movd		[edi+ecx*4],mm6	; update 1 pixels

rgb2lum_even:
		add			edi,pitch
		mov			edx,myx
		xor			ecx,ecx
		dec			eax
		jnle		rgb2lum_mmxloop

		emms
    }
  }
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE                   
  else if (vi.IsRGB()) {  // RGB C
    BYTE* p_count = srcp;
    const int rgb_inc = vi.IsRGB32() ? 4 : 3;
	if (theMatrix == Rec709) {
//	  const int cyb709 = int(0.0722*65536+0.5); //  4732
//	  const int cyg709 = int(0.7152*65536+0.5); // 46871
//	  const int cyr709 = int(0.2126*65536+0.5); // 13933

	  for (int y=0; y<vi.height; ++y) {
		for (int x=0; x<vi.width; x++) {
		  int greyscale=((srcp[0]*4732)+(srcp[1]*46871)+(srcp[2]*13933)+32768)>>16; // This is the correct brigtness calculations (standardized in Rec. 709)
		  srcp[0]=srcp[1]=srcp[2]=greyscale;
		  srcp += rgb_inc;
		}
		p_count+=pitch;
		srcp=p_count;
	  }
	}
	else if (theMatrix == Average) {
//	  const int cyav = int(0.333333*65536+0.5); //  21845

	  for (int y=0; y<vi.height; ++y) {
		for (int x=0; x<vi.width; x++) {
		  int greyscale=((srcp[0]+srcp[1]+srcp[2])*21845+32768)>>16; // This is the average of R, G & B
		  srcp[0]=srcp[1]=srcp[2]=greyscale;
		  srcp += rgb_inc;
		}
		p_count+=pitch;
		srcp=p_count;
	  }
	}
	else {
//	  const int cyb = int(0.114*65536+0.5); //  7471
//	  const int cyg = int(0.587*65536+0.5); // 38470
//	  const int cyr = int(0.299*65536+0.5); // 19595

	  for (int y=0; y<vi.height; ++y) {
		for (int x=0; x<vi.width; x++) {
		  int greyscale=((srcp[0]*7471)+(srcp[1]*38470)+(srcp[2]*19595)+32768)>>16; // This produces similar results as YUY2 (luma calculation)
		  srcp[0]=srcp[1]=srcp[2]=greyscale;
		  srcp += rgb_inc;
		}
		p_count+=pitch;
		srcp=p_count;
	  }
	}
  }
  return frame;
}


AVSValue __cdecl Greyscale::Create(AVSValue args, void*, IScriptEnvironment* env)
{
  return new Greyscale(args[0].AsClip(), args[1].AsString(0), env);
}

}; // namespace avxsynth
