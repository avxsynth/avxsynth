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

/*
** Turn. version 0.1
** (c) 2003 - Ernst Pech�
**
*/
#ifndef __Turn_H__
#define __Turn_H__

#include "../internal.h"
#include "turnfunc.h"

namespace avxsynth {

class Turn : public GenericVideoFilter {
	
void (*TurnFunc) (const unsigned char *srcp, unsigned char *dstp, const int rowsize,
					const int height, const int src_pitch, const int dst_pitch,
					const int direction);
void (*TurnPlanFunc) (const unsigned char *srcp_y, unsigned char *dstp_y,
				  const unsigned char *srcp_u, unsigned char *dstp_u,
				  const unsigned char *srcp_v, unsigned char *dstp_v,
				  const int rowsize, const int height,
				  const int rowsizeUV, const int heightUV,
				  const int src_pitch_y, const int dst_pitch_y,
				  const int src_pitch_u, const int dst_pitch_uv,
				  const int src_pitch_v, const int direction);

	int			direction;

public:
    Turn(PClip _child, int _direction, IScriptEnvironment* env):GenericVideoFilter(_child){
		if (_direction) {
			const int src_height = vi.height;
			vi.height = vi.width;
			vi.width = src_height;
		}
		direction = _direction;
		
		if (vi.IsRGB())
		{
			if (vi.BitsPerPixel() == 32) TurnFunc = TurnRGB32;
			else if (vi.BitsPerPixel() == 24) TurnFunc = TurnRGB24;
			else env->ThrowError("Turn: Unsupported RGB bit depth");
		}
		else if (vi.IsYUY2())
		{
			if (vi.width%2) env->ThrowError("Turn: YUY2 data must have MOD2 height");
			TurnFunc = TurnYUY2;
		}
		else if (vi.IsPlanar())
		{
			TurnPlanFunc = TurnPlanar;
		}
	};

	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  static AVSValue __cdecl Create_TurnLeft(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue __cdecl Create_TurnRight(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue __cdecl Create_Turn180(AVSValue args, void* user_data, IScriptEnvironment* env);

};

}; // namespace avxsynth

#endif // __Turn_H__

