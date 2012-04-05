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


#include <internal.h>

namespace avxsynth {
	
class AveragePlane {

public:
  static AVSValue __cdecl Create_y(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue __cdecl Create_u(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue __cdecl Create_v(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue AvgPlane(AVSValue clip, void* user_data, int plane, IScriptEnvironment* env);
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
  static unsigned int isse_average_plane(const BYTE* c_plane, int height, int width, int c_pitch);
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
  static unsigned int C_average_plane(const BYTE* c_plane, int height, int width, int c_pitch);

};

class ComparePlane {

public:
  static AVSValue CmpPlane(AVSValue clip, AVSValue clip2, void* user_data, int plane, IScriptEnvironment* env);
  static AVSValue CmpPlaneSame(AVSValue clip, void* user_data, int offset, int plane, IScriptEnvironment* env);

  static AVSValue __cdecl Create_y(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue __cdecl Create_u(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue __cdecl Create_v(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue __cdecl Create_rgb(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue Create_prev_y(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue Create_prev_u(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue Create_prev_v(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue Create_prev_rgb(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue Create_next_y(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue Create_next_u(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue Create_next_v(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue Create_next_rgb(AVSValue args, void* user_data, IScriptEnvironment* env);

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
  static unsigned int isse_scenechange_16(const BYTE* c_plane, const BYTE* tplane, int height, int width, int c_pitch, int t_pitch);
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
  static unsigned int C_scenechange_16(const BYTE* c_plane, const BYTE* tplane, int height, int width, int c_pitch, int t_pitch);
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE  
  static unsigned int isse_scenechange_rgb_16(const BYTE* c_plane, const BYTE* tplane, int height, int width, int c_pitch, int t_pitch);
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE 
  static unsigned int C_scenechange_rgb_16(const BYTE* c_plane, const BYTE* tplane, int height, int width, int c_pitch, int t_pitch);

};


class MinMaxPlane {

public:
  static AVSValue MinMax(AVSValue clip, void* user_data, float threshold, int plane, int mode, IScriptEnvironment* env);
  static AVSValue __cdecl Create_max_y(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue __cdecl Create_min_y(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue __cdecl Create_median_y(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue __cdecl Create_minmax_y(AVSValue args, void* user_data, IScriptEnvironment* env);

  static AVSValue __cdecl Create_max_u(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue __cdecl Create_min_u(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue __cdecl Create_median_u(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue __cdecl Create_minmax_u(AVSValue args, void* user_data, IScriptEnvironment* env);

  static AVSValue __cdecl Create_max_v(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue __cdecl Create_min_v(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue __cdecl Create_median_v(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue __cdecl Create_minmax_v(AVSValue args, void* user_data, IScriptEnvironment* env);


private:
  enum { MIN = 1, MAX = 2, MEDIAN = 3, MINMAX_DIFFERENCE = 4 };

};

}; // namespace avxsynth
