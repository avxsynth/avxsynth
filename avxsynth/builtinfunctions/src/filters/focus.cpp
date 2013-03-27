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

#include "common/include/stdafx.h"

#include "focus.h"
#include "common/include/filters/text-overlay.h"



namespace avxsynth {

 
/********************************************************************
***** Declare index of new filters for Avisynth's filter engine *****
********************************************************************/

AVSFunction Focus_filters[] = {
  { "Blur", "cf[]f[mmx]b", Create_Blur },                     // amount [-1.0 - 1.5849625] -- log2(3)
  { "Sharpen", "cf[]f[mmx]b", Create_Sharpen },               // amount [-1.5849625 - 1.0]
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE  
  { "TemporalSoften", "ciii[scenechange]i[mode]i", TemporalSoften::Create }, // radius, luma_threshold, chroma_threshold
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
  { "SpatialSoften", "ciii", SpatialSoften::Create },   // radius, luma_threshold, chroma_threshold
  { 0 }
};


 


/****************************************
 ***  AdjustFocus helper classes     ***
 ***  Originally by Ben R.G.         ***
 ***  MMX code by Marc FD            ***
 ***  Adaptation and bugfixes sh0dan ***
 ***  Code actually requires ISSE!   ***
 ***  Not anymore - pure MMX    IanB ***
 ***  Implement boundary proc.  IanB ***
 ***  Impl. full 8bit MMX proc. IanB ***
 ***************************************/

AdjustFocusV::AdjustFocusV(double _amount, PClip _child, bool _mmx)
: GenericVideoFilter(_child), mmx(_mmx), amount(int(32768*pow(2.0, _amount)+0.5)), line(NULL) {}

AdjustFocusV::~AdjustFocusV(void) 
{ 
  if (line) delete[] line; 
}

// --------------------------------
// Blur/Sharpen Vertical GetFrame()
// --------------------------------

PVideoFrame __stdcall AdjustFocusV::GetFrame(int n, IScriptEnvironment* env) 
{
	PVideoFrame frame = child->GetFrame(n, env);
	env->MakeWritable(&frame);
	if (!line)
		line = new uc[frame->GetRowSize()+32];
	uc* linea = (uc*)(((uint64_t)line+15) & -16); // Align 16

	if (vi.IsPlanar()) {
    int plane,cplane;
		for(cplane=0;cplane<3;cplane++) {
			if (cplane==0)  plane = PLANAR_Y;
			if (cplane==1)  plane = PLANAR_U;
			if (cplane==2)  plane = PLANAR_V;
			uc* buf      = frame->GetWritePtr(plane);
			int pitch    = frame->GetPitch(plane);
			int row_size = frame->GetRowSize(plane);
			int height   = frame->GetHeight(plane);
			memcpy(linea, buf, row_size); // First row - map centre as upper
			// All normal cases will have pitch aligned 16, we
			// need 8. If someone works hard enough to override
			// this we can't process the short fall. Use C Code.
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE                
			if (mmx && (pitch >= ((row_size+7) & -8))) {
				AFV_MMX(linea, buf, height, pitch, row_size, amount);
			} else 
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE               
            {
				AFV_C(linea, buf, height, pitch, row_size, amount);
			}
		}

	} else {
		uc* buf      = frame->GetWritePtr();
		int pitch    = frame->GetPitch();
		int row_size = vi.RowSize();
		int height   = vi.height;
		memcpy(linea, buf, row_size); // First row - map centre as upper
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE            
		if (mmx && (pitch >= ((row_size+7) & -8))) {
			AFV_MMX(linea, buf, height, pitch, row_size, amount);
		} else 
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE           
        {
			AFV_C(linea, buf, height, pitch, row_size, amount);
		}
	}
	return frame;
}

// ------------------------------
// Blur/Sharpen Vertical C++ Code
// ------------------------------

void AFV_C(uc* l, uc* p, const int height, const int pitch, const int row_size, const int amount) {
	const int center_weight = amount*2;
	const int outer_weight = 32768-amount;
	for (int y = height-1; y>0; --y) {
		for (int x = 0; x < row_size; ++x) {
			uc a = ScaledPixelClip(p[x] * center_weight + (l[x] + p[x+pitch]) * outer_weight);
			l[x] = p[x];
			p[x] = a;
		}
		p += pitch;
	}
	for (int x = 0; x < row_size; ++x) { // Last row - map centre as lower
		p[x] = ScaledPixelClip(p[x] * center_weight + (l[x] + p[x]) * outer_weight);
	}
}

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE

// ------------------------------
// Blur/Sharpen Vertical MMX Code
// ------------------------------

void AFV_MMX(const uc* l, const uc* p, const int height, const int pitch, const int row_size, const int amount) {
	// round masks
	__declspec(align(8)) const static __int64 r7 = 0x0040004000400040;

	__asm { 
		sub		esp,8				; temp space on stack
		// signed word center weight ((amount+0x100)>>9) x4
		mov		eax,amount
		add		eax,100h
		sar		eax,9               ; [21..128]
		mov		[esp],ax
		mov		[esp+2],ax
		mov		[esp+4],ax
		mov		[esp+6],ax
		movq	mm7,[esp]
		// signed word outer weight 64-((amount+0x100)>>9) x4
		mov		ax,40h
		sub		ax,[esp]            ; [43..-64]
		mov		[esp],ax
		mov		[esp+2],ax
		mov		[esp+4],ax
		mov		[esp+6],ax
		movq	mm6,[esp]

		add		esp,8
		pxor	mm0,mm0
	}
	
	int nloops = __std::min(pitch, (row_size+7) & -8) >> 3;

	for (int y = height-1; y>0; --y) {

	__asm {
		mov			edx,p			; frame buffer
		mov			ecx,pitch
		mov			eax,edx
		add			eax,ecx
		mov			p,eax			; p += pitch;
		mov			eax,l			; line buffer
		mov			edi,nloops

		align		16
row_loop:
		movq		mm4,[eax]		; 8 Upper pixels (buffered)
		 movq		mm5,[edx+ecx]	; 8 Lower pixels
		movq		mm1,mm4			; Duplicate uppers
		 movq		mm3,mm5			; Duplicate lowers
		punpcklbw	mm4,mm0			; unpack 4 low bytes uppers
		 movq		mm2,[edx]		; 8 Centre pixels
		punpcklbw	mm5,mm0			; unpack 4 low bytes lowers
		 movq		[eax],mm2		; Save centres as next uppers
		paddw		mm5,mm4			; Uppers + Lowers				-- low
		 movq		mm4,mm2			; Duplicate centres
		pmullw		mm5,mm6			; *= outer weight				-- low [-32130, 21930]
		 punpcklbw	mm4,mm0			; unpack 4 low bytes centres
		punpckhbw	mm1,mm0			; unpack 4 high bytes uppers
		 punpckhbw	mm3,mm0			; unpack 4 high bytes lowers
		pmullw		mm4,mm7			; *= centre weight				-- low [32640, 5355]
		 paddw		mm1,mm3			; Uppers + Lowers				-- high
		punpckhbw	mm2,mm0			; unpack 4 high bytes centres
		 pmullw		mm1,mm6			; *= outer weight				-- high
		pmullw		mm2,mm7			; *= centre weight				-- high
		 paddsw		mm5,mm4			; Weighted outers+=0.5*centres	-- low
		paddsw		mm1,mm2			; Weighted outers+=0.5*centres	-- high
		 paddsw		mm5,mm4			; +=0.5*centres					-- low
		paddsw		mm1,mm2			; +=0.5*centres					-- high
 		 paddsw		mm5,r7			; += 0.5						-- low
		paddsw		mm1,r7			; += 0.5						-- high
		 psraw		mm5,7			; /= 128						-- low
		add			eax,8			; upper += 8
		 psraw		mm1,7			; /= 128						-- high
		add			edx,8			; centre += 8
		 packuswb	mm5,mm1			; pack 4 lows with 4 highs
		dec			edi				; count--
		 movq		[edx-8],mm5		; Update 8 pixels
		jnle		row_loop		; 
		}

	} // for (int y = height

	__asm { // Last row - map centre as lower
		mov			edx,p			; frame buffer
		mov			eax,l			; line buffer
		mov			edi,nloops

		align		16
lrow_loop:
		movq		mm5,[eax]		; 8 Upper pixels (buffered)
		 movq		mm4,[edx]		; 8 Centre pixels as lowers
		movq		mm1,mm5			; Duplicate uppers
		 movq		mm3,mm4			; Duplicate lowers (centres)
		punpcklbw	mm5,mm0			; unpack 4 low bytes uppers
		 punpcklbw	mm4,mm0			; unpack 4 low bytes lowers (centres)
		punpckhbw	mm3,mm0			; unpack 4 high bytes lowers (centres)
		 paddw		mm5,mm4			; Uppers + lowers (centres)		-- low
		punpckhbw	mm1,mm0			; unpack 4 high bytes uppers
		 pmullw		mm5,mm6			; *= outer weight				-- low
		paddw		mm1,mm3			; Uppers + lowers (centres)		-- high
		 pmullw		mm4,mm7			; *= centre weight				-- low
		pmullw		mm1,mm6			; *= outer weight				-- high
		 pmullw		mm3,mm7			; *= centre weight				-- high
		paddsw		mm5,mm4			; Weighted outers+=0.5*centres	-- low
		 paddsw		mm1,mm3			; Weighted outers+=0.5*centres	-- high
		paddsw		mm5,mm4			; +=0.5*centres					-- low
		 paddsw		mm1,mm3			; +=0.5*centres					-- high
		paddsw		mm5,r7			; += 0.5						-- low
		 paddsw		mm1,r7			; += 0.5						-- high
		psraw		mm5,7			; /= 128						-- low
		 add		eax,8			; upper += 8
		psraw		mm1,7			; /= 128						-- high
		 add		edx,8			; centre += 8
		packuswb	mm5,mm1			; pack 4 lows with 4 highs
		 dec		edi				; count--
		movq		[edx-8],mm5		; Update 8 pixels
		 jnle		lrow_loop		; 
	}
	__asm emms
}
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE

AdjustFocusH::AdjustFocusH(double _amount, PClip _child, bool _mmx)
: GenericVideoFilter(_child), mmx(_mmx), amount(int(32768*pow(2.0, _amount)+0.5)) {}

// ----------------------------------
// Blur/Sharpen Horizontal GetFrame()
// ----------------------------------

PVideoFrame __stdcall AdjustFocusH::GetFrame(int n, IScriptEnvironment* env) 
{
	PVideoFrame frame = child->GetFrame(n, env);
	env->MakeWritable(&frame); // Damn! This screws FillBorder

	if (vi.IsPlanar()) {
    int plane,cplane;
		for(cplane=0;cplane<3;cplane++) {
		if (cplane==0) plane = PLANAR_Y;
		if (cplane==1) plane = PLANAR_U;
		if (cplane==2) plane = PLANAR_V;
			const int row_size = frame->GetRowSize(plane);
			uc* q = frame->GetWritePtr(plane);
			const int pitch = frame->GetPitch(plane);
			int height = frame->GetHeight(plane);
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE            
			if (mmx && (pitch >= ((row_size+7) & -8))) {
				AFH_YV12_MMX(q,height,pitch,row_size,amount);
			} else 
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
            {
				AFH_YV12_C(q,height,pitch,row_size,amount);
			} 
		}
	} else {
		const int row_size = vi.RowSize();
		uc* q = frame->GetWritePtr();
		const int pitch = frame->GetPitch();
		if (vi.IsYUY2()) {
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE            
			if (mmx) {
				AFH_YUY2_MMX(q,vi.height,pitch,vi.width,amount);
			} else 
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE                
            {
				AFH_YUY2_C(q,vi.height,pitch,vi.width,amount);
			}
		} 
		else if (vi.IsRGB32()) {
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE            
			if (mmx) {
				AFH_RGB32_MMX(q,vi.height,pitch,vi.width,amount);
			} else 
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE               
            {
				AFH_RGB32_C(q,vi.height,pitch,vi.width,amount);
			}
		} 
		else { //rgb24
			AFH_RGB24_C(q,vi.height,pitch,vi.width,amount);
		}
	}

	return frame;
}

// --------------------------------------
// Blur/Sharpen Horizontal RGB32 C++ Code
// --------------------------------------

void AFH_RGB32_C(uc* p, int height, const int pitch, const int width, const int amount) {
	const int center_weight = amount*2;
	const int outer_weight = 32768-amount;
	int x;
	for (int y = height; y>0; --y) 
	{
		uc bb = p[0];
		uc gg = p[1];
		uc rr = p[2];
		uc aa = p[3];
		for (x = 0; x < width-1; ++x) 
		{
			uc b = ScaledPixelClip(p[x*4+0] * center_weight + (bb + p[x*4+4]) * outer_weight);
			bb = p[x*4+0]; p[x*4+0] = b;
			uc g = ScaledPixelClip(p[x*4+1] * center_weight + (gg + p[x*4+5]) * outer_weight);
			gg = p[x*4+1]; p[x*4+1] = g;
			uc r = ScaledPixelClip(p[x*4+2] * center_weight + (rr + p[x*4+6]) * outer_weight);
			rr = p[x*4+2]; p[x*4+2] = r;
			uc a = ScaledPixelClip(p[x*4+3] * center_weight + (aa + p[x*4+7]) * outer_weight);
			aa = p[x*4+3]; p[x*4+3] = a;
		}
		p[x*4+0] = ScaledPixelClip(p[x*4+0] * center_weight + (bb + p[x*4+0]) * outer_weight);
		p[x*4+1] = ScaledPixelClip(p[x*4+1] * center_weight + (gg + p[x*4+1]) * outer_weight);
		p[x*4+2] = ScaledPixelClip(p[x*4+2] * center_weight + (rr + p[x*4+2]) * outer_weight);
		p[x*4+3] = ScaledPixelClip(p[x*4+3] * center_weight + (aa + p[x*4+3]) * outer_weight);
		p += pitch;
	}
}

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE

// --------------------------------------
// Blur/Sharpen Horizontal RGB32 MMX Code
// --------------------------------------

void AFH_RGB32_MMX(const uc* p, const int height, const int pitch, const int width, const int amount) {
	// round masks
	__declspec(align(8)) const __int64 r7 = 0x0040004000400040;
	// weights
	__declspec(align(8)) __int64 cw;
	__declspec(align(8)) __int64 ow;
	__asm { 
		// signed word center weight ((amount+0x100)>>9) x4
		mov		eax,amount
		add		eax,100h
		sar		eax,9
		lea		edx,cw
		mov		[edx],ax
		mov		[edx+2],ax
		mov		[edx+4],ax
		mov		[edx+6],ax
		// signed word outer weight 64-((amount+0x100)>>9) x4
		mov		ax,40h
		sub		ax,[edx]
		lea		edx,ow
		mov		[edx],ax
		mov		[edx+2],ax
		mov		[edx+4],ax
		mov		[edx+6],ax
	}
	for (int y=0;y<height;y++) {
	__asm {
		mov			ecx,p
		mov			edi,width

		movq		mm1,[ecx]		; trash + left pixel
		pxor		mm0,mm0			; zeros
		movq		mm2,mm1			; centre + right pixel
		psllq		mm1,32			; left + zero

		align		16
row_loop:
		dec			edi
		jle			odd_end

		movq		mm7,mm2			; duplicate right pixel
		punpckhbw	mm1,mm0			; unpack left pixel
		punpckhbw	mm7,mm0			; unpack right pixel
		movq		mm4,mm2			; duplicate centre pixel
		paddsw		mm7,mm1			; right + left
		punpcklbw	mm4,mm0			; unpack centre pixel
		pmullw		mm7,ow			; *= outer weight
		pmullw		mm4,cw			; *= centre weight
		movq		mm1,mm2			; left + centre pixel
		paddsw		mm7,mm4			; Weighted centres + outers
		dec			edi
		paddsw		mm7,mm4			; Weighted centres + outers
		paddsw		mm7,r7			; += 0.5
		psraw		mm7,7			; /= 32768
		jle			even_end

		movq		mm6,mm1			; duplicate left pixel
		movq		mm2,[ecx+8]		; right + trash pixel
		punpcklbw	mm6,mm0			; unpack left pixel
		movq		mm5,mm2			; duplicate right pixel
		movq		mm4,mm1			; duplicate centre pixel
		punpcklbw	mm5,mm0			; unpack right pixel
		punpckhbw	mm4,mm0			; unpack centre pixel
		paddsw		mm6,mm5			; left + right
		pmullw		mm4,cw			; *= centre weight
		pmullw		mm6,ow			; *= outer weight
		paddsw		mm6,mm4			; Weighted centres + outers
		paddsw		mm6,mm4			; Weighted centres + outers
		paddsw		mm6,r7			; += 0.5
		psraw		mm6,7			; /= 32768
		add			ecx,8
		packuswb	mm7,mm6			; pack low with high
		movq		[ecx-8],mm7		; Update 2 centre pixels
		jmp			row_loop

		align		16
odd_end:
		punpckhbw	mm1,mm0			; unpack left pixel
		punpcklbw	mm2,mm0			; unpack centre pixel
		paddsw		mm1,mm2			; left + centre
		pmullw		mm2,cw			; *= centre weight
		pmullw		mm1,ow			; *= outer weight
		paddsw		mm1,mm2			; Weighted centres + outers
		paddsw		mm1,mm2			; Weighted centres + outers
		paddsw		mm1,r7			; += 0.5
		psraw		mm1,7			; /= 32768
		packuswb	mm1,mm1			; pack low with high
		movd		[ecx],mm1		; Update 1 centre pixels
		jmp			next_loop

		align		16
even_end:
		punpckhbw	mm2,mm0			; unpack centre pixel
		punpcklbw	mm1,mm0			; unpack left pixel
		paddsw		mm1,mm2			; left + centre
		pmullw		mm2,cw			; *= centre weight
		pmullw		mm1,ow			; *= outer weight
		paddsw		mm1,mm2			; Weighted centres + outers
		paddsw		mm1,mm2			; Weighted centres + outers
		paddsw		mm1,r7			; += 0.5
		psraw		mm1,7			; /= 32768
		packuswb	mm7,mm1			; pack low with high
		movq		[ecx],mm7		; Update 2 centre pixels

next_loop:
		}
		p += pitch;
	}
	__asm emms
}

#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE

// -------------------------------------
// Blur/Sharpen Horizontal YUY2 C++ Code
// -------------------------------------

void AFH_YUY2_C(uc* p, int height, const int pitch, const int width, const int amount) {
	const int center_weight = amount*2;
	const int outer_weight = 32768-amount;
	int x;
	for (int y = height; y>0; --y) 
	{
		uc yy = p[0];
		uc uv = p[1];
		uc vu = p[3];
		for (x = 0; x < width-2; ++x) 
		{
			uc y = ScaledPixelClip(p[x*2+0] * center_weight + (yy + p[x*2+2]) * outer_weight);
			yy   = p[x*2+0];
			p[x*2+0] = y;
			uc w = ScaledPixelClip(p[x*2+1] * center_weight + (uv + p[x*2+5]) * outer_weight);
			uv   = vu;
			vu   = p[x*2+1];
			p[x*2+1] = w;
		}
		uc c     = ScaledPixelClip(p[x*2+0] * center_weight + (yy + p[x*2+2]) * outer_weight);
		yy       = p[x*2+0];
		p[x*2+0] = c;
		p[x*2+1] = ScaledPixelClip(p[x*2+1] * center_weight + (uv + p[x*2+1]) * outer_weight);
		p[x*2+2] = ScaledPixelClip(p[x*2+2] * center_weight + (yy + p[x*2+2]) * outer_weight);
		p[x*2+3] = ScaledPixelClip(p[x*2+3] * center_weight + (vu + p[x*2+3]) * outer_weight);
   
		p += pitch;
	}
}

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE

// -------------------------------------
// Blur/Sharpen Horizontal YUY2 MMX Code
// -------------------------------------

void AFH_YUY2_MMX(const uc* p, const int height, const int pitch, const int width, const int amount) {
	// round masks
	__declspec(align(8)) const __int64 r7 = 0x0040004000400040;
	// YY and UV masks
	__declspec(align(8)) const __int64 ym = 0x00FF00FF00FF00FF;
	__declspec(align(8)) const __int64 cm = 0xFF00FF00FF00FF00;

	__declspec(align(8)) const __int64 rightmask = 0x00FF000000000000;
	// weights
	__declspec(align(8)) __int64 cw;
	__declspec(align(8)) __int64 ow;

	__asm { 
		// signed word center weight ((amount+0x100)>>9) x4
		mov		eax,amount
		add		eax,100h
		sar		eax,9
		lea		edx,cw
		mov		[edx],ax
		mov		[edx+2],ax
		mov		[edx+4],ax
		mov		[edx+6],ax
		// signed word outer weight 64-((amount+0x100)>>9) x4
		mov		ax,40h
		sub		ax,[edx]
		lea		edx,ow
		mov		[edx],ax
		mov		[edx+2],ax
		mov		[edx+4],ax
		mov		[edx+6],ax
	}

	for (int y=0;y<height;y++) {
	__asm {
		mov			ecx,p
		 movq		mm1,ym			; 0x00FF00FF00FF00FF
		movq		mm2,[ecx]		; [2v 3y 2u 2y 0v 1y 0u 0y]	-- Centre
		 movq		mm3,cm			; 0xFF00FF00FF00FF00
		pand		mm1,mm2			; [00 3y 00 2y 00 1y 00 0y]
		 pand		mm3,mm2			; [2v 00 2u 00 0v 00 0u 00]
		psllq		mm1,48			; [00 0y 00 00 00 00 00 00]
		 psllq		mm3,32			; [0v 00 0u 00 00 00 00 00]
		mov			edi,width
		 por		mm1,mm3			; [0v 0y 0u 00 00 00 00 00]	-- Left
		sub			edi,2

		align		16
row_loop:
		jle			odd_end
		sub			edi,2
		jle			even_end

		movq		mm0,mm1			;   [-2v -1y -2u -2y -4v -3y -4u -4y]
		 movq		mm3,[ecx+8]		; [6v 7y 6u 6y 4v 5y 4u 4y]	-- Right
		movq		mm5,ym			; 0x00FF00FF00FF00FF
		 movq		mm6,mm3			;   [6v 7y 6u 6y 4v 5y 4u 4y]
		pand		mm0,mm5			;   [00-1y 00-2y 00-3y 00-4y]
		 pand		mm6,mm5			;   [007y 006y 005y 004y]
		psrlq		mm0,48			;   [0000  0000  0000  00-1y]
		 pand		mm5,mm2			; [003y 002y 001y 000y]		-- Centre
		psllq		mm6,48			;   [004y 0000 0000 0000]
		 movq		mm7,mm5			;   [003y 002y 001y 000y]
		movq		mm4,mm5			;   [003y 002y 001y 000y]
		 psllq		mm7,16			;   [002y 001y 000y 0000]
		psrlq		mm5,16			;   [0000 003y 002y 001y]
		 por		mm0,mm7			; [002y 001y 000y 00-1y]	-- Left
		por			mm6,mm5			; [004y 003y 002y 001y]		-- Right
		 pmullw		mm4,cw			; *= center weight
		paddsw		mm0,mm6			; left += right 
		 movq		mm6,cm			; 0xFF00FF00FF00FF00
		pmullw		mm0,ow			; *= outer weight
		 pand		mm1,mm6			;   [-2v00 -2u00 -4v00 -4u00]
		movq		mm5,mm2			;   [2v 3y 2u 2y 0v 1y 0u 0y]
		 paddsw		mm0,mm4			; Weighted centres + outers
		psrlq		mm1,40			;   [0000 0000 00-2v 00-2u]
		 pand		mm5,mm6			;   [2v00 2u00 0v00 0u00]
		paddsw		mm0,mm4			; Weighted centres + outers
		 psrlq		mm5,8			; [002v 002u 000v 000u]		-- Centre
		pand		mm6,mm3			;   [6v00 6u00 4v00 4u00]
		 movq		mm7,mm5			;   [002v 002u 000v 000u]
		movq		mm4,mm5			;   [002v 002u 000v 000u]
		 psllq		mm6,24			;   [004v 004u 0000 0000]
		psllq		mm7,32			;   [000v 000u 0000 0000]
		 psrlq		mm4,32			;   [0000 0000 002v 002u]
		por			mm7,mm1			; [000v 000u 00-2v 00-2u]	-- Left
		 por		mm6,mm4			; [004v 004u 002v 002u]		-- Right
		 paddsw		mm7,mm6			; left += right
		pmullw		mm5,cw			; *= center weight
		 pmullw		mm7,ow			; *= outer weight
		add			ecx,8			; 
		 paddsw		mm7,mm5			; Weighted centres + outers
		paddsw		mm0,r7			; += 0.5
		 paddsw		mm7,mm5			; Weighted centres + outers
		psraw		mm0,7			; /= 32768
		 paddsw		mm7,r7			; += 0.5
		packuswb	mm0,mm0			; [3y 2y 1y 0y 3y 2y 1y 0y] -- Unsign Saturated
		 psraw		mm7,7			; /= 32768
		movq		mm1,mm2			; 
		 packuswb	mm7,mm7			; [2v 2u 0v 0u 2v 2u 0v 0u] -- Unsign Saturated
		movq		mm2,mm3			; 
		 punpcklbw	mm0,mm7			; [2v 3y 2u 2y 0v 1y 0v 0y]
		sub			edi,2
		 movq		[ecx-8],mm0		; Update 4 centre pixels
		jmp			row_loop		; 

		align		16
odd_end:
		movq		mm5,ym			; 0x00FF00FF00FF00FF
		 movq		mm0,mm1			;   [-2v -1y -2u -2y -4v -3y -4u -4y]
	;stall
		 pand		mm0,mm5			;   [00-1y 00-2y 00-3y 00-4y]
		pand		mm5,mm2			; [00xx 00xx 001y 000y]		-- Centre
		 psrlq		mm0,48			;   [0000  0000  0000  00-1y]
		movq		mm7,mm5			;   [00xx 00xx 001y 000y]
		 movq		mm6,mm5			;   [00xx 00xx 001y 000y]
		psllq		mm7,16			;   [00xx 001y 000y 0000]
		 psrlq		mm6,16			;   [0000 00xx 00xx 001y]
		por			mm0,mm7			; [00xx 001y 000y 00-1y]	-- Left
		 punpcklwd	mm6,mm6			; [00xx 00xx 001y 001y]		-- Right
		pmullw		mm5,cw			; *= center weight YY
		 paddsw		mm0,mm6			; left += right  YY
		movq		mm3,cm			; 0xFF00FF00FF00FF00
		 pmullw		mm0,ow			; *= outer weight YY
		pand		mm1,mm3			;   [-2v00 -2u00 -4v00 -4u00]
		pand		mm3,mm2			;   [xx00 xx00 0v00 0u00]
		 psrlq		mm1,40			; [0000 0000 00-2v 00-2u]	-- Left
		psrlq		mm3,8			; [0000 0000 000v 000u]		-- Centre
		paddsw		mm1,mm3			; left += centre UV
		 pmullw		mm3,cw			; *= center weight UV
		pmullw		mm1,ow			; *= outer weight UV
		 paddsw		mm0,mm5			; Weighted centres + outers YY
		paddsw		mm1,mm3			; Weighted centres + outers UV
		 paddsw		mm0,mm5			; Weighted centres + outers YY
		paddsw		mm1,mm3			; Weighted centres + outers UV
		 paddsw		mm0,r7			; += 0.5 YY
		paddsw		mm1,r7			; += 0.5 UV
		 psraw		mm0,7			; /= 32768 YY
		psraw		mm1,7			; /= 32768 UV
		 packuswb	mm0,mm0			; [xx xx 1y 0y xx xx 1y 0y] -- Unsign Saturated
		packuswb	mm1,mm1			; [xx xx 0v 0u xx xx 0v 0u] -- Unsign Saturated
		punpcklbw	mm0,mm1			; [xx xx xx xx 0v 1y 0v 0y]
		movd		[ecx],mm0		; Update 2 centre pixels
		jmp			next_loop

		align		16
even_end:

		 movq		mm5,ym			; 0x00FF00FF00FF00FF
		movq		mm0,mm1			;   [-2v -1y -2u -2y -4v -3y -4u -4y]
		 movq		mm6,rightmask	; 0x00FF000000000000
		pand		mm0,mm5			;   [00-1y 00-2y 00-3y 00-4y]
		 pand		mm6,mm2			;   [003y 0000 0000 0000]
		pand		mm5,mm2			; [003y 002y 001y 000y]		-- Centre
		 psrlq		mm0,48			;   [0000  0000  0000  00-1y]
		movq		mm7,mm5			;   [003y 002y 001y 000y]
		 movq		mm4,mm5			;   [003y 002y 001y 000y]
		psllq		mm7,16			;   [002y 001y 000y 0000]
		 psrlq		mm4,16			;   [0000 003y 002y 001y]
		por			mm0,mm7			; [002y 001y 000y 00-1y]	-- Left
		 por		mm6,mm4			; [003y 003y 002y 001y]		-- Right
		movq		mm3,cm			; 0xFF00FF00FF00FF00
		 paddsw		mm0,mm6			; left += right 
		pmullw		mm5,cw			; *= center weight
		 pmullw		mm0,ow			; *= outer weight
		 pand		mm1,mm3			;   [-2v00 -2u00 -4v00 -4u00]
		pand		mm3,mm2			;   [2v00 2u00 0v00 0u00]
		 psrlq		mm1,40			;   [0000 0000 00-2v 00-2u]
		movq		mm4,mm3			;   [2v00 2u00 0v00 0u00]
		 movq		mm7,mm3			;   [2v00 2u00 0v00 0u00]
		psrlq		mm4,40			;   [0000 0000 002v 002u]
		 psllq		mm7,24			;   [000v 000u 0000 0000]
		movq		mm6,mm4			;   [0000 0000 002v 002u]
		 por		mm1,mm7			; [000v 000u 00-2v 00-2u]	-- Left
		psllq		mm6,32			;   [002v 002u 0000 0000]
		por			mm6,mm4			; [002v 002u 002v 002u]		-- Right
		 psrlq		mm3,8			; [002v 002u 000v 000u]		-- Centre
		paddsw		mm1,mm6			; left += right
		 pmullw		mm3,cw			; *= center weight
		pmullw		mm1,ow			; *= outer weight
		 paddsw		mm0,mm5			; Weighted centres + outers
		paddsw		mm1,mm3			; Weighted centres + outers
		 paddsw		mm0,mm5			; Weighted centres + outers
		paddsw		mm1,mm3			; Weighted centres + outers
		 paddsw		mm0,r7			; += 0.5
		paddsw		mm1,r7			; += 0.5
		 psraw		mm0,7			; /= 32768
		psraw		mm1,7			; /= 32768
		 packuswb	mm0,mm0			; [3y 2y 1y 0y 3y 2y 1y 0y] -- Unsign Saturated
		packuswb	mm1,mm1			; [2v 2u 0v 0u 2v 2u 0v 0u] -- Unsign Saturated
		punpcklbw	mm0,mm1			; [2v 3y 2u 2y 0v 1y 0v 0y]
		movq		[ecx],mm0		; Update 4 centre pixels

next_loop:
		}
	p += pitch;
	}
	__asm emms
}
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE

// --------------------------------------
// Blur/Sharpen Horizontal RGB24 C++ Code
// --------------------------------------

void AFH_RGB24_C(uc* p, int height, const int pitch, const int width, const int amount) {
  const int center_weight = amount*2;
  const int outer_weight = 32768-amount;
  int x;
  for (int y = height; y>0; --y) 
    {

      uc bb = p[0];
      uc gg = p[1];
      uc rr = p[2];
      for (x = 0; x < width-1; ++x) 
      {
        uc b = ScaledPixelClip(p[x*3+0] * center_weight + (bb + p[x*3+3]) * outer_weight);
        bb = p[x*3+0]; p[x*3+0] = b;
        uc g = ScaledPixelClip(p[x*3+1] * center_weight + (gg + p[x*3+4]) * outer_weight);
        gg = p[x*3+1]; p[x*3+1] = g;
        uc r = ScaledPixelClip(p[x*3+2] * center_weight + (rr + p[x*3+5]) * outer_weight);
        rr = p[x*3+2]; p[x*3+2] = r;
      }
      p[x*3+0] = ScaledPixelClip(p[x*3+0] * center_weight + (bb + p[x*3+0]) * outer_weight);
      p[x*3+1] = ScaledPixelClip(p[x*3+1] * center_weight + (gg + p[x*3+1]) * outer_weight);
      p[x*3+2] = ScaledPixelClip(p[x*3+2] * center_weight + (rr + p[x*3+2]) * outer_weight);
      p += pitch;
    }
}

// -------------------------------------
// Blur/Sharpen Horizontal YV12 C++ Code
// -------------------------------------

void AFH_YV12_C(uc* p, int height, const int pitch, const int row_size, const int amount) 
{
	const int center_weight = amount*2;
	const int outer_weight = 32768-amount;
	uc pp,l;
	int x;
	for (int y = height; y>0; --y) {
		l = p[0];
		for (x = 1; x < row_size-1; ++x) {
			pp = ScaledPixelClip(p[x] * center_weight + (l + p[x+1]) * outer_weight);
			l=p[x]; p[x]=pp;
		}
		p[x] = ScaledPixelClip(p[x] * center_weight + (l + p[x]) * outer_weight);
		p += pitch;
	}
}

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE

// -------------------------------------
// Blur/Sharpen Horizontal YV12 MMX Code
// -------------------------------------

#define scale(mmAA,mmBB,mmCC,mmA,mmB,mmC,zeros)	\
__asm	movq		mmC,mmCC	/* Right 8 pixels      */\
__asm	 movq		mmA,mmAA	/* Left     "          */\
__asm	movq		mmB,mmBB	/* Centre   "          */\
__asm	 punpcklbw	mmC,zeros	/* Low 4 right         */\
__asm	punpcklbw	mmA,zeros	/* Low 4 left          */\
__asm	 punpcklbw	mmB,zeros	/* Low 4 centre        */\
__asm	paddsw		mmA,mmC		/* Low 4 left + right  */\
__asm	 pmullw		mmB,cw		/* *= centre weight    */\
__asm	pmullw		mmA,ow		/* *= outer weight     */\
__asm	 punpckhbw	mmCC,zeros	/* High 4 Right        */\
__asm	punpckhbw	mmAA,zeros	/* High 4 Left         */\
__asm	 punpckhbw	mmBB,zeros	/* High 4 Centre       */\
__asm	paddsw		mmAA,mmCC	/* High 4 left + right */\
__asm	 pmullw		mmBB,cw		/* *= centre weight    */\
__asm	pmullw		mmAA,ow		/* *= outer weight     */\
__asm	 paddsw		mmA,mmB		/* += weighed low 4    */\
__asm	paddsw		mmAA,mmBB	/* += weighed high 4   */\
__asm	 paddsw		mmA,mmB		/* += weighed low 4    */\
__asm	paddsw		mmAA,mmBB	/* += weighed high 4   */\
__asm	 paddsw		mmA,r7		/* += 0.5              */\
__asm	paddsw		mmAA,r7		/* += 0.5              */\
__asm	 psraw		mmA,7		/* /= 128              */\
__asm	psraw		mmAA,7		/* /= 128              */\
__asm	 add		ecx,8		/* p += 8              */\
__asm	packuswb	mmA,mmAA	/* Packed new 8 pixels */

// 
// Planer MMX blur/sharpen - process 8 pixels at a time
//   FillBorder::Create(_child)) ensures the right edge is repeated to mod 8 width
//   frame->GetRowSize(plane|PLANAR_ALIGNED) ensured rowsize is also mod 8
// For pitch less than 8 C code is used (unlikely, I couldn't force a case)
// 
void AFH_YV12_MMX(uc* p, int height, const int pitch, const int row_size, const int amount) 
{
	// weights
	__declspec(align(8)) __int64 cw;
	__declspec(align(8)) __int64 ow;
	// round masks
	__declspec(align(8)) const __int64 r7 = 0x0040004000400040;
	// edge masks
	__declspec(align(8)) const __int64 leftmask  = 0x00000000000000FF;
	__declspec(align(8)) const __int64 rightmask = 0xFF00000000000000;

	__asm { 
		// signed word center weight ((amount+0x100)>>9) x4
		mov		eax,amount
		add		eax,100h
		sar		eax,9
		lea		edx,cw
		mov		[edx],ax
		mov		[edx+2],ax
		mov		[edx+4],ax
		mov		[edx+6],ax
		// signed word outer weight 64-((amount+0x100)>>9) x4
		mov		ax,40h
		sub		ax,[edx]
		lea		edx,ow
		mov		[edx],ax
		mov		[edx+2],ax
		mov		[edx+4],ax
		mov		[edx+6],ax
	}

	for (int y=0;y<height;y++) {

	__asm {
		mov			ecx,p
		mov			edi,pitch
		add			edi,ecx
		mov			p,edi			; p += pitch;

		mov			edi,row_size
		test		edi,7
		jz			nopad
		mov			al,[ecx+edi-1]	; Pad edge pixel if needed
		mov			[ecx+edi],al
nopad:
		add			edi,7
		shr			edi,3

		pxor		mm0,mm0
; first row
		 movq		mm2,[ecx]		; Centre 8 pixels
		movq		mm1,leftmask	; 0x00000000000000ff
		 movq		mm3,mm2			; Duplicate for left
		pand		mm1,mm2			; Left edge pixel
		 psllq		mm3,8			; Left 7 other pixels
		dec			edi
		 por		mm1,mm3			; Left pixels, left most repeated
		jz			out_row_loop_ex	; 8 or less pixels per line

		movq		mm3,[ecx+1]		; Right 8 pixels

		scale(mm1,mm2,mm3,mm4,mm5,mm6,mm0)

		dec			edi
		jz			out_row_loop	; 9 to 16 pixels per line

		align 16
row_loop:
		movq		mm1,[ecx-1]		; Pickup left 8th before it is updated
		 movq		[ecx-8],mm4		; update current 8 pixels
		movq		mm2,[ecx]		; Centre 8 pixels
		 movq		mm3,[ecx+1]		; Right 8 pixels

		scale(mm1,mm2,mm3,mm4,mm5,mm6,mm0)

		dec			edi
		jnz			row_loop		; more ?

out_row_loop:
		movq		mm1,[ecx-1]		; Pickup left 8th before it is updated
		 movq		[ecx-8],mm4		; update current 8 pixels
		movq		mm2,[ecx]		; Centre 8 pixels
out_row_loop_ex:
		 movq		mm3,rightmask	; 0xFF00000000000000
		movq		mm4,mm2			; Duplicate for right
		 pand		mm3,mm2			; Right edge pixel
		psrlq		mm4,8			; Right 7 other pixels
		por			mm3,mm4			; Right pixles, right most repeated

		scale(mm1,mm2,mm3,mm4,mm5,mm6,mm0)

		movq		[ecx-8],mm4		; update current 8 pixels
		}
	}
	__asm emms
}

#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE


/************************************************
 *******   Sharpen/Blur Factory Methods   *******
 ***********************************************/

AVSValue __cdecl Create_Sharpen(AVSValue args, void*, IScriptEnvironment* env) 
{
	try {	// HIDE DAMN SEH COMPILER BUG!!!
  const double amountH = args[1].AsFloat(), amountV = args[2].AsFloat(amountH);
  const bool mmx = args[3].AsBool(true) && (env->GetCPUFlags() & CPUF_MMX);

  if (amountH < -1.5849625 || amountH > 1.0 || amountV < -1.5849625 || amountV > 1.0) // log2(3)
    env->ThrowError("Sharpen: arguments must be in the range -1.58 to 1.0");

  if (fabs(amountH) < 0.00002201361136) { // log2(1+1/65536)
    if (fabs(amountV) < 0.00002201361136) {
      return args[0].AsClip();
    }
    else {
      return new AdjustFocusV(amountV, args[0].AsClip(), mmx);
    }
  }
  else {
    if (fabs(amountV) < 0.00002201361136) {
      return new AdjustFocusH(amountH, args[0].AsClip(), mmx);
    }
    else {
      return new AdjustFocusH(amountH, new AdjustFocusV(amountV, args[0].AsClip(), mmx), mmx);
    }
  }
	}
	catch (...) { throw; }
}

AVSValue __cdecl Create_Blur(AVSValue args, void*, IScriptEnvironment* env) 
{
	try {	// HIDE DAMN SEH COMPILER BUG!!!
  const double amountH = args[1].AsFloat(), amountV = args[2].AsFloat(amountH);
  const bool mmx = args[3].AsBool(true) && (env->GetCPUFlags() & CPUF_MMX);

  if (amountH < -1.0 || amountH > 1.5849625 || amountV < -1.0 || amountV > 1.5849625) // log2(3)
    env->ThrowError("Blur: arguments must be in the range -1.0 to 1.58");

  if (fabs(amountH) < 0.00002201361136) { // log2(1+1/65536)
    if (fabs(amountV) < 0.00002201361136) {
      return args[0].AsClip();
    }
    else {
      return new AdjustFocusV(-amountV, args[0].AsClip(), mmx);
    }
  }
  else {
    if (fabs(amountV) < 0.00002201361136) {
      return new AdjustFocusH(-amountH, args[0].AsClip(), mmx);
    }
    else {
      return new AdjustFocusH(-amountH, new AdjustFocusV(-amountV, args[0].AsClip(), mmx), mmx);
    }
  }
	}
	catch (...) { throw; }
}


#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE


/***************************
 ****  TemporalSoften  *****
 **************************/

 /**
 *  YV12 / YUY2 code (c) by Klaus Post // sh0dan 2002 - 2003
 *
 * - All frames are loaded (we rely on the cache for this, which is why it has to be rewritten)
 * - Pointer array is given to the mmx function for planes.
 * - One line of each planes is one after one compared to the current frame. 
 * - Accumulated values are stored in an arrays. (as shorts)
 * - The divisor is stored in a separate array (as bytes)
 * - The divisor is looked up.
 * - Result is stored.
 * Mode 2:
 * - Works by storing the pixels of the original plane, 
 *     when pixels of the tested frames are larger than threshold.
 **/



TemporalSoften::TemporalSoften( PClip _child, unsigned radius, unsigned luma_thresh, 
                                unsigned chroma_thresh, int _scenechange, int _mode, IScriptEnvironment* env )
  : GenericVideoFilter  (_child),
    chroma_threshold    (std::min(chroma_thresh,(unsigned int)255)),
    luma_threshold      (std::min(luma_thresh,(unsigned int)255)),
    kernel              (2*std::min(radius,(unsigned int)MAX_RADIUS)+1),
    scaletab_MMX        (NULL),
    scenechange (_scenechange),
    mode(_mode)
{

  child->SetCacheHints(CACHE_RANGE,kernel);

  if (vi.IsRGB24()) {
    env->ThrowError("TemporalSoften: RGB24 Not supported, use ConvertToRGB32().");
  }

  if ((vi.IsRGB32()) && (vi.width&1)) {
    env->ThrowError("TemporalSoften: RGB32 source must be multiple of 2 in width.");
  }

  if ((vi.IsYUY2()) && (vi.width&3)) {
    env->ThrowError("TemporalSoften: YUY2 source must be multiple of 4 in width.");
  }

  if (mode!=std::max(std::min(mode,2),1)) {
    env->ThrowError("TemporalSoften: Mode must be 1 or 2.");
  }

  if (scenechange >= 255) {
    scenechange = 0;
  }
  if (scenechange>0) {
    if (!(env->GetCPUFlags() & CPUF_INTEGER_SSE))
      env->ThrowError("TemporalSoften: Scenechange requires Integer SSE capable CPU.");
    if (vi.IsRGB32()) {
      env->ThrowError("TemporalSoften: Scenechange not available on RGB32");
    }
  }

  scenechange *= ((vi.width/32)*32)*vi.height*vi.BytesFromPixels(1);
  
  planes = new int[8];
  planeP = new const BYTE*[16];
  planeP2 = new const BYTE*[16];
  planePitch = new int[16];
  planePitch2 = new int[16];
  planeDisabled = new bool[16];
  divtab = new int[16]; // First index = x/1
  for (int i = 0; i<16;i++)
    divtab[i] = 32768/(i+1);
  int c = 0;
  if (vi.IsPlanar()) {
    if (luma_thresh>0) {planes[c++] = PLANAR_Y; planes[c++] = luma_thresh;}
    if (chroma_thresh>0) { planes[c++] = PLANAR_V;planes[c++] =chroma_thresh; planes[c++] = PLANAR_U;planes[c++] = chroma_thresh;}
  } else if (vi.IsYUY2()) {
    planes[c++]=0;
    planes[c++]=luma_thresh|(chroma_thresh<<8);
  } else if (vi.IsRGB()) {  // For RGB We use Luma.
    planes[c++]=0;
    planes[c++]=luma_thresh|luma_thresh;
  }
  planes[c]=0;
  accum_line=(int*)_aligned_malloc(((vi.width*vi.BytesFromPixels(1)+FRAME_ALIGN-1)/8)*16,64);
  div_line=(int*)_aligned_malloc(((vi.width*vi.BytesFromPixels(1)+FRAME_ALIGN-1)/8)*16,64);
  frames = new PVideoFrame[kernel];
}



TemporalSoften::~TemporalSoften(void) 
{
    delete[] planes;
    delete[] divtab;
    delete[] planeP;
    delete[] planeP2;
    delete[] planePitch;
    delete[] planePitch2;
    delete[] planeDisabled;
    _aligned_free(accum_line);
    _aligned_free(div_line);
    delete[] frames;
}



PVideoFrame TemporalSoften::GetFrame(int n, IScriptEnvironment* env) 
{
  __int64 i64_thresholds = 0x1000010000100001i64;
  int radius = (kernel-1) / 2 ;
  int c=0;
  
  // Just skip if silly settings

  if ((!luma_threshold) && (!chroma_threshold) || (!radius))
    return child->GetFrame(n,env);
    

  for (int p=0;p<16;p++)
    planeDisabled[p]=false;

  
  for (int p=n-radius;p<=n+radius;p++) {
    frames[p+radius-n] = child->GetFrame(std::min(vi.num_frames-1,std::max(p,0)), env);
  }

  env->MakeWritable(&frames[radius]);

  do {
    int c_thresh = planes[c+1];  // Threshold for current plane.
    int d=0;
    for (int i = 0;i<radius; i++) { // Fetch all planes sequencially
      planePitch[d] = frames[i]->GetPitch(planes[c]);
      planeP[d++] = frames[i]->GetReadPtr(planes[c]);
    }

    BYTE* c_plane= frames[radius]->GetWritePtr(planes[c]);

    for (int i = 1;i<=radius;i++) { // Fetch all planes sequencially
      planePitch[d] = frames[radius+i]->GetPitch(planes[c]);
      planeP[d++] = frames[radius+i]->GetReadPtr(planes[c]);
    }
    
    int rowsize=frames[radius]->GetRowSize(planes[c]|PLANAR_ALIGNED);
    int h = frames[radius]->GetHeight(planes[c]);
    int w=frames[radius]->GetRowSize(planes[c]); // Non-aligned
    int pitch = frames[radius]->GetPitch(planes[c]);

    if (scenechange>0) {
      int d2=0;
      bool skiprest = false;
      for (int i = radius-1;i>=0;i--) { // Check frames backwards
        if ((!skiprest) && (!planeDisabled[i])) {
          int scenevalues = isse_scenechange(c_plane, planeP[i], h, frames[radius]->GetRowSize(planes[c]), pitch, planePitch[i]);
          if (scenevalues < scenechange) {
            planePitch2[d2] =  planePitch[i];
            planeP2[d2++] = planeP[i]; 
          } else {
            skiprest = true;
          }
          planeDisabled[i] = skiprest;  // Disable this frame on next plane (so that Y can affect UV)
        } else {
          planeDisabled[i] = true;
        }
      }
      skiprest = false;
      for (int i = 0;i<radius;i++) { // Check forward frames
        if ((!skiprest)  && (!planeDisabled[i+radius]) ) {   // Disable this frame on next plane (so that Y can affect UV)
          int scenevalues = isse_scenechange(c_plane, planeP[i+radius], h, frames[radius]->GetRowSize(planes[c]), pitch,  planePitch[i+radius]);
          if (scenevalues < scenechange) {
            planePitch2[d2] =  planePitch[i+radius];
            planeP2[d2++] = planeP[i+radius];
          } else {
            skiprest = true;
          }
          planeDisabled[i+radius] = skiprest;
        } else {
          planeDisabled[i+radius] = true;
        }
      }
      
      //Copy back
      for (int i=0;i<d2;i++) {
        planeP[i]=planeP2[i];
        planePitch[i]=planePitch2[i];
      }
      d = d2;
    }
    if (d<1) 
      return frames[radius];

    
    i64_thresholds = (__int64)c_thresh | (__int64)(c_thresh<<8) | (((__int64)c_thresh)<<16) | (((__int64)c_thresh)<<24);

    if (vi.IsYUY2()) { 
      i64_thresholds = (__int64)luma_threshold | (__int64)(chroma_threshold<<8) | ((__int64)(luma_threshold)<<16) | ((__int64)(chroma_threshold)<<24);
    }

    i64_thresholds |= (i64_thresholds<<32);
    int c_div = 32768/(d+1);  // We also have the tetplane included, thus d+1.
    if (c_thresh) {
      for (int y=0;y<h;y++) { // One line at the time
        if (mode == 1) {
          if ((env->GetCPUFlags() & CPUF_INTEGER_SSE)) {
            isse_accumulate_line(c_plane, planeP, d, rowsize,&i64_thresholds);
          } else {
            mmx_accumulate_line(c_plane, planeP, d, rowsize,&i64_thresholds);
          }
          short* s_accum_line = (short*)accum_line;
          BYTE* b_div_line = (BYTE*)div_line;
          for (int i=0;i<w;i++) {
            int div=divtab[b_div_line[i]];
            c_plane[i]=(div*(int)s_accum_line[i]+16384)>>15; //Todo: Attempt asm/mmx mix - maybe faster
          }
        } else {
          if ((env->GetCPUFlags() & CPUF_INTEGER_SSE)) {
            isse_accumulate_line_mode2(c_plane, planeP, d, rowsize,&i64_thresholds, c_div);
          } else {
            mmx_accumulate_line_mode2(c_plane, planeP, d, rowsize,&i64_thresholds, c_div);
          }
        }
        for (int p=0;p<d;p++)
          planeP[p] += planePitch[p];
        c_plane += pitch;
      }
    } else { // Just maintain the plane
    }
    c+=2;
  } while (planes[c]);

  return frames[radius];
}


void TemporalSoften::mmx_accumulate_line(const BYTE* c_plane, const BYTE** planeP, int planes, int rowsize, __int64* t) {
  static const __int64 indexer = 0x0101010101010101i64;
  const __int64 t2 = *t;
  int* _accum_line=accum_line;
  int* _div_line=div_line;

  __asm {
	push ebx
    mov esi,c_plane;
    xor eax,eax          // EAX will be plane offset (all planes).
    mov ecx,[_accum_line]
testplane:
    mov ebx,[rowsize]
    cmp ebx, eax
    jle outloop

    movq mm0,[esi+eax]  // Load current frame pixels
     pxor mm2,mm2        // Clear mm2
    movq mm1,mm0
     movq mm3,mm0
    PUNPCKlbw mm3,mm2    // mm0 = lower 4 pixels  (exhanging h/l in these two give funny results)
     PUNPCKhbw mm1,mm2     // mm1 = upper 4 pixels

    mov edi,[planeP];  // Adress of planeP array is now in edi
    mov ebx,[planes]   // How many planes (this will be our counter)
    movq [ecx],mm3
     pxor mm7,mm7        // Clear divisor table
    lea edi,[edi+ebx*4-4]
    movq [ecx+8],mm1
    align 16
kernel_loop:
    mov edx,[edi]
    movq mm1,[edx+eax]      // Load 8 pixels from test plane
     movq mm2,mm0
    movq mm5, mm1           // Save test plane pixels (twice for unpack)
     pxor mm4,mm4

    movq mm3,mm1            // Calc abs difference
     psubusb   mm1, mm2
    psubusb   mm2, mm3
    por       mm2, mm1               
     movq mm3,[t2]          // Using t also gives funny results
    PSUBUSB mm2,mm3
     movq mm1,mm5
    PCMPEQB mm2,mm4
    movq mm3,mm2
     movq mm6,mm2           // Store mask for accumulation
    punpckhbw mm3,mm4       // mm3 = upper 4 pixels mask
     punpcklbw mm2,mm4      // mm2 = lower 4 pixels mask
    pand mm6,[indexer]
     punpckhbw mm1,mm4       // mm1 = upper 4 pixels
    punpcklbw mm5,mm4      // mm5 = lower 4 pixels  // mm4, 
     paddb mm7,mm6
    pand  mm1,mm3
     pand  mm5,mm2          // mm2,mm3,mm6 also free
    movq mm2,[ecx]          // mm2 = lower accumulated 
     movq mm3,[ecx+8]
    paddusw mm2,mm5         // Add lower pixel values
     paddusw mm3,mm1        // Add upper pixel values
    movq [ecx],mm2
     movq [ecx+8],mm3

    sub edi,4
    dec ebx
    jnz kernel_loop
    mov edx,[_div_line]
    add eax,8   // Next 8 pixels
    movq [edx],mm7
    add ecx,16  // Next 8 accumulated pixels
    add edx,8   // Next 8 divisor pixels
    mov [_div_line],edx
    jmp testplane
outloop:
    emms
	pop ebx
  }
}


void TemporalSoften::isse_accumulate_line(const BYTE* c_plane, const BYTE** planeP, int planes, int rowsize, __int64* t) {
  static const __int64 indexer = 0x0101010101010101i64;
  const __int64 t2 = *t;
  int* _accum_line=accum_line;
  int* _div_line=div_line;

  __asm {
	push ebx
    mov esi,c_plane;
    xor eax,eax          // EAX will be plane offset (all planes).
    mov ecx,[_accum_line]
testplane:
    mov ebx,[rowsize]
    cmp ebx, eax
    jle outloop

    movq mm0,[esi+eax]  // Load current frame pixels
     pxor mm2,mm2        // Clear mm2
    movq mm1,mm0
     movq mm3,mm0
    punpcklbw mm3,mm2    // mm0 = lower 4 pixels  (exhanging h/l in these two give funny results)
     punpckhbw mm1,mm2     // mm1 = upper 4 pixels

    mov edi,[planeP];  // Adress of planeP array is now in edi
    mov ebx,[planes]   // How many planes (this will be our counter)
    movq [ecx],mm3
     pxor mm7,mm7        // Clear divisor table
    lea edi,[edi+ebx*4-4]
    movq [ecx+8],mm1
    align 16
kernel_loop:
    mov edx,[edi]
    movq mm1,[edx+eax]      // Load 8 pixels from test plane
     movq mm2,mm0
    movq mm5, mm1           // Save test plane pixels (twice for unpack)
     pxor mm4,mm4
    pmaxub mm2,mm1          // Calc abs difference
     pminub mm1,mm0
    psubusb mm2,mm1
     movq mm3,[t2]          // Using t also gives funny results
    psubusb mm2,mm3         // Subtrack threshold (unsigned, so all below threshold will give 0)
     movq mm1,mm5
    pcmpeqb mm2,mm4         // Compare values to 0
     prefetchnta [edx+eax+64]
    movq mm3,mm2
     movq mm6,mm2           // Store mask for accumulation
    punpckhbw mm3,mm4       // mm3 = upper 4 pixels mask
     punpcklbw mm2,mm4      // mm2 = lower 4 pixels mask
    pand mm6,[indexer]
     punpckhbw mm1,mm4       // mm1 = upper 4 pixels
    punpcklbw mm5,mm4      // mm5 = lower 4 pixels  // mm4, 
     paddb mm7,mm6
    pand  mm1,mm3
     pand  mm5,mm2          // mm2,mm3,mm6 also free
    movq mm2,[ecx]          // mm2 = lower accumulated 
     movq mm3,[ecx+8]
    paddusw mm2,mm5         // Add lower pixel values
     paddusw mm3,mm1        // Add upper pixel values
    movq [ecx],mm2
     movq [ecx+8],mm3

    sub edi,4
    dec ebx
    jnz kernel_loop
    mov edx,[_div_line]
    add eax,8   // Next 8 pixels
    movq [edx],mm7
    add ecx,16  // Next 8 accumulated pixels
    add edx,8   // Next 8 divisor pixels
    mov [_div_line],edx
    jmp testplane
outloop:
    emms
	pop ebx
  }
}


void TemporalSoften::isse_accumulate_line_mode2(const BYTE* c_plane, const BYTE** planeP, int planes, int rowsize, __int64* t, int div) {
  static __int64 full = 0xffffffffffffffffi64;
  static const __int64 add64 = (__int64)(16384) | ((__int64)(16384)<<32);
  const __int64 t2 = *t;
  __int64 div64 = (__int64)(div) | ((__int64)(div)<<16) | ((__int64)(div)<<32) | ((__int64)(div)<<48);
  div>>=1;

  __asm {
	push ebx
    mov esi,c_plane;
    xor eax,eax          // EAX will be plane offset (all planes).
    align 16
testplane:
    mov ebx,[rowsize]
    cmp ebx, eax
    jle outloop

    movq mm0,[esi+eax]  // Load current frame pixels
     pxor mm2,mm2        // Clear mm2
    movq mm6,mm0
     movq mm7,mm0
    punpcklbw mm6,mm2    // mm0 = lower 4 pixels  (exhanging h/l in these two give funny results)
     punpckhbw mm7,mm2     // mm1 = upper 4 pixels

    mov edi,[planeP];  // Adress of planeP array is now in edi
    mov ebx,[planes]   // How many planes (this will be our counter)
    lea edi,[edi+ebx*4-4]
    align 16
kernel_loop:
    mov edx,[edi]
    movq mm1,[edx+eax]      // Load 8 pixels from test plane
     movq mm2,mm0
    movq mm5, mm1           // Save test plane pixels (twice for unpack)
     pxor mm4,mm4
    pmaxub mm2,mm1          // Calc abs difference
     pminub mm1,mm0
    psubusb mm2,mm1         // mm2 = ads difference (packed bytes)
     movq mm3,[t2]          // Using t also gives funny results
    psubusb mm2,mm3         // Subtrack threshold (unsigned, so all below threshold will give 0)
     movq mm1,mm5
    pcmpeqb mm2,mm4         // Compare values to 0
     prefetchnta [edx+eax+64]  // it might just help - and we have an idle CPU here anyway ;)
    movq mm3,mm2
     pxor mm2,[full]       // mm2 inverse mask
    movq mm4, mm0
     pand mm5, mm3
    pand mm4,mm2
     pxor mm1,mm1
    por mm4,mm5
    movq mm5,mm4  // stall (this & below)
    punpcklbw mm4,mm1         // mm4 = lower pixels
     punpckhbw mm5,mm1        // mm5 = upper pixels
    paddusw mm6,mm4
     paddusw mm7,mm5

    sub edi,4
    dec ebx
    jnz kernel_loop
     // Multiply (or in reality divides) added values, repack and store.
    movq mm4,[add64]
    pxor mm5,mm5
     movq mm0,mm6
    movq mm1,mm6
     punpcklwd mm0,mm5         // low,low
    movq mm6,[div64]
    punpckhwd mm1,mm5         // low,high
     movq mm2,mm7
    pmaddwd mm0,mm6            // pmaddwd is used due to it's better rounding.
     punpcklwd mm2,mm5         // high,low
     movq mm3,mm7
     paddd mm0,mm4
    pmaddwd mm1,mm6
     punpckhwd mm3,mm5         // high,high
     psrld mm0,15
     paddd mm1,mm4
    pmaddwd mm2,mm6
     packssdw mm0, mm0
     psrld mm1,15
     paddd mm2,mm4
    pmaddwd mm3,mm6
     packssdw mm1, mm1
     psrld mm2,15
     paddd mm3,mm4
    psrld mm3,15
     packssdw mm2, mm2
    packssdw mm3, mm3
     packuswb mm0,mm5
    packuswb mm1,mm5
     packuswb mm2,mm5
    packuswb mm3,mm5
     pshufw mm0,mm0,11111100b
    pshufw mm1,mm1,11110011b
     pshufw mm2,mm2,11001111b
    pshufw mm3,mm3,00111111b
     por mm0,mm1
    por mm2,mm3
    por mm0,mm2
    movq [esi+eax],mm0

    add eax,8   // Next 8 pixels
    jmp testplane
outloop:
    emms
	pop ebx
  }
}

void TemporalSoften::mmx_accumulate_line_mode2(const BYTE* c_plane, const BYTE** planeP, int planes, int rowsize, __int64* t, int div) {
  static const __int64 full = 0xffffffffffffffffi64;
  static const __int64 low_ffff = 0x000000000000ffffi64;
  static const __int64 add64 = (__int64)(16384) | ((__int64)(16384)<<32);
  const __int64 t2 = *t;

  __int64 div64 = (__int64)(div) | ((__int64)(div)<<16) | ((__int64)(div)<<32) | ((__int64)(div)<<48);
  div>>=1;

  __asm {
	push ebx
    mov esi,c_plane;
    xor eax,eax          // EAX will be plane offset (all planes).
    align 16
testplane:
    mov ebx,[rowsize]
    cmp ebx, eax
    jle outloop

    movq mm0,[esi+eax]  // Load current frame pixels
     pxor mm2,mm2        // Clear mm2
    movq mm6,mm0
     movq mm7,mm0
    punpcklbw mm6,mm2    // mm0 = lower 4 pixels  (exhanging h/l in these two give funny results)
     punpckhbw mm7,mm2     // mm1 = upper 4 pixels

    mov edi,[planeP];  // Adress of planeP array is now in edi
    mov ebx,[planes]   // How many planes (this will be our counter)
    lea edi,[edi+ebx*4-4]
    align 16
kernel_loop:
    mov edx,[edi]
    movq mm1,[edx+eax]      // Load 8 pixels from test plane
     movq mm2,mm0
    movq mm5, mm1           // Save test plane pixels (twice for unpack)
     pxor mm4,mm4
    movq mm3,mm1            // Calc abs difference
     psubusb   mm1, mm2
    psubusb   mm2, mm3
    por       mm2, mm1               

    movq mm3,[t2]          // Using t also gives funny results
    PSUBUSB mm2,mm3         // Subtrack threshold (unsigned, so all below threshold will give 0)
     movq mm1,mm5
    PCMPEQB mm2,mm4         // Compare values to 0
    movq mm3,mm2
     pxor mm2,[full]       // mm2 inverse mask
    movq mm4, mm0
     pand mm5, mm3
    pand mm4,mm2
     pxor mm1,mm1
    por mm4,mm5
    movq mm5,mm4  // stall (this & below)
    punpcklbw mm4,mm1         // mm4 = lower pixels
     punpckhbw mm5,mm1        // mm5 = upper pixels
    paddusw mm6,mm4
     paddusw mm7,mm5

    sub edi,4
    dec ebx
    jnz kernel_loop
     // Multiply (or in reality divides) added values, repack and store.
    movq mm4,[add64]
    pxor mm5,mm5
     movq mm0,mm6
    movq mm1,mm6
     punpcklwd mm0,mm5         // low,low
    movq mm6,[div64]
    punpckhwd mm1,mm5         // low,high
     movq mm2,mm7
    pmaddwd mm0,mm6
     punpcklwd mm2,mm5         // high,low
     movq mm3,mm7
     paddd mm0,mm4
    pmaddwd mm1,mm6
     punpckhwd mm3,mm5         // high,high
     psrld mm0,15
     paddd mm1,mm4
    pmaddwd mm2,mm6
     packssdw mm0, mm0
     psrld mm1,15
     paddd mm2,mm4
    pmaddwd mm3,mm6
     packssdw mm1, mm1
     psrld mm2,15
     paddd mm3,mm4
    psrld mm3,15
     packssdw mm2, mm2
    packssdw mm3, mm3
     packuswb mm0,mm5
    packuswb mm1,mm5
     packuswb mm2,mm5
    packuswb mm3,mm5
     movq mm4, [low_ffff]
    pand mm0, mm4;
     pand mm1, mm4;
    pand mm2, mm4;
     psllq mm1, 16
    psllq mm2, 32
     por mm0,mm1
    psllq mm3, 48
    por mm2,mm3
    por mm0,mm2
    movq [esi+eax],mm0

    add eax,8   // Next 8 pixels
    jmp testplane
outloop:
    emms
	pop ebx
  }
}


int TemporalSoften::isse_scenechange(const BYTE* c_plane, const BYTE* tplane, int height, int width, int c_pitch, int t_pitch) {
  __declspec(align(8)) static __int64 full = 0xffffffffffffffffi64;
  int wp=(width/32)*32;
  int hp=height;
  int returnvalue=0xbadbad00;
  __asm {
	push ebx
    xor ebx,ebx     // Height
    pxor mm5,mm5  // Maximum difference
    mov edx, c_pitch    //copy pitch
    mov ecx, t_pitch    //copy pitch
    pxor mm6,mm6   // We maintain two sums, for better pairablility
    pxor mm7,mm7
    mov esi, c_plane
    mov edi, tplane
    jmp yloopover
    align 16
yloop:
    inc ebx
    add edi,ecx     // add pitch to both planes
    add esi,edx
yloopover:
    cmp ebx,[hp]
    jge endframe
    xor eax,eax     // Width
    align 16
xloop:
    cmp eax,[wp]    
    jge yloop

    movq mm0,[esi+eax]
     movq mm2,[esi+eax+8]
    movq mm1,[edi+eax]
     movq mm3,[edi+eax+8]
    psadbw mm0,mm1    // Sum of absolute difference
     psadbw mm2,mm3
    paddd mm6,mm0     // Add...
     paddd mm7,mm2
    movq mm0,[esi+eax+16]
     movq mm2,[esi+eax+24]
    movq mm1,[edi+eax+16]
     movq mm3,[edi+eax+24]
    psadbw mm0,mm1
     psadbw mm2,mm3
    paddd mm6,mm0
     paddd mm7,mm2

    add eax,32
    jmp xloop
endframe:
    paddd mm7,mm6
    movd returnvalue,mm7
    emms
	pop ebx
  }
  return returnvalue;
}

AVSValue __cdecl TemporalSoften::Create(AVSValue args, void*, IScriptEnvironment* env) 
{
  return new TemporalSoften( args[0].AsClip(), args[1].AsInt(), args[2].AsInt(), 
                             args[3].AsInt(), args[4].AsInt(0),args[5].AsInt(1),env );
}


#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE





/****************************
 ****  Spatial Soften   *****
 ***************************/

SpatialSoften::SpatialSoften( PClip _child, int _radius, unsigned _luma_threshold, 
                              unsigned _chroma_threshold, IScriptEnvironment* env )
  : GenericVideoFilter(_child),
    luma_threshold(_luma_threshold), chroma_threshold(_chroma_threshold), diameter(_radius*2+1)
{
  if (!vi.IsYUY2())
    env->ThrowError("SpatialSoften: requires YUY2 input");
}


PVideoFrame SpatialSoften::GetFrame(int n, IScriptEnvironment* env) 
{
  PVideoFrame src = child->GetFrame(n, env);
  PVideoFrame dst = env->NewVideoFrame(vi);

  const BYTE* srcp = src->GetReadPtr();
  BYTE* dstp = dst->GetWritePtr();
  int src_pitch = src->GetPitch();
  int dst_pitch = dst->GetPitch();
  int row_size = src->GetRowSize();

  for (int y=0; y<vi.height; ++y) 
  {
    const BYTE* line[65];    // better not make diameter bigger than this...
    for (int h=0; h<diameter; ++h)
      line[h] = &srcp[src_pitch * std::min(std::max(y+h-(diameter>>1), 0), vi.height-1)];
    int x;

    int edge = (diameter+1) & -4;
    for (x=0; x<edge; ++x)  // diameter-1 == (diameter>>1) * 2
      dstp[y*dst_pitch + x] = srcp[y*src_pitch + x];
    for (; x < row_size - edge; x+=2) 
    {
      int cnt=0, _y=0, _u=0, _v=0;
      int xx = x | 3;
      int Y = srcp[y*src_pitch + x], U = srcp[y*src_pitch + xx - 2], V = srcp[y*src_pitch + xx];
      for (int h=0; h<diameter; ++h) 
      {
        for (int w = -diameter+1; w < diameter; w += 2) 
        {
          int xw = (x+w) | 3;
          if (IsClose(line[h][x+w], Y, luma_threshold) && IsClose(line[h][xw-2], U,
                      chroma_threshold) && IsClose(line[h][xw], V, chroma_threshold)) 
          {
            ++cnt; _y += line[h][x+w]; _u += line[h][xw-2]; _v += line[h][xw];
          }
        }
      }
      dstp[y*dst_pitch + x] = (_y + (cnt>>1)) / cnt;
      if (!(x&3)) {
        dstp[y*dst_pitch + x+1] = (_u + (cnt>>1)) / cnt;
        dstp[y*dst_pitch + x+3] = (_v + (cnt>>1)) / cnt;
      }
    }
    for (; x<row_size; ++x)
      dstp[y*dst_pitch + x] = srcp[y*src_pitch + x];
  }

  return dst;
}


AVSValue __cdecl SpatialSoften::Create(AVSValue args, void*, IScriptEnvironment* env)
{
  return new SpatialSoften( args[0].AsClip(), args[1].AsInt(), args[2].AsInt(), 
                            args[3].AsInt(), env );
}

}; // namespace avxsynth
