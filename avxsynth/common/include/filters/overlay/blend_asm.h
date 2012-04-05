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

// Overlay (c) 2003, 2004 by Klaus Post

#ifndef __blend_asm_h
#define __blend_asm_h

namespace avxsynth {
	
/*******************
 * Blends two planes.
 * A weight between the two planes are given.
 * Has rather ok pairing, 
 * and has very little memory usage.
 * Processes four pixels per loop, so rowsize must be mod 4.
 * Thanks to ARDA for squeezing out a bit more performance.
 * 
 * Weights must be multipled by 32767
 * Returns the blended plane in p1;
 * (c) 2002 by sh0dan.
 ********/


void mmx_weigh_planar(BYTE *p1, const BYTE *p2, int p1_pitch, int p2_pitch,int rowsize, int height, int weight, int invweight);
 
void MMerge_MMX(unsigned char *dstp, const unsigned char *srcp,
				const unsigned char *maskp, const int dst_pitch, const int src_pitch,
				const int mask_pitch, const int row_size, const int height);

void mmx_darken_planar(BYTE *p1, BYTE *p1U, BYTE *p1V, const BYTE *p2, const BYTE *p2U, const BYTE *p2V, int p1_pitch, int p2_pitch,int rowsize, int height);

void mmx_lighten_planar(BYTE *p1, BYTE *p1U, BYTE *p1V, const BYTE *p2, const BYTE *p2U, const BYTE *p2V, int p1_pitch, int p2_pitch,int rowsize, int height);

}; // namespace avxsynth
#endif // __blend_asm_h
