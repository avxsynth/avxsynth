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

// Alignplanar
// Copyright (c) Klaus Post 2001 - 2005

/******************************
 *******   AlignPlanar   ******
 *****************************/

#include "stdafx.h"
#include "avxsynth.h"

namespace avxsynth {
	
AlignPlanar::AlignPlanar(PClip _clip) : GenericVideoFilter(_clip) {}

PVideoFrame __stdcall AlignPlanar::GetFrame(int n, IScriptEnvironment* env) {
  int plane = (env->PlanarChromaAlignment(IScriptEnvironment::PlanarChromaAlignmentTest)) ? PLANAR_U_ALIGNED : PLANAR_Y_ALIGNED;

  PVideoFrame src = child->GetFrame(n, env);

  if (!(src->GetRowSize(plane)&(FRAME_ALIGN-1)))
    return src;

  PVideoFrame dst = env->NewVideoFrame(vi);

  if ((dst->GetRowSize(PLANAR_Y_ALIGNED)&(FRAME_ALIGN-1))) 
    env->ThrowError("AlignPlanar: [internal error] Returned frame was not aligned!");

  env->BitBlt(dst->GetWritePtr(), dst->GetPitch(), src->GetReadPtr(), src->GetPitch(), src->GetRowSize(), src->GetHeight());
  env->BitBlt(dst->GetWritePtr(PLANAR_V), dst->GetPitch(PLANAR_V), src->GetReadPtr(PLANAR_V), src->GetPitch(PLANAR_V), src->GetRowSize(PLANAR_V), src->GetHeight(PLANAR_V));
  env->BitBlt(dst->GetWritePtr(PLANAR_U), dst->GetPitch(PLANAR_U), src->GetReadPtr(PLANAR_U), src->GetPitch(PLANAR_U), src->GetRowSize(PLANAR_U), src->GetHeight(PLANAR_U));

  return dst;
}


PClip AlignPlanar::Create(PClip clip) 
{
  if (!clip->GetVideoInfo().IsPlanar()) {  // If not planar, already ok.
    return clip;
  }
  else 
    return new AlignPlanar(clip);
}

}; // namespace avxsynth
