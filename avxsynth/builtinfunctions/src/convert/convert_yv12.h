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


#include "internal.h"

namespace avxsynth {
    
void yuy2_to_yv12(const BYTE* src, int src_rowsize, int src_pitch, 
                    BYTE* dstY, BYTE* dstU, BYTE* dstV, int dst_pitch, int dst_pitchUV,
                    int height);    

void yuy2_i_to_yv12(const BYTE* src, int src_rowsize, int src_pitch, 
                    BYTE* dstY, BYTE* dstU, BYTE* dstV, int dst_pitch, int dst_pitchUV,
                    int height);    

void yv12_to_yuy2(const BYTE* srcY, const BYTE* srcU, const BYTE* srcV, int src_rowsize, int src_pitch, int src_pitch_uv, 
                  BYTE* dst, int dst_pitch,
                  int height); 

void yv12_i_to_yuy2(const BYTE* srcY, const BYTE* srcU, const BYTE* srcV, int src_rowsize, int src_pitch, int src_pitch_uv, 
                    BYTE* dst, int dst_pitch,
                    int height); 



#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
void isse_yv12_i_to_yuy2(const BYTE* srcY, const BYTE* srcU, const BYTE* srcV, int src_rowsize, int src_pitch, int src_pitch_uv, 
                    BYTE* dst, int dst_pitch,
                    int height);


void isse_yv12_to_yuy2(const BYTE* srcY, const BYTE* srcU, const BYTE* srcV, int src_rowsize, int src_pitch, int src_pitch_uv, 
                    BYTE* dst, int dst_pitch,
                    int height);

void mmx_yv12_i_to_yuy2(const BYTE* srcY, const BYTE* srcU, const BYTE* srcV, int src_rowsize, int src_pitch, int src_pitch_uv, 
                    BYTE* dst, int dst_pitch,
                    int height);

void mmx_yv12_to_yuy2(const BYTE* srcY, const BYTE* srcU, const BYTE* srcV, int src_rowsize, int src_pitch, int src_pitch_uv, 
                    BYTE* dst, int dst_pitch,
                    int height);


void isse_yuy2_to_yv12(const BYTE* src, int src_rowsize, int src_pitch, 
                    BYTE* dstY, BYTE* dstU, BYTE* dstV, int dst_pitch, int dst_pitchUV,
                    int height);

void isse_yuy2_i_to_yv12(const BYTE* src, int src_rowsize, int src_pitch, 
                    BYTE* dstY, BYTE* dstU, BYTE* dstV, int dst_pitch, int dst_pitchUV,
                    int height);

void mmx_yuy2_to_yv12(const BYTE* src, int src_rowsize, int src_pitch, 
                    BYTE* dstY, BYTE* dstU, BYTE* dstV, int dst_pitch, int dst_pitchUV,
                    int height);

void mmx_yuy2_i_to_yv12(const BYTE* src, int src_rowsize, int src_pitch, 
                    BYTE* dstY, BYTE* dstU, BYTE* dstV, int dst_pitch, int dst_pitchUV,
                    int height);
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE

}; // namespace avxsynth
