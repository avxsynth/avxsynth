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


// Avisynth filter: Plane Swap
// by Klaus Post


#ifndef __Planeswap_H__
#define __Planeswap_H__

#include "common/include/internal.h"

namespace avxsynth {

/****************************************************
****************************************************/

class Swap : public GenericVideoFilter
/**
  * Swaps planar channels
 **/
{
public:
  Swap(PClip _child, PClip _clip, PClip _clipY, int _mode, int _test, IScriptEnvironment* env);  
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);

  static AVSValue __cdecl CreateUV(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue __cdecl CreateUToY(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue __cdecl CreateVToY(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue __cdecl CreateYToUV(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue __cdecl CreateYToYUV(AVSValue args, void* user_data, IScriptEnvironment* env);

private:
  PClip clip, clipY;
  int mode;
  int test;

  enum {SwapUV=1, UToY, VToY, YToUV};
};

}; // namespace avxsynth

#endif  // __Planeswap_H__
