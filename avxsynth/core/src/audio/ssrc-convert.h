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

#ifndef __SSRC_Audio_H__
#define __SSRC_Audio_H__

typedef float REAL;

#include "avxsynth/common/include/internal.h"
#include "ssrc.h"

namespace avxsynth {

class SSRC : public GenericVideoFilter 
/**
  * Class to resample the audio stream
 **/
{
public:
  SSRC(PClip _child, int _target_rate, bool _fast, IScriptEnvironment* env);
  ~SSRC() {
     if (srcbuffer) delete[] srcbuffer;
    }
  void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env);
  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);

private:
  const int target_rate;
  int source_rate;
  int srcbuffer_size;
  bool skip_conversion;
  double factor;
  int input_samples;
  bool fast;

  SFLOAT* srcbuffer;
  __int64 next_sample;
  __int64 inputReadOffset;

	Resampler_base * res;

};

}; // namespace avxsynth

#endif  // __SSRC_Audio_H__

