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

#include "common/include/stdafx.h"

#include "turn.h"

namespace avxsynth {


AVSFunction Turn_filters[] = {
  { "TurnLeft","c",Turn::Create_TurnLeft },
  { "TurnRight","c",Turn::Create_TurnRight },
  { "Turn180","c",Turn::Create_Turn180 },
  { 0 }
};






PVideoFrame __stdcall Turn::GetFrame(int n, IScriptEnvironment* env) {

	PVideoFrame src = child->GetFrame(n, env);

    PVideoFrame dst = env->NewVideoFrame(vi);

	if (vi.IsPlanar())
		TurnPlanFunc(src->GetReadPtr(PLANAR_Y), dst->GetWritePtr(PLANAR_Y),
					 src->GetReadPtr(PLANAR_U), dst->GetWritePtr(PLANAR_U),
					 src->GetReadPtr(PLANAR_V), dst->GetWritePtr(PLANAR_V),
					 src->GetRowSize(PLANAR_Y), src->GetHeight(PLANAR_Y),
					 src->GetRowSize(PLANAR_U), src->GetHeight(PLANAR_U),
					 src->GetPitch(PLANAR_Y), dst->GetPitch(PLANAR_Y),
					 src->GetPitch(PLANAR_U), dst->GetPitch(PLANAR_U),
					 src->GetPitch(PLANAR_V), direction);
	else
		TurnFunc(src->GetReadPtr(),dst->GetWritePtr(),src->GetRowSize(),
				 src->GetHeight(),src->GetPitch(),dst->GetPitch(),direction);
    return dst;
}


AVSValue __cdecl Turn::Create_TurnLeft(AVSValue args, void* user_data, IScriptEnvironment* env) {
	return new Turn(args[0].AsClip(),-1, env);
}

AVSValue __cdecl Turn::Create_TurnRight(AVSValue args, void* user_data, IScriptEnvironment* env) {
	return new Turn(args[0].AsClip(),+1, env);
}

AVSValue __cdecl Turn::Create_Turn180(AVSValue args, void* user_data, IScriptEnvironment* env) {
	return new Turn(args[0].AsClip(),0, env);
}

}; // namespace avxsynth


