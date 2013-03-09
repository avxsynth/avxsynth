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

#include "misc.h"


namespace avxsynth {




/********************************************************************
***** Declare index of new filters for Avisynth's filter engine *****
********************************************************************/

AVSFunction Misc_filters[] = {
  { "FixLuminance", "cif", FixLuminance::Create },    // clip, intercept, slope
  { "FixBrokenChromaUpsampling", "c", FixBrokenChromaUpsampling::Create },
  { "PeculiarBlend", "ci", PeculiarBlend::Create },   // clip, cutoff    
  { 0,0,0 }
};







/********************************
 *******   Fix Luminance   ******
 ********************************/

FixLuminance::FixLuminance(PClip _child, int _vertex, int _slope, IScriptEnvironment* env)
 : GenericVideoFilter(_child), vertex(_vertex), slope(_slope)
{
  if (!vi.IsYUY2())
    env->ThrowError("FixLuminance: requires YUY2 input");
}


PVideoFrame FixLuminance::GetFrame(int n, IScriptEnvironment* env) 
{
  PVideoFrame frame = child->GetFrame(n, env);
  env->MakeWritable(&frame);
  BYTE* p = frame->GetWritePtr();
  const int pitch = frame->GetPitch();
  for (int y=0; y<=vertex-slope/16; ++y) 
  {
    const int subtract = (vertex-y)*16/slope;
    for (int x=0; x<vi.width; ++x)
      p[x*2] = std::max(0, p[x*2]-subtract);
    p += pitch;
  }
  return frame;
}


AVSValue __cdecl FixLuminance::Create(AVSValue args, void*, IScriptEnvironment* env)
{
  return new FixLuminance(args[0].AsClip(), args[1].AsInt(), int(args[2].AsFloat()*16), env);
}






/***********************************************
 *******   Fix Broken Chroma Upsampling   ******
 ***********************************************/

FixBrokenChromaUpsampling::FixBrokenChromaUpsampling(PClip _clip, IScriptEnvironment* env) 
  : GenericVideoFilter(_clip) 
{
  if (!vi.IsYUY2())
    env->ThrowError("FixBrokenChromaUpsampling: requires YUY2 input");
}


PVideoFrame __stdcall FixBrokenChromaUpsampling::GetFrame(int n, IScriptEnvironment* env) 
{
  PVideoFrame frame = child->GetFrame(n, env);
  env->MakeWritable(&frame);
  const int pitch = frame->GetPitch();
  BYTE* p = frame->GetWritePtr() + pitch;
  for (int y = (frame->GetHeight()+1)/4; y > 0; --y) {
    for (int x = 0; x < frame->GetRowSize(); x += 4) {
      BYTE t1 = p[x+1], t3 = p[x+3];
      p[x+1] = p[pitch+x+1]; p[x+3] = p[pitch+x+3];
      p[pitch+x+1] = t1; p[pitch+x+3] = t3;
    }
    p += pitch*4;
  }
  return frame;
}


AVSValue __cdecl FixBrokenChromaUpsampling::Create( AVSValue args, void*, 
                                                    IScriptEnvironment* env ) 
{
  return new FixBrokenChromaUpsampling(args[0].AsClip(), env);
}






/*********************************
 *******   Peculiar Blend   ******
 ********************************/

PeculiarBlend::PeculiarBlend(PClip _child, int _cutoff, IScriptEnvironment* env)
 : GenericVideoFilter(_child), cutoff(_cutoff)
{
  if (!vi.IsYUY2())
    env->ThrowError("PeculiarBlend: requires YUY2 input");
}


PVideoFrame PeculiarBlend::GetFrame(int n, IScriptEnvironment* env) {
  PVideoFrame a = child->GetFrame(n, env);
  PVideoFrame b = child->GetFrame(n+1, env);
  env->MakeWritable(&a);
  BYTE* main = a->GetWritePtr();
  const BYTE* other = b->GetReadPtr();
  const int main_pitch = a->GetPitch();
  const int other_pitch = b->GetPitch();
  const int row_size = a->GetRowSize();

  if (cutoff-31 > 0) {
    int copy_top = std::min(cutoff-31, vi.height);
    BitBlt(main, main_pitch, other, other_pitch, row_size, copy_top);
    main += main_pitch * copy_top;
    other += other_pitch * copy_top;
  }
  for (int y = std::max(0, cutoff-31); y < std::min(cutoff, vi.height-1); ++y) {
    int scale = cutoff - y;
    for (int x = 0; x < row_size; ++x)
      main[x] += ((other[x] - main[x]) * scale + 16) >> 5;
    main += main_pitch;
    other += other_pitch;
  }

  return a;
}


AVSValue __cdecl PeculiarBlend::Create(AVSValue args, void*, IScriptEnvironment* env) 
{
  return new PeculiarBlend(args[0].AsClip(), args[1].AsInt(), env);
}

}; // namespace avxsynth

