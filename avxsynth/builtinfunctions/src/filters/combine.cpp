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

#include "stdafx.h"

#include "combine.h"



namespace avxsynth {


/********************************************************************
***** Declare index of new filters for Avisynth's filter engine *****
********************************************************************/

AVSFunction Combine_filters[] = {
  { "StackVertical", "cc+", StackVertical::Create },
  { "StackHorizontal", "cc+", StackHorizontal::Create },
  { "ShowFiveVersions", "ccccc", ShowFiveVersions::Create },
  { "Animate", "iis.*", Animate::Create },  // start frame, end frame, filter, start-args, end-args
  { "Animate", "ciis.*", Animate::Create }, 
  { "ApplyRange", "ciis.*", Animate::Create_Range }, 
  { 0 }
};






/********************************
 *******   StackVertical   ******
 ********************************/

StackVertical::StackVertical(PClip _child1, PClip _child2, IScriptEnvironment* env)
{
  // swap the order of the parameters in RGB mode because it's upside-down
  if (_child1->GetVideoInfo().IsYUV()) {
    child1 = _child1; child2 = _child2;
  } else {
    child1 = _child2; child2 = _child1;
  }

  VideoInfo vi1 = child1->GetVideoInfo();
  VideoInfo vi2 = child2->GetVideoInfo();

  if (vi1.width != vi2.width)
    env->ThrowError("StackVertical: image widths don't match");
  if (!(vi1.IsSameColorspace(vi2)))  
    env->ThrowError("StackVertical: image formats don't match");

  vi = vi1;
  vi.height += vi2.height;
  vi.num_frames = std::max(vi1.num_frames, vi2.num_frames);
  vi.num_audio_samples = std::max(vi1.num_audio_samples, vi2.num_audio_samples);
}


PVideoFrame __stdcall StackVertical::GetFrame(int n, IScriptEnvironment* env) 
{
  PVideoFrame src1 = child1->GetFrame(n, env);
  PVideoFrame src2 = child2->GetFrame(n, env);
  PVideoFrame dst = env->NewVideoFrame(vi);
  const BYTE* src1p = src1->GetReadPtr();
  const BYTE* src1pU = src1->GetReadPtr(PLANAR_U);
  const BYTE* src1pV = src1->GetReadPtr(PLANAR_V);
  const BYTE* src2p = src2->GetReadPtr();
  const BYTE* src2pU = src2->GetReadPtr(PLANAR_U);
  const BYTE* src2pV = src2->GetReadPtr(PLANAR_V);
  BYTE* dstp = dst->GetWritePtr();
  BYTE* dstpU = dst->GetWritePtr(PLANAR_U);
  BYTE* dstpV = dst->GetWritePtr(PLANAR_V);
  const int src1_pitch = src1->GetPitch();
  const int src1_pitchUV = src1->GetPitch(PLANAR_U);
  const int src2_pitch = src2->GetPitch();
  const int src2_pitchUV = src2->GetPitch(PLANAR_U);
  const int dst_pitch = dst->GetPitch();
  const int dst_pitchUV = dst->GetPitch(PLANAR_V);
  const int src1_height = src1->GetHeight();
  const int src1_heightUV = src1->GetHeight(PLANAR_U);
  const int src2_height = src2->GetHeight();
  const int src2_heightUV = src2->GetHeight(PLANAR_U);
  const int row_size = dst->GetRowSize();
  const int row_sizeUV = dst->GetRowSize(PLANAR_U);

  BYTE* dstp2 = dstp + dst_pitch * src1->GetHeight();
  BYTE* dstp2U = dstpU + dst_pitchUV * src1->GetHeight(PLANAR_U);
  BYTE* dstp2V = dstpV + dst_pitchUV * src1->GetHeight(PLANAR_V);

  if (vi.IsPlanar())
  {
    // Copy Planar
    BitBlt(dstp, dst_pitch, src1p, src1_pitch, row_size, src1_height);
    BitBlt(dstp2, dst_pitch, src2p, src2_pitch, row_size, src2_height);

    BitBlt(dstpU, dst_pitchUV, src1pU, src1_pitchUV, row_sizeUV, src1_heightUV);
    BitBlt(dstp2U, dst_pitchUV, src2pU, src2_pitchUV, row_sizeUV, src2_heightUV);

    BitBlt(dstpV, dst_pitchUV, src1pV, src1_pitchUV, row_sizeUV, src1_heightUV);
    BitBlt(dstp2V, dst_pitchUV, src2pV, src2_pitchUV, row_sizeUV, src2_heightUV);
  } else {
    BitBlt(dstp, dst_pitch, src1p, src1_pitch, row_size, src1_height);
    BitBlt(dstp2, dst_pitch, src2p, src2_pitch, row_size, src2_height);
  }
  return dst;
}


AVSValue __cdecl StackVertical::Create(AVSValue args, void*, IScriptEnvironment* env) 
{
  PClip result = args[0].AsClip();
  for (int i=0; i<args[1].ArraySize(); ++i)
    result = new StackVertical(result, args[1][i].AsClip(), env);
  return result;
}









/**********************************
 *******   StackHorizontal   ******
 **********************************/

StackHorizontal::StackHorizontal(PClip _child1, PClip _child2, IScriptEnvironment* env)
   : child1(_child1), child2(_child2)
{
  VideoInfo vi1 = child1->GetVideoInfo();
  VideoInfo vi2 = child2->GetVideoInfo();

  if (vi1.height != vi2.height)
    env->ThrowError("StackHorizontal: image heights don't match");
  if (!(vi1.IsSameColorspace(vi2)))  
    env->ThrowError("StackHorizontal: image formats don't match");

  vi = vi1;
  vi.width += vi2.width;
  vi.num_frames = std::max(vi1.num_frames, vi2.num_frames);
  vi.num_audio_samples = std::max(vi1.num_audio_samples, vi2.num_audio_samples);
}

PVideoFrame __stdcall StackHorizontal::GetFrame(int n, IScriptEnvironment* env) 
{
  PVideoFrame src1 = child1->GetFrame(n, env);
  PVideoFrame src2 = child2->GetFrame(n, env);
  PVideoFrame dst = env->NewVideoFrame(vi);

  const BYTE* src1p = src1->GetReadPtr();
  const BYTE* src1pU = src1->GetReadPtr(PLANAR_U);
  const BYTE* src1pV = src1->GetReadPtr(PLANAR_V);

  const BYTE* src2p = src2->GetReadPtr();
  const BYTE* src2pU = src2->GetReadPtr(PLANAR_U);
  const BYTE* src2pV = src2->GetReadPtr(PLANAR_V);

  BYTE* dstp = dst->GetWritePtr();
  BYTE* dstpU = dst->GetWritePtr(PLANAR_U);
  BYTE* dstpV = dst->GetWritePtr(PLANAR_V);

  const int src1_pitch = src1->GetPitch();
  const int src2_pitch = src2->GetPitch();
  const int src1_pitchUV = src1->GetPitch(PLANAR_U);
  const int src2_pitchUV = src2->GetPitch(PLANAR_U);

  const int dst_pitch = dst->GetPitch();
  const int dst_pitchUV = dst->GetPitch(PLANAR_U);

  const int src1_row_size = src1->GetRowSize();
  const int src2_row_size = src2->GetRowSize();
  const int src1_row_sizeUV = src1->GetRowSize(PLANAR_U);
  const int src2_row_sizeUV = src2->GetRowSize(PLANAR_V);

  BYTE* dstp2 = dstp + src1_row_size;
  BYTE* dstp2U = dstpU + src1_row_sizeUV;
  BYTE* dstp2V = dstpV + src1_row_sizeUV;
  const int dst_heightUV = dst->GetHeight(PLANAR_U);

  BitBlt(dstp, dst_pitch, src1p, src1_pitch, src1_row_size, vi.height);
  BitBlt(dstp2, dst_pitch, src2p, src2_pitch, src2_row_size, vi.height);

  BitBlt(dstpU, dst_pitchUV, src1pU, src1_pitchUV, src1_row_sizeUV, dst_heightUV);
  BitBlt(dstp2U, dst_pitchUV, src2pU, src2_pitchUV, src2_row_sizeUV, dst_heightUV);

  BitBlt(dstpV, dst_pitchUV, src1pV, src1_pitchUV, src1_row_sizeUV, dst_heightUV);
  BitBlt(dstp2V, dst_pitchUV, src2pV, src2_pitchUV, src2_row_sizeUV, dst_heightUV);

  return dst;
}


AVSValue __cdecl StackHorizontal::Create(AVSValue args, void*, IScriptEnvironment* env) 
{
  PClip result = args[0].AsClip();
  for (int i=0; i<args[1].ArraySize(); ++i)
    result = new StackHorizontal(result, args[1][i].AsClip(), env);
  return result;
}







/********************************
 *******   Five Versions   ******
 ********************************/

ShowFiveVersions::ShowFiveVersions(PClip* children, IScriptEnvironment* env)
{
  for (int b=0; b<5; ++b)
    child[b] = children[b];

  vi = child[0]->GetVideoInfo();

  for (int c=1; c<5; ++c)
  {
    const VideoInfo& viprime = child[c]->GetVideoInfo();
    vi.num_frames = std::max(vi.num_frames, viprime.num_frames);
    if (vi.width != viprime.width || vi.height != viprime.height || vi.pixel_type != viprime.pixel_type)
      env->ThrowError("ShowFiveVersions: video attributes of all clips must match");
  }

  vi.width  *= 3;
  vi.height *= 2;
}

PVideoFrame __stdcall ShowFiveVersions::GetFrame(int n, IScriptEnvironment* env)
{
  PVideoFrame dst = env->NewVideoFrame(vi);
  BYTE* dstp = dst->GetWritePtr();
  BYTE* dstpU = dst->GetWritePtr(PLANAR_U);
  BYTE* dstpV = dst->GetWritePtr(PLANAR_V);
  const int dst_pitch = dst->GetPitch();
  const int dst_pitchUV = dst->GetPitch(PLANAR_U);
  const int height = dst->GetHeight()/2;
  const int heightUV = dst->GetHeight(PLANAR_U)/2;

  if (vi.IsYUV()) {
    const int wg = dst->GetRowSize()/6;
    for (int i=0; i<height; i++){
      memset(dstp + ((height+i)*dst_pitch),        128, wg);
      memset(dstp + ((height+i)*dst_pitch) + wg*5, 128, wg);
    }
    if (dst_pitchUV) {
      const int wgUV = dst->GetRowSize(PLANAR_U)/6;
      for (int i=0; i<heightUV; i++) {
        memset(dstpU + ((heightUV+i)*dst_pitchUV),          128, wgUV);
        memset(dstpU + ((heightUV+i)*dst_pitchUV) + wgUV*5, 128, wgUV);
        memset(dstpV + ((heightUV+i)*dst_pitchUV),          128, wgUV);
        memset(dstpV + ((heightUV+i)*dst_pitchUV) + wgUV*5, 128, wgUV);
      }
    }
  }
  else { // vi.IsRGB()
    const int wg = dst->GetRowSize()/6;
    for (int i=0; i<height; i++){
      memset(dstp + i*dst_pitch,        128, wg);
      memset(dstp + i*dst_pitch + wg*5, 128, wg);
    }
  }

  for (int c=0; c<5; ++c) 
  {
    PVideoFrame src = child[c]->GetFrame(n, env);
	if (vi.IsPlanar()) {
	  const BYTE* srcpY = src->GetReadPtr(PLANAR_Y);
	  const BYTE* srcpU = src->GetReadPtr(PLANAR_U);
	  const BYTE* srcpV = src->GetReadPtr(PLANAR_V);
	  const int src_pitchY  = src->GetPitch(PLANAR_Y);
	  const int src_pitchUV = src->GetPitch(PLANAR_U);
	  const int src_row_sizeY  = src->GetRowSize(PLANAR_Y);
	  const int src_row_sizeUV = src->GetRowSize(PLANAR_U);

	  // staggered arrangement
	  BYTE* dstp2  = dstp  + (c>>1) * src_row_sizeY;
	  BYTE* dstp2U = dstpU + (c>>1) * src_row_sizeUV;
	  BYTE* dstp2V = dstpV + (c>>1) * src_row_sizeUV;
	  if (c&1) {
		dstp2  += (height   * dst_pitch)   + src_row_sizeY /2;
		dstp2U += (heightUV * dst_pitchUV) + src_row_sizeUV/2;
		dstp2V += (heightUV * dst_pitchUV) + src_row_sizeUV/2;
	  }

	  BitBlt(dstp2,  dst_pitch,   srcpY, src_pitchY,  src_row_sizeY,  height);
	  BitBlt(dstp2U, dst_pitchUV, srcpU, src_pitchUV, src_row_sizeUV, heightUV);
	  BitBlt(dstp2V, dst_pitchUV, srcpV, src_pitchUV, src_row_sizeUV, heightUV);
	}
	else {
	  const BYTE* srcp = src->GetReadPtr();
	  const int src_pitch = src->GetPitch();
	  const int src_row_size = src->GetRowSize();

	  // staggered arrangement
	  BYTE* dstp2 = dstp + (c>>1) * src_row_size;
	  if ((c&1)^vi.IsRGB())
		dstp2 += (height * dst_pitch);
	  if (c&1)
		dstp2 += vi.BytesFromPixels(vi.width/6);

	  BitBlt(dstp2, dst_pitch, srcp, src_pitch, src_row_size, height);
	}
  }

  return dst;
}


AVSValue __cdecl ShowFiveVersions::Create(AVSValue args, void*, IScriptEnvironment* env)
{
  PClip children[5];
  for (int i=0; i<5; ++i)
    children[i] = args[i].AsClip();
  return new ShowFiveVersions(children, env);
}









/**************************************
 *******   Animate (Recursive)   ******
 **************************************/

Animate::Animate( PClip context, int _first, int _last, const char* _name, const AVSValue* _args_before, 
                  const AVSValue* _args_after, int _num_args, bool _range_limit, IScriptEnvironment* env )
   : first(_first), last(_last), num_args(_num_args), name(_name), range_limit(_range_limit)
{
  if (first > last) 
    env->ThrowError("Animate: final frame number must be greater than initial.");

  if (first == last && (!range_limit)) 
    env->ThrowError("Animate: final frame cannot be the same as initial frame.");

  // check that argument types match
  for (int arg=0; arg<num_args; ++arg) {
    const AVSValue& a = _args_before[arg];
    const AVSValue& b = _args_after[arg];
    if (a.IsString() && b.IsString()) {
      if (strcasecmp(a.AsString(), b.AsString()))
        env->ThrowError("Animate: string arguments must match before and after");
    }
    else if (a.IsBool() && b.IsBool()) {
      if (a.AsBool() != b.AsBool())
        env->ThrowError("Animate: boolean arguments must match before and after");
    }
    else if (a.IsFloat() && b.IsFloat()) {
      // ok; also catches other numeric types
    }
    else if (a.IsClip() && b.IsClip()) {
      // ok
    }
    else {
      env->ThrowError("Animate: must have two argument lists with matching types");
    }
  }

  // copy args, and add initial clip arg for OOP notation

  if (context)
    num_args++;

  args_before = new AVSValue[num_args*3];
  args_after = args_before + num_args;
  args_now = args_after + num_args;

  if (context) {
    args_after[0] = args_before[0] = context;
    for (int i=1; i<num_args; ++i) {
      args_before[i] = _args_before[i-1];
      args_after[i] = _args_after[i-1];
    }
  }
  else {
    for (int i=0; i<num_args; ++i) {
      args_before[i] = _args_before[i];
      args_after[i] = _args_after[i];
    }
  }

  memset(cache_stage, -1, sizeof(cache_stage));

  cache[0] = env->Invoke(name, AVSValue(args_before, num_args)).AsClip();
  cache_stage[0] = 0;
  VideoInfo vi1 = cache[0]->GetVideoInfo();

  if (range_limit) {
    VideoInfo vi = context->GetVideoInfo();

    if (vi.width != vi1.width || vi.height != vi1.height)
      env->ThrowError("ApplyRange: Filtered and unfiltered video frame sizes must match");
      
    if (!vi.IsSameColorspace(vi1)) 
      env->ThrowError("ApplyRange: Filtered and unfiltered video colorspace must match");
  }
  else {
    cache[1] = env->Invoke(name, AVSValue(args_after, num_args)).AsClip();
    cache_stage[1] = last-first;
    VideoInfo vi2 = cache[1]->GetVideoInfo();

    if (vi1.width != vi2.width || vi1.height != vi2.height)
      env->ThrowError("Animate: initial and final video frame sizes must match");
  }
}


bool __stdcall Animate::GetParity(int n) 
{
  if (range_limit) {
    if ((n<first) || (n>last)) {
      return args_after[0].AsClip()->GetParity(n);
    }
  }
  // We could go crazy here and replicate the GetFrame
  // logic and share the cache_stage but it is not
  // really worth it. Although clips that change parity
  // are supported they are very confusing.
  return cache[0]->GetParity(n);
}


PVideoFrame __stdcall Animate::GetFrame(int n, IScriptEnvironment* env) 
{
  if (range_limit) {
    if ((n<first) || (n>last)) {
      return args_after[0].AsClip()->GetFrame(n, env);
    }
    return cache[0]->GetFrame(n, env);
  }
  int stage = std::min(std::max(n, first), last) - first;
  for (int i=0; i<cache_size; ++i)
    if (cache_stage[i] == stage)
      return cache[i]->GetFrame(n, env);

  // filter not found in cache--create it
  int furthest = 0;
  for (int j=1; j<cache_size; ++j)
    if (abs(stage-cache_stage[j]) > abs(stage-cache_stage[furthest]))
      furthest = j;

  int scale = last-first;
  for (int a=0; a<num_args; ++a) {
    if (args_before[a].IsInt() && args_after[a].IsInt()) {
      args_now[a] = int((Int32x32To64(args_before[a].AsInt(), scale-stage) + Int32x32To64(args_after[a].AsInt(), stage)) / scale);
    }
    else if (args_before[a].IsFloat() && args_after[a].IsFloat()) {
      args_now[a] = float(((double)args_before[a].AsFloat()*(scale-stage) + (double)args_after[a].AsFloat()*stage) / scale);
    }
    else {
      args_now[a] = args_before[a];
    }
  }
  cache_stage[furthest] = stage;
  cache[furthest] = env->Invoke(name, AVSValue(args_now, num_args)).AsClip();
  return cache[furthest]->GetFrame(n, env);
}

void __stdcall Animate::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env)  { 
  if (range_limit) {  // Applyrange - hard switch between streams.

    const VideoInfo& vi1 = cache[0]->GetVideoInfo();
    const __int64 start_switch =  vi1.AudioSamplesFromFrames(first);
    const __int64 end_switch   =  vi1.AudioSamplesFromFrames(last+1);

    if ( (start+count <= start_switch) || (start >= end_switch) ) {
      // Everything unfiltered
      args_after[0].AsClip()->GetAudio(buf, start, count, env);
      return;
    }
    else if ( (start < start_switch) || (start+count > end_switch) ) {
      // We are at one or both switchover points

      // The bit before
      if (start_switch > start) {
	const __int64 pre_count = start_switch - start;
        args_after[0].AsClip()->GetAudio(buf, start, pre_count, env);  // UnFiltered
	start += pre_count;
	count -= pre_count;
	buf = (void*)( (BYTE*)buf + vi1.BytesFromAudioSamples(pre_count) );
      }

      // The bit in the middle
      const __int64 filt_count = (end_switch < start+count) ? (end_switch - start) : count;
      cache[0]->GetAudio(buf, start, filt_count, env);  // Filtered 
      start += filt_count;
      count -= filt_count;
      buf = (void*)( (BYTE*)buf + vi1.BytesFromAudioSamples(filt_count) );

      // The bit after
      if (count > 0) 
        args_after[0].AsClip()->GetAudio(buf, start, count, env);  // UnFiltered

      return;
    }
    // Everything filtered
  }
  cache[0]->GetAudio(buf, start, count, env);  // Filtered 
} 
  

AVSValue __cdecl Animate::Create(AVSValue args, void*, IScriptEnvironment* env) 
{
  PClip context;
  if (args[0].IsClip()) {
    context = args[0].AsClip();
    args = AVSValue(&args[1], 4);
  }
  const int first = args[0].AsInt();
  const int last = args[1].AsInt();
  const char* const name = args[2].AsString();
  int n = args[3].ArraySize();
  if (n&1)
    env->ThrowError("Animate: must have two argument lists of the same length");
  return new Animate(context, first, last, name, &args[3][0], &args[3][n>>1], n>>1, false, env);
}


AVSValue __cdecl Animate::Create_Range(AVSValue args, void*, IScriptEnvironment* env) 
{
  PClip context = args[0].AsClip();

  const int first = args[1].AsInt();
  const int last = args[2].AsInt();
  const char* const name = args[3].AsString();
  int n = args[4].ArraySize();
  return new Animate(context, first, last, name, &args[4][0], &args[4][0], n, true, env);
}

}; // namespace avxsynth

