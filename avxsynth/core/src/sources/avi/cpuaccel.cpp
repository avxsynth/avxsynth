// Avisynth v1.0 beta.  Copyright 2000 Ben Rudiak-Gould.
// http://www.math.berkeley.edu/~benrg/avisynth.html

//	VirtualDub - Video processing and capture application
//	Copyright (C) 1998-2000 Avery Lee
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include "avxplugin.h"

namespace avxsynth {
	
static long g_lCPUExtensionsAvailable;

#define CPUF_SUPPORTS_SSE                       (0x00000010L)
#define CPUF_SUPPORTS_SSE2                      (0x00000020L)

// This is ridiculous.

static long CPUCheckForSSESupport() {
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
	__try {
//		__asm andps xmm0,xmm0

		__asm _emit 0x0f
		__asm _emit 0x54
		__asm _emit 0xc0

	} __except(EXCEPTION_EXECUTE_HANDLER) {
		if (GetExceptionCode() == 0xC000001Du) // illegal instruction
			g_lCPUExtensionsAvailable &= ~(CPUF_SUPPORTS_SSE|CPUF_SUPPORTS_SSE2);
	}
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
	return g_lCPUExtensionsAvailable;
}

long CPUCheckForExtensions() {
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
	__asm {
		push	ebp
		push	edi
		push	esi
		push	ebx

		xor		ebp,ebp			//cpu flags - if we don't have CPUID, we probably
								//won't want to try FPU optimizations.

		//check for CPUID.

		pushfd					//flags -> EAX
		pop		eax
		or		eax,00200000h	//set the ID bit
		push	eax				//EAX -> flags
		popfd
		pushfd					//flags -> EAX
		pop		eax
		and		eax,00200000h	//ID bit set?
		jz		done			//nope...

		//CPUID exists, check for features register.

		mov		ebp,00000003h
		xor		eax,eax
		cpuid
		or		eax,eax
		jz		done			//no features register?!?

		//features register exists, look for MMX, SSE, SSE2.

		mov		eax,1
		cpuid
		mov		ebx,edx
		and		ebx,00800000h	//MMX is bit 23
		shr		ebx,21
		or		ebp,ebx			//set bit 2 if MMX exists

		mov		ebx,edx
		and		edx,02000000h	//SSE is bit 25
		shr		edx,25
		neg		edx
		and		edx,00000018h	//set bits 3 and 4 if SSE exists
		or		ebp,edx

		and		ebx,04000000h	//SSE2 is bit 26
		shr		ebx,21
		and		ebx,00000020h	//set bit 5
		or		ebp,ebx

		mov		ebx,ecx
		and		ebx,1 	//SSE3 is bit 0
		shl		ebx,8
		and		ebx,00000100h	//set bit 8
		or		ebp,ebx

		mov		ebx,ecx
		shr		ebx,8		//SSSE3 is bit 8
		and		ebx,1 	
		shl		ebx,9
		and		ebx,00000200h	//set bit 9
		or		ebp,ebx

		mov		ebx,ecx
		shr		ebx,18		//SSE4 is bit 18
		and		ebx,1 	
		shl		ebx,10
		and		ebx,00000400h	//set bit 10
		or		ebp,ebx

		mov		ebx,ecx
		shr		ebx,19		//SSE4.2 is bit 19
		and		ebx,1 	
		shl		ebx,11
		and		ebx,00000800h	//set bit 11
		or		ebp,ebx

		//check for vendor feature register (K6/Athlon).

		mov		eax,80000000h
		cpuid
		mov		ecx,80000001h
		cmp		eax,ecx
		jb		done

		//vendor feature register exists, look for 3DNow! and Athlon extensions

		mov		eax,ecx
		cpuid

		mov		eax,edx
		and		edx,80000000h	//3DNow! is bit 31
		shr		edx,25
		or		ebp,edx			//set bit 6

		mov		edx,eax
		and		eax,40000000h	//3DNow!2 is bit 30
		shr		eax,23
		or		ebp,eax			//set bit 7

		and		edx,00400000h	//AMD MMX extensions (integer SSE) is bit 22
		shr		edx,19
		or		ebp,edx			//set bit 3

done:
		mov		eax,ebp
		mov		g_lCPUExtensionsAvailable, ebp

		//Full SSE and SSE-2 require OS support for the xmm* registers.

		test	eax,00000030h
		jz		nocheck
		call	CPUCheckForSSESupport
nocheck:
		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret
	}
#else
    std::ifstream procInfo;
    procInfo.open("/proc/cpuinfo");
    if(procInfo.fail())
        return 0;
    std::string line;
    while(1)
    {
        std::getline(procInfo, line);
        if(std::string::npos != line.find("flags", 0))
        {
            // http://blog.bradiceanu.net/2009/07/20/linux-proccpuinfo-flags/
            g_lCPUExtensionsAvailable = 0;
            if(std::string::npos != line.find(" fpu "))
                g_lCPUExtensionsAvailable |= CPUF_FPU;
            if(std::string::npos != line.find(" mmx "))
                g_lCPUExtensionsAvailable |= CPUF_MMX;
            // CPUF_INTEGER_SSE ?
            if(std::string::npos != line.find(" sse "))
                g_lCPUExtensionsAvailable |= CPUF_SSE;
            if(std::string::npos != line.find(" sse2 "))
                g_lCPUExtensionsAvailable |= CPUF_SSE2;
            if(std::string::npos != line.find(" 3dnow "))
                g_lCPUExtensionsAvailable |= CPUF_3DNOW;
            if(std::string::npos != line.find(" 3dnowext "))
                g_lCPUExtensionsAvailable |= CPUF_3DNOW_EXT;
            //
            // the lingo of /proc/cpuinfo flags and the lingo of AviSynth differ 
            // in the way of how the X86_64 is declared
            //
            // This is what Linux convention assumes
            // if(std::string::npos != line.find(" lm "))
            //    g_lCPUExtensionsAvailable |= CPUF_X86_64;
            //
            //...and this is what AviSynth conventions assume
            // quote:
            // ====================================================================================
            // For GetCPUFlags.  These are backwards-compatible with those in VirtualDub.
            // enum {
            // ...
            // ...CPUF_X86_64       =  0xA0,    //  Hammer (note: equiv. to 3DNow + SSE2, which
                                                //          only Hammer will have anyway)
            // ...
            if((std::string::npos != line.find(" 3dnow ")) && (std::string::npos != line.find(" sse2 ")))
               g_lCPUExtensionsAvailable |= CPUF_X86_64;
            
            if(std::string::npos != line.find(" pni "))
                g_lCPUExtensionsAvailable |= CPUF_SSE3;
            break;
        }
    }
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
	return g_lCPUExtensionsAvailable;
}

}; // namespace avxsynth
