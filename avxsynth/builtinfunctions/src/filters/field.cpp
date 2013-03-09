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

#include "field.h"
#include "resample.h"


namespace avxsynth {


/********************************************************************
***** Declare index of new filters for Avisynth's filter engine *****
********************************************************************/

AVSFunction Field_filters[] = {
  { "ComplementParity", "c", ComplementParity::Create },
  { "AssumeTFF", "c", AssumeParity::Create, (void*)true },
  { "AssumeBFF", "c", AssumeParity::Create, (void*)false },
  { "AssumeFieldBased", "c", AssumeFieldBased::Create },
  { "AssumeFrameBased", "c", AssumeFrameBased::Create },
  { "SeparateFields", "c", SeparateFields::Create },
  { "Weave", "c", Create_Weave },
  { "DoubleWeave", "c", Create_DoubleWeave },
  { "Pulldown", "cii", Create_Pulldown },
  { "SelectEvery", "cii*", SelectEvery::Create },
  { "SelectEven", "c", SelectEvery::Create_SelectEven },
  { "SelectOdd", "c", SelectEvery::Create_SelectOdd },
  { "Interleave", "c+", Interleave::Create },
  { "SwapFields", "c", Create_SwapFields },
//  { "Bob", "c[b]f[c]f[height]i", Create_Bob },
  { "SelectRangeEvery", "c[every]i[length]i[offset]i[audio]b", SelectRangeEvery::Create},
  { 0 }
};





/*********************************
 *******   SeparateFields   ******
 *********************************/

SeparateFields::SeparateFields(PClip _child, IScriptEnvironment* env)
 : GenericVideoFilter(_child)
{
  if (vi.height & 1)
    env->ThrowError("SeparateFields: height must be even");
  if (vi.IsYV12() && vi.height & 3)
    env->ThrowError("SeparateFields: YV12 height must be multiple of 4");
  vi.height >>= 1;
  vi.MulDivFPS(2, 1);
  vi.num_frames *= 2;
  vi.SetFieldBased(true);
}


PVideoFrame SeparateFields::GetFrame(int n, IScriptEnvironment* env) 
{
  PVideoFrame frame = child->GetFrame(n>>1, env);
  if (vi.IsPlanar()) {
    const bool topfield = (child->GetParity(n)+n)&1;
    const int UVoffset = !topfield ? frame->GetPitch(PLANAR_U) : 0;
    const int Yoffset = !topfield ? frame->GetPitch(PLANAR_Y) : 0;
    return env->SubframePlanar(frame,Yoffset, frame->GetPitch()*2, frame->GetRowSize(), frame->GetHeight()>>1,
	                       UVoffset, UVoffset, frame->GetPitch(PLANAR_U)*2);
  }
  return env->Subframe(frame,(GetParity(n) ^ vi.IsYUY2()) * frame->GetPitch(),
                         frame->GetPitch()*2, frame->GetRowSize(), frame->GetHeight()>>1);  
}


AVSValue __cdecl SeparateFields::Create(AVSValue args, void*, IScriptEnvironment* env) 
{
  PClip clip = args[0].AsClip();
  if (clip->GetVideoInfo().IsFieldBased())
    env->ThrowError("SeparateFields: SeparateFields should be applied on frame-based material: use AssumeFrameBased() beforehand");
//    return clip;
//  else
    return new SeparateFields(clip, env);
}







/******************************
 *******   Interleave   *******
 ******************************/

Interleave::Interleave(int _num_children, const PClip* _child_array, IScriptEnvironment* env)
  : num_children(_num_children), child_array(_child_array)
{
  vi = child_array[0]->GetVideoInfo();
  vi.MulDivFPS(num_children, 1);
  vi.num_frames = (vi.num_frames - 1) * num_children + 1;
  for (int i=1; i<num_children; ++i) 
  {
    const VideoInfo& vi2 = child_array[i]->GetVideoInfo();
    if (vi.width != vi2.width || vi.height != vi2.height)
      env->ThrowError("Interleave: videos must be of the same size.");
    if (!vi.IsSameColorspace(vi2))
      env->ThrowError("Interleave: video formats don't match");
    
    vi.num_frames = std::max(vi.num_frames, (vi2.num_frames - 1) * num_children + i + 1);
  }
}

AVSValue __cdecl Interleave::Create(AVSValue args, void*, IScriptEnvironment* env) 
{
  args = args[0];
  const int num_args = args.ArraySize();
  if (num_args == 1)
    return args[0];
  PClip* child_array = new PClip[num_args];
  for (int i=0; i<num_args; ++i)
    child_array[i] = args[i].AsClip();
  return new Interleave(num_args, child_array, env);
}






/*********************************
 ********   SelectEvery    *******
 *********************************/


SelectEvery::SelectEvery(PClip _child, int _every, int _from)
 : GenericVideoFilter(_child), every(_every), from(_from)
{
  vi.MulDivFPS(1, every);
  vi.num_frames = (vi.num_frames-1-from) / every + 1;
}


AVSValue __cdecl SelectEvery::Create(AVSValue args, void*, IScriptEnvironment* env) 
{
  const int num_vals = args[2].ArraySize();
  if (num_vals <= 1)
    return new SelectEvery(args[0].AsClip(), args[1].AsInt(), num_vals>0 ? args[2][0].AsInt() : 0);
  else {
    PClip* child_array = new PClip[num_vals];
    for (int i=0; i<num_vals; ++i)
      child_array[i] = new SelectEvery(args[0].AsClip(), args[1].AsInt(), args[2][i].AsInt());
    return new Interleave(num_vals, child_array, env);
  }
}








/**************************************
 ********   DoubleWeaveFields   *******
 *************************************/

DoubleWeaveFields::DoubleWeaveFields(PClip _child)
  : GenericVideoFilter(_child) 
{
  vi.height *= 2;
  vi.SetFieldBased(false);
}


PVideoFrame DoubleWeaveFields::GetFrame(int n, IScriptEnvironment* env) 
{
  PVideoFrame a = child->GetFrame(n, env);
  PVideoFrame b = child->GetFrame(n+1, env);

  PVideoFrame result = env->NewVideoFrame(vi);

  const bool parity = child->GetParity(n);

  CopyField(result, a, parity);
  CopyField(result, b, !parity);

  return result;
}


void DoubleWeaveFields::CopyField(const PVideoFrame& dst, const PVideoFrame& src, bool parity) 
{
  const int add_pitch = dst->GetPitch() * (parity ^ vi.IsYUV());
  const int add_pitchUV = dst->GetPitch(PLANAR_U) * (parity ^ vi.IsYUV());

  BitBlt( dst->GetWritePtr()         + add_pitch,   dst->GetPitch()*2,
          src->GetReadPtr(),                        src->GetPitch(),
		  src->GetRowSize(),                        src->GetHeight() );

  BitBlt( dst->GetWritePtr(PLANAR_U) + add_pitchUV, dst->GetPitch(PLANAR_U)*2,
          src->GetReadPtr(PLANAR_U),                src->GetPitch(PLANAR_U),
		  src->GetRowSize(PLANAR_U),                src->GetHeight(PLANAR_U) );

  BitBlt( dst->GetWritePtr(PLANAR_V) + add_pitchUV, dst->GetPitch(PLANAR_V)*2,
          src->GetReadPtr(PLANAR_V),                src->GetPitch(PLANAR_V),
		  src->GetRowSize(PLANAR_V),                src->GetHeight(PLANAR_V) );
}








/**************************************
 ********   DoubleWeaveFrames   *******
 *************************************/

DoubleWeaveFrames::DoubleWeaveFrames(PClip _child) 
  : GenericVideoFilter(_child) 
{
  vi.num_frames *= 2;
  vi.MulDivFPS(2, 1);
}


PVideoFrame DoubleWeaveFrames::GetFrame(int n, IScriptEnvironment* env) 
{
  if (!(n&1)) 
  {
    return child->GetFrame(n>>1, env);
  } 
  else {
    PVideoFrame a = child->GetFrame(n>>1, env);
    PVideoFrame b = child->GetFrame((n+1)>>1, env);
    bool parity = this->GetParity(n);
//  env->MakeWritable(&a);
    if (a->IsWritable()) 
    {
      CopyAlternateLines(a, b, !parity);
      return a;
    } 
    else {
      PVideoFrame result = env->NewVideoFrame(vi);
      CopyAlternateLines(result, a, parity);
      CopyAlternateLines(result, b, !parity);
      return result;
    }
  }
}


void DoubleWeaveFrames::CopyAlternateLines(const PVideoFrame& dst, const PVideoFrame& src, bool parity) 
{
  const int src_add_pitch   = src->GetPitch()         * (parity ^ vi.IsYUV());
  const int src_add_pitchUV = src->GetPitch(PLANAR_U) * (parity ^ vi.IsYUV());

  const int dst_add_pitch   = dst->GetPitch()         * (parity ^ vi.IsYUV());
  const int dst_add_pitchUV = dst->GetPitch(PLANAR_U) * (parity ^ vi.IsYUV());
 
  BitBlt( dst->GetWritePtr()         + dst_add_pitch,   dst->GetPitch()*2,
          src->GetReadPtr()          + src_add_pitch,   src->GetPitch()*2,
          src->GetRowSize(),                            src->GetHeight()>>1 );

  BitBlt( dst->GetWritePtr(PLANAR_U) + dst_add_pitchUV, dst->GetPitch(PLANAR_U)*2,
          src->GetReadPtr(PLANAR_U)  + src_add_pitchUV, src->GetPitch(PLANAR_U)*2,
          src->GetRowSize(PLANAR_U),                    src->GetHeight(PLANAR_U)>>1 );

  BitBlt( dst->GetWritePtr(PLANAR_V) + dst_add_pitchUV, dst->GetPitch(PLANAR_V)*2,
          src->GetReadPtr(PLANAR_V)  + src_add_pitchUV, src->GetPitch(PLANAR_V)*2,
          src->GetRowSize(PLANAR_V),                    src->GetHeight(PLANAR_V)>>1 );
}







/*******************************
 ********   Bob Filter   *******
 *******************************/

Fieldwise::Fieldwise(PClip _child1, PClip _child2) 
 : GenericVideoFilter(_child1), child2(_child2) {}


PVideoFrame __stdcall Fieldwise::GetFrame(int n, IScriptEnvironment* env) 
{
  return (child->GetParity(n) ? child2 : child)->GetFrame(n, env);
}







/************************************
 ********   Factory Methods   *******
 ***********************************/

AVSValue __cdecl Create_DoubleWeave(AVSValue args, void*, IScriptEnvironment* env) 
{
  PClip clip = args[0].AsClip();
  if (clip->GetVideoInfo().IsFieldBased())
    return new DoubleWeaveFields(clip);
  else
    return new DoubleWeaveFrames(clip);
}


AVSValue __cdecl Create_Weave(AVSValue args, void*, IScriptEnvironment* env) 
{
  PClip clip = args[0].AsClip();
  if (!clip->GetVideoInfo().IsFieldBased())
    env->ThrowError("Weave: Weave should be applied on field-based material: use AssumeFieldBased() beforehand");
  return new SelectEvery(Create_DoubleWeave(args, 0, env).AsClip(), 2, 0);
}


AVSValue __cdecl Create_Pulldown(AVSValue args, void*, IScriptEnvironment* env) 
{
  PClip clip = args[0].AsClip();
  PClip* child_array = new PClip[2];
  child_array[0] = new SelectEvery(clip, 5, args[1].AsInt() % 5);
  child_array[1] = new SelectEvery(clip, 5, args[2].AsInt() % 5);
  return new AssumeFrameBased(new Interleave(2, child_array, env));
}


AVSValue __cdecl Create_SwapFields(AVSValue args, void*, IScriptEnvironment* env) 
{
  return new SelectEvery(new DoubleWeaveFields(new ComplementParity(
    new SeparateFields(args[0].AsClip(), env))), 2, 0);
}


#if 0
AVSValue __cdecl Create_Bob(AVSValue args, void*, IScriptEnvironment* env)
{
	try {	// HIDE DAMN SEH COMPILER BUG!!!
  PClip clip = args[0].AsClip();
  if (!clip->GetVideoInfo().IsFieldBased()) 
    clip = new_SeparateFields(clip, env);
  
  const VideoInfo& vi = clip->GetVideoInfo();

  const double b = args[1].AsFloat(1./3.);
  const double c = args[2].AsFloat(1./3.);
  const int new_height = args[3].AsInt(vi.height*2);
  MitchellNetravaliFilter filter(b, c);
  return new_AssumeFrameBased(new Fieldwise(new FilteredResizeV(clip, -0.25, vi.height, 
                                                                new_height, &filter, env),
                                            new FilteredResizeV(clip, +0.25, vi.height, 
                                                                new_height, &filter, env)));  
	}
	catch (...) { throw; }
}
#endif

PClip new_SeparateFields(PClip _child, IScriptEnvironment* env) 
{
  return new SeparateFields(_child, env);
}


PClip new_AssumeFrameBased(PClip _child) 
{
  return new AssumeFrameBased(_child);
}



SelectRangeEvery::SelectRangeEvery(PClip _child, int _every, int _length, int _offset, bool _audio, IScriptEnvironment* env)
    : GenericVideoFilter(_child), audio(_audio)
{
  AVSValue trimargs[3] = { _child, _offset, 0};
  PClip c = env->Invoke("Trim",AVSValue(trimargs,3)).AsClip();
  child = c;
  vi = c->GetVideoInfo();

  every = std::min(std::max(_every,1),vi.num_frames);
  length = std::min(std::max(_length,1),every);

  const int n = vi.num_frames;
  vi.num_frames = (n/every)*length+(n%every<length?n%every:length);

  if (vi.HasAudio()) {
    vi.num_audio_samples = vi.AudioSamplesFromFrames(vi.num_frames);
  }
}


PVideoFrame __stdcall SelectRangeEvery::GetFrame(int n, IScriptEnvironment* env)
{
  return child->GetFrame((n/length)*every+(n%length), env);
}


bool __stdcall SelectRangeEvery::GetParity(int n)
{
  return child->GetParity((n/length)*every+(n%length));
}


void __stdcall SelectRangeEvery::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env)
{
  if (!audio) {
    child->GetAudio(buf, start, count, env);
    return;
  }

  __int64 samples_filled = 0;
  BYTE* samples = (BYTE*)buf;
  const int bps = vi.BytesPerAudioSample();
  int startframe = vi.FramesFromAudioSamples(start);
  __int64 general_offset = start - vi.AudioSamplesFromFrames(startframe);  // General compensation for startframe rounding.

  while (samples_filled < count) {
    const int iteration = startframe / length;                    // Which iteration is this.
    const int iteration_into = startframe % length;               // How far, in frames are we into this iteration.
    const int iteration_left = length - iteration_into;           // How many frames is left of this iteration.

    const __int64 iteration_left_samples = vi.AudioSamplesFromFrames(iteration_left);
    const __int64 getsamples = std::min(iteration_left_samples, count-samples_filled);   // This is the number of samples we can get without either having to skip, or being finished.
    const __int64 start_offset = vi.AudioSamplesFromFrames(iteration * every + iteration_into) + general_offset;

    child->GetAudio(&samples[samples_filled*bps], start_offset, getsamples, env);
    samples_filled += getsamples;
    startframe = (iteration+1) * every;
    general_offset = 0; // On the following loops, general offset should be 0, as we are either skipping.
  }
}

AVSValue __cdecl SelectRangeEvery::Create(AVSValue args, void* user_data, IScriptEnvironment* env) {
    return new SelectRangeEvery(args[0].AsClip(), args[1].AsInt(1500), args[2].AsInt(50), args[3].AsInt(0), args[4].AsBool(true), env);
}

}; // namespace avxsynth
