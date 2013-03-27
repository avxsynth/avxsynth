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

#include "edit.h"

namespace avxsynth {

/********************************************************************
***** Declare index of new filters for Avisynth's filter engine *****
********************************************************************/

AVSFunction Edit_filters[] = {  
  { "Trim", "cii[]b", Trim::Create },                       // first frame, last frame[, pad audio]
  { "FreezeFrame", "ciii", FreezeFrame::Create },           // first frame, last frame, source frame
  { "DeleteFrame", "ci+", DeleteFrame::Create },             // frame #
  { "DuplicateFrame", "ci+", DuplicateFrame::Create },       // frame #
  { "UnalignedSplice", "cc+", Splice::CreateUnaligned },    // clips
  { "AlignedSplice", "cc+", Splice::CreateAligned },        // clips
  { "Dissolve", "cc+i[fps]f", Dissolve::Create },           // clips, overlap frames[, fps]
  { "AudioDub", "cc", AudioDub::Create, (void*)0},          // video src, audio src
  { "AudioDubEx", "cc", AudioDub::Create, (void*)1},        // video! src, audio! src
  { "Reverse", "c", Reverse::Create },                      // plays backwards
  { "FadeOut0", "ci[color]i[fps]f", Create_FadeOut0},       // # frames[, color][, fps]
  { "FadeOut", "ci[color]i[fps]f", Create_FadeOut},         // # frames[, color][, fps]
  { "FadeOut2", "ci[color]i[fps]f", Create_FadeOut2},       // # frames[, color][, fps]
  { "FadeIn0", "ci[color]i[fps]f", Create_FadeIn0},         // # frames[, color][, fps]
  { "FadeIn", "ci[color]i[fps]f", Create_FadeIn},           // # frames[, color][, fps]
  { "FadeIn2", "ci[color]i[fps]f", Create_FadeIn2},         // # frames[, color][, fps]
  { "FadeIO0", "ci[color]i[fps]f", Create_FadeIO0},         // # frames[, color][, fps]
  { "FadeIO", "ci[color]i[fps]f", Create_FadeIO},           // # frames[, color][, fps]
  { "FadeIO2", "ci[color]i[fps]f", Create_FadeIO2},         // # frames[, color][, fps]
  { "Loop", "c[times]i[start]i[end]i", Loop::Create },      // number of loops, first frame, last frames
  { 0 }
};



 

/******************************
 *******   Trim Filter   ******
 ******************************/

Trim::Trim(int _firstframe, int _lastframe, bool _padaudio, PClip _child, IScriptEnvironment* env) : GenericVideoFilter(_child) 
{
  int lastframe;

  if (!vi.HasVideo())
    env->ThrowError("Trim: Cannot trim if there is no video.");

  firstframe = std::min(std::max(_firstframe, 0), vi.num_frames-1);

  if (_lastframe == 0)
    lastframe = vi.num_frames-1;
  else if (_lastframe < 0)
    lastframe = firstframe - _lastframe - 1;
  else
    lastframe = _lastframe;

  lastframe = std::min(std::max(lastframe, firstframe), vi.num_frames-1);
  vi.num_frames = lastframe+1 - firstframe;

  audio_offset = vi.AudioSamplesFromFrames(firstframe);
  if (_padaudio)
    vi.num_audio_samples = vi.AudioSamplesFromFrames(lastframe+1) - audio_offset;
  else {
	__int64 samples;

    if (_lastframe == 0)
      samples = vi.num_audio_samples;
	else {
	  samples = vi.AudioSamplesFromFrames(lastframe+1);
	  if (samples > vi.num_audio_samples)
		samples = vi.num_audio_samples;
	}
    if (audio_offset >= samples)
      vi.num_audio_samples = 0;
    else
      vi.num_audio_samples = samples - audio_offset;
  }
}


PVideoFrame Trim::GetFrame(int n, IScriptEnvironment* env) 
{ 
  return child->GetFrame(n+firstframe, env); 
}


void __stdcall Trim::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) 
{
  child->GetAudio(buf, start+audio_offset, count, env);
}


bool Trim::GetParity(int n) 
{ 
  return child->GetParity(n+firstframe); 
}


AVSValue __cdecl Trim::Create(AVSValue args, void*, IScriptEnvironment* env) 
{
  return new Trim(args[1].AsInt(), args[2].AsInt(), args[3].AsBool(true), args[0].AsClip(), env);
}








/*******************************
 *******   Freeze Frame   ******
 *******************************/

FreezeFrame::FreezeFrame(int _first, int _last, int _source, PClip _child)
 : GenericVideoFilter(_child), first(_first), last(_last), source(_source) {}


PVideoFrame FreezeFrame::GetFrame(int n, IScriptEnvironment* env) 
{
  return child->GetFrame((n >= first && n <= last) ? source : n, env);
}


bool FreezeFrame::GetParity(int n) 
{
  return child->GetParity((n >= first && n <= last) ? source : n);
}


AVSValue __cdecl FreezeFrame::Create(AVSValue args, void*, IScriptEnvironment* env)
{
  return new FreezeFrame(args[1].AsInt(), args[2].AsInt(), args[3].AsInt(), args[0].AsClip());
}


/******************************
 *******   Delete Frame  ******
 ******************************/

DeleteFrame::DeleteFrame(int _frame, PClip _child)
 : GenericVideoFilter(_child), frame(_frame) { --vi.num_frames; }


PVideoFrame DeleteFrame::GetFrame(int n, IScriptEnvironment* env) 
{
  return child->GetFrame(n + (n>=frame), env);
}


bool DeleteFrame::GetParity(int n) 
{ 
  return child->GetParity(n + (n>=frame)); 
}

AVSValue __cdecl DeleteFrame::Create(AVSValue args, void*, IScriptEnvironment* env) 
{
  const int n = args[1].ArraySize();
  int m = n-1;
  int *frames = new int[n];

  frames[0] = args[1][0].AsInt();
  for (int i=1; i<n; ++i) {
    frames[i] = args[1][i].AsInt();
    // Bubble insert
    for (int j=0; j<i; ++j) {
      // Remove duplicates
      if (frames[i] == frames[j]) {
        m -= 1;
        frames[i] = MAX_INT;
        break;
      }
      else if (frames[i] < frames[j]) {
        const int t = frames[j];
        frames[j] = frames[i];
        frames[i] = t;
      }
    }
  }
  PClip result = args[0].AsClip();
  for (int k=m; k>=0; --k)
    result = new DeleteFrame(frames[k], result);

  delete[] frames;
  return result;
}



/*********************************
 *******   Duplicate Frame  ******
 *********************************/

DuplicateFrame::DuplicateFrame(int _frame, PClip _child)
 : GenericVideoFilter(_child), frame(_frame) { ++vi.num_frames; }


PVideoFrame DuplicateFrame::GetFrame(int n, IScriptEnvironment* env) 
{
  return child->GetFrame(n - (n>frame), env);
}


bool DuplicateFrame::GetParity(int n) 
{ 
  return child->GetParity(n - (n>frame)); 
}


AVSValue __cdecl DuplicateFrame::Create(AVSValue args, void*, IScriptEnvironment* env) 
{
  const int n = args[1].ArraySize();
  int *frames = new int[n];

  frames[0] = args[1][0].AsInt();
  for (int i=1; i<n; ++i) {
    frames[i] = args[1][i].AsInt();
    // Bubble insert
    for (int j=0; j<i; ++j) {
      if (frames[i] < frames[j]) {
        const int t = frames[j];
        frames[j] = frames[i];
        frames[i] = t;
      }
    }
  }
  PClip result = args[0].AsClip();
  for (int k=n-1; k>=0; --k)
    result = new DuplicateFrame(frames[k], result);

  delete[] frames;
  return result;
}



/*******************************
 *******   Splice Filter  ******
 *******************************/

Splice::Splice(PClip _child1, PClip _child2, bool realign_sound, IScriptEnvironment* env)
 : GenericVideoFilter(_child1), child2(_child2)
{
  VideoInfo vi2 = child2->GetVideoInfo();

  if (vi.HasVideo() ^ vi2.HasVideo())
    env->ThrowError("Splice: one clip has video and the other doesn't (not allowed)");
  if (vi.HasAudio() ^ vi2.HasAudio())
    env->ThrowError("Splice: one clip has audio and the other doesn't (not allowed)");


  // Check video
  if (vi.HasVideo()) {
    if (vi.width != vi2.width || vi.height != vi2.height)
      env->ThrowError("Splice: Frame sizes don't match");

    if (!vi.IsSameColorspace(vi2))  
      env->ThrowError("Splice: Video formats don't match");

    double fps_v1 = (double)vi.fps_numerator / (double)vi.fps_denominator;
    double fps_v2 = (double)vi2.fps_numerator / (double)vi2.fps_denominator;
    if (fabs(fps_v1-fps_v2) > 0.000001)
      env->ThrowError("Splice: Video framerate doesn't match");
  }

  // Check Audio
  if (vi.HasAudio()) {
    // If sample types do not match they are both converted to float samples to avoid loss of precision.
    child2 = ConvertAudio::Create(child2, vi.SampleType(), SAMPLE_FLOAT);  // Clip 1 is check to be same type as clip 1, if not, convert to float samples.
    vi2 = child2->GetVideoInfo();

    child = ConvertAudio::Create(child, vi2.SampleType(), vi2.SampleType());  // Clip 1 is now be same type as clip 2.
    vi = child->GetVideoInfo();

    if (vi.AudioChannels() != vi2.AudioChannels())
      env->ThrowError("Splice: The number of audio channels doesn't match");

    if (vi.SamplesPerSecond() != vi2.SamplesPerSecond())
      env->ThrowError("Splice: The audio of the two clips have different samplerates! Use SSRC()/ResampleAudio()");
  }

  video_switchover_point = vi.num_frames;
  
  if (!video_switchover_point)  // We don't have video, so we cannot align sound to frames
    realign_sound = false;

  if (realign_sound)
    audio_switchover_point = vi.AudioSamplesFromFrames(video_switchover_point);
  else
    audio_switchover_point = vi.num_audio_samples;

  vi.num_frames += vi2.num_frames;
  vi.num_audio_samples = audio_switchover_point + vi2.num_audio_samples;
}


PVideoFrame Splice::GetFrame(int n, IScriptEnvironment* env) 
{
  if (n < video_switchover_point)
    return child->GetFrame(n, env);
  else
    return child2->GetFrame(n - video_switchover_point, env);
}


void Splice::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) 
{
  if (start+count <= audio_switchover_point)
    child->GetAudio(buf, start, count, env);
  else if (start >= audio_switchover_point)
    child2->GetAudio(buf, start - audio_switchover_point, count, env);
  else {
    const __int64 count1 = audio_switchover_point - start;
    child->GetAudio(buf, start, count1, env);
    child2->GetAudio((char*)buf+vi.BytesFromAudioSamples(count1), 0, count-count1, env);
  }
}


bool Splice::GetParity(int n) 
{
  if (n < video_switchover_point)
    return child->GetParity(n);
  else
    return child2->GetParity(n - video_switchover_point);
}



AVSValue __cdecl Splice::CreateUnaligned(AVSValue args, void*, IScriptEnvironment* env) 
{
  PClip result = args[0].AsClip();
  for (int i=0; i<args[1].ArraySize(); ++i)
    result = new Splice(result, args[1][i].AsClip(), false, env);
  return result;
}



AVSValue __cdecl Splice::CreateAligned(AVSValue args, void*, IScriptEnvironment* env) 
{
  PClip result = args[0].AsClip();
  for (int i=0; i<args[1].ArraySize(); ++i)
    result = new Splice(result, args[1][i].AsClip(), true, env);
  return result;
}













/*********************************
 *******   Dissolve Filter  ******
 *********************************/


AVSValue __cdecl Dissolve::Create(AVSValue args, void*, IScriptEnvironment* env) 
{
  const int overlap = args[2].AsInt();
  const float fps = args[3].AsFloat(24);
  PClip result = args[0].AsClip();
  for (int i=0; i < args[1].ArraySize(); ++i)
    result = new Dissolve(result, args[1][i].AsClip(), overlap, fps, env);
  return result;
}


Dissolve::Dissolve(PClip _child1, PClip _child2, int _overlap, double fps, IScriptEnvironment* env)
 : GenericVideoFilter(ConvertAudio::Create(_child1,SAMPLE_INT16|SAMPLE_FLOAT,SAMPLE_FLOAT)),
   child2(_child2),
   overlap(_overlap),
   audbuffer(0),
   audbufsize(0)
{
  VideoInfo vi2 = child2->GetVideoInfo();

  if (vi.HasVideo() ^ vi2.HasVideo())
    env->ThrowError("Dissolve: one clip has video and the other doesn't (not allowed)");
  if (vi.HasAudio() ^ vi2.HasAudio())
    env->ThrowError("Dissolve: one clip has audio and the other doesn't (not allowed)");

  if (overlap<0)
    env->ThrowError("Dissolve: Cannot dissolve if overlap is less than zero");

  if (vi.HasAudio()) {
    child2 = ConvertAudio::Create(child2, vi.SampleType(), SAMPLE_FLOAT);  // Clip 1 is check to be same type as clip 1, if not, convert to float samples.
    vi2 = child2->GetVideoInfo();

    child = ConvertAudio::Create(child, vi2.SampleType(), vi2.SampleType());  // Clip 1 is now be same type as clip 2.
    vi = child->GetVideoInfo();

    if (vi.AudioChannels() != vi2.AudioChannels())
      env->ThrowError("Dissolve: The number of audio channels doesn't match");

    if (vi.SamplesPerSecond() != vi2.SamplesPerSecond())
      env->ThrowError("Dissolve: The audio of the two clips have different samplerates! Use SSRC()/ResampleAudio()");
  }

  if (vi.HasVideo()) {
    if (vi.width != vi2.width || vi.height != vi2.height)
      env->ThrowError("Dissolve: frame sizes don't match");
    if (!(vi.IsSameColorspace(vi2)))
      env->ThrowError("Dissolve: video formats don't match");

	video_fade_start = vi.num_frames - overlap;
	video_fade_end = vi.num_frames - 1;

	audio_fade_start = vi.AudioSamplesFromFrames(video_fade_start);
	audio_fade_end = vi.AudioSamplesFromFrames(video_fade_end+1)-1;
  }
  else {
	video_fade_start = 0;
	video_fade_end = 0;

	audio_fade_start = vi.num_audio_samples - __int64(Int32x32To64(vi.SamplesPerSecond(), overlap)/fps+0.5);
	audio_fade_end = vi.num_audio_samples-1;
  }
  audio_overlap = int(audio_fade_end - audio_fade_start);

  if (video_fade_start < 0) video_fade_start= 0;
  if (audio_fade_start < 0) audio_fade_start= 0;

  vi.num_frames = video_fade_start + vi2.num_frames;
  vi.num_audio_samples = audio_fade_start + vi2.num_audio_samples;
}


bool Dissolve::GetParity(int n) 
{
  return (n < video_fade_start) ? child->GetParity(n) : child2->GetParity(n - video_fade_start);
}


PVideoFrame Dissolve::GetFrame(int n, IScriptEnvironment* env) 
{
  if (n < video_fade_start)
    return child->GetFrame(n, env);
  if (n > video_fade_end)
    return child2->GetFrame(n - video_fade_start, env);

  PVideoFrame a = child->GetFrame(n, env);
  PVideoFrame b = child2->GetFrame(n - video_fade_start, env);

  const int multiplier = n - video_fade_end + overlap;

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
  if ((env->GetCPUFlags() & CPUF_MMX) && (!(a->GetRowSize(PLANAR_Y_ALIGNED)&7)) ) {  // MMX and Video is mod 8
    int weight = (multiplier * 32767) / (overlap+1);
    int invweight = 32767-weight;
    env->MakeWritable(&a);
    mmx_weigh_plane(a->GetWritePtr(), b->GetReadPtr(), a->GetPitch(), b->GetPitch(), a->GetRowSize(PLANAR_Y_ALIGNED), a->GetHeight(), weight, invweight);
    if (vi.IsPlanar()) {
      mmx_weigh_plane(a->GetWritePtr(PLANAR_U), b->GetReadPtr(PLANAR_U), a->GetPitch(PLANAR_U), b->GetPitch(PLANAR_U), a->GetRowSize(PLANAR_U_ALIGNED), a->GetHeight(PLANAR_U), weight, invweight);
      mmx_weigh_plane(a->GetWritePtr(PLANAR_V), b->GetReadPtr(PLANAR_V), a->GetPitch(PLANAR_V), b->GetPitch(PLANAR_V), a->GetRowSize(PLANAR_V_ALIGNED), a->GetHeight(PLANAR_V), weight, invweight);    
    }
    return a;  
  }
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
  
  PVideoFrame c;
  if (!a->IsWritable())
    c = env->NewVideoFrame(vi);
  if (vi.IsPlanar()) {
    for (int i=0;i<3;i++) {
      int p = (i==0) ? PLANAR_Y : PLANAR_U;      
      p = (i==1) ? PLANAR_V : p;      
      const BYTE *src1 = a->GetReadPtr(p), *src2 = b->GetReadPtr(p);
      BYTE* dst = (c?c:a)->GetWritePtr(p);
      int src1_pitch = a->GetPitch(p), src2_pitch = b->GetPitch(p), dst_pitch = (c?c:a)->GetPitch(p);
      const int row_size = a->GetRowSize(p), height = a->GetHeight(p);

      for (int y=height; y>0; --y) {
        for (int x=0; x<row_size; ++x)
          dst[x] = src1[x] + ((src2[x]-src1[x]) * multiplier + (overlap>>1)) / (overlap+1);
        dst += dst_pitch;
        src1 += src1_pitch;
        src2 += src2_pitch;
      }
    }    
  } else {
    const BYTE *src1 = a->GetReadPtr(), *src2 = b->GetReadPtr();
    BYTE* dst = (c?c:a)->GetWritePtr();
    int src1_pitch = a->GetPitch(), src2_pitch = b->GetPitch(), dst_pitch = (c?c:a)->GetPitch();
    const int row_size = a->GetRowSize(), height = a->GetHeight();

    for (int y=height; y>0; --y) {
      for (int x=0; x<row_size; ++x)
        dst[x] = src1[x] + ((src2[x]-src1[x]) * multiplier + (overlap>>1)) / (overlap+1);
      dst += dst_pitch;
      src1 += src1_pitch;
      src2 += src2_pitch;
    }
  }

  return (c?c:a);
}



void Dissolve::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) 
{
  if (start+count <= audio_fade_start) {
    child->GetAudio(buf, start, count, env);
    return;
  }

  if (start > audio_fade_end) {
    child2->GetAudio(buf, start - audio_fade_start, count, env);
    return;
  }

  const int bytes = vi.BytesFromAudioSamples(count);
  if (audbufsize < bytes) {
    delete[] audbuffer;
    audbuffer = new BYTE[bytes];
    audbufsize = bytes;
  }
  
  child->GetAudio(buf, start, count, env);
  child2->GetAudio(audbuffer, start - audio_fade_start, count, env);
  
  const int nch = vi.AudioChannels();
  const int countXnch = count*nch;
  const int denominator = audio_overlap;
  int numerator = (audio_fade_end - start);
  int i;
  if (vi.IsSampleType(SAMPLE_INT16)) {
    short *const a = (short*)buf;
    const short *const b = (short*)audbuffer;
    for (i=0; i<countXnch; i+=nch) {
      if (numerator <= 0) {                          // Past end of dissolve
        break;
      }
      else if (numerator < denominator) {            // In dissolve region
        for (int p=0; p < nch; p++)
          a[i+p] = b[i+p] + MulDiv(a[i+p]-b[i+p], numerator, denominator);
      }
   // else                                           // Before begining of dissolve
      numerator--;
    }
    const int nchb = (countXnch - i) * sizeof(short);
    memcpy(a+i, b+i, nchb);
    return;
  } 

  if (vi.IsSampleType(SAMPLE_FLOAT)) {
    const SFLOAT frdenominator = SFLOAT(1.0/denominator);
    SFLOAT *const a = (SFLOAT*)buf;
    const SFLOAT *const b = (SFLOAT*)audbuffer;

    for (i=0; i<countXnch; i+=nch) {
      if (numerator <= 0) {                          // Past end of dissolve
        break;
      }
      else if (numerator < denominator) {            // In dissolve region
        const SFLOAT w = numerator * frdenominator;  // How far into the fade are we?
        for (int p=0; p < nch; p++)
          a[i+p] = b[i+p] + w * (a[i+p]-b[i+p]);
      }
   // else                                           // Before begining of dissolve
      numerator--; // When was a float only worked for N < 2^24
    }
    const int nchb = (countXnch - i) * sizeof(SFLOAT);
    memcpy(a+i, b+i, nchb);
    return;
  }

  env->ThrowError("Dissolve: Wow - this should never happend!");

}









/*********************************
 *******   AudioDub Filter  ******
 *********************************/

AudioDub::AudioDub(PClip child1, PClip child2, size_t mode, IScriptEnvironment* env) 
{
  const VideoInfo& vi1 = child1->GetVideoInfo();
  const VideoInfo& vi2 = child2->GetVideoInfo();
  const VideoInfo *vi_video, *vi_audio;
  if (mode) { // Unconditionally accept audio and video
	vchild = child1; achild = child2;
	vi_video = &vi1; vi_audio = &vi2;
  }
  else {
	if (vi1.HasVideo() && vi2.HasAudio()) {
	  vchild = child1; achild = child2;
	  vi_video = &vi1, vi_audio = &vi2;
	} else if (vi2.HasVideo() && vi1.HasAudio()) {
	  vchild = child2; achild = child1;
	  vi_video = &vi2, vi_audio = &vi1;
	} else {
	  env->ThrowError("AudioDub: need an audio and a video track");
	}
  }

  vi = *vi_video;
  vi.audio_samples_per_second = vi_audio->audio_samples_per_second;
  vi.num_audio_samples = vi_audio->num_audio_samples;
  vi.sample_type = vi_audio->sample_type;
  vi.nchannels = vi_audio->nchannels;
}


const VideoInfo& AudioDub::GetVideoInfo() 
{ 
  return vi; 
}


PVideoFrame AudioDub::GetFrame(int n, IScriptEnvironment* env) 
{ 
  return vchild->GetFrame(n, env); 
}


bool AudioDub::GetParity(int n) 
{ 
  return vchild->GetParity(n); 
}


void AudioDub::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) 
{
  achild->GetAudio(buf, start, count, env);
}


AVSValue __cdecl AudioDub::Create(AVSValue args, void* mode, IScriptEnvironment* env) 
{
  return new AudioDub(args[0].AsClip(), args[1].AsClip(), (size_t)mode, env);
}









/*******************************
 *******   Reverse Filter  ******
 *******************************/

Reverse::Reverse(PClip _child) : GenericVideoFilter(_child) {}


PVideoFrame Reverse::GetFrame(int n, IScriptEnvironment* env) 
{
  return child->GetFrame(vi.num_frames-n-1, env);
}


bool Reverse::GetParity(int n) 
{ 
  return child->GetParity(vi.num_frames-n-1); 
}


void Reverse::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) 
{
  child->GetAudio(buf, vi.num_audio_samples - start - count, count, env);
  int xortmp = vi.BytesPerAudioSample() - 1;
  char* buf2 = (char*)buf;
  const int count_bytes = vi.BytesFromAudioSamples(count);
  for (int i=0; i<(count_bytes>>1); ++i) {
    char temp = buf2[i]; buf2[i] = buf2[count_bytes-1-(i^xortmp)]; buf2[count_bytes-1-(i^xortmp)] = temp;
  }
}


AVSValue __cdecl Reverse::Create(AVSValue args, void*, IScriptEnvironment* env) 
{
  return new Reverse(args[0].AsClip());
}








/*****************************
 ******   Loop Filter   *******
 *****************************/

Loop::Loop(PClip _child, int times, int _start, int _end, IScriptEnvironment* env)
 : GenericVideoFilter(_child), start(_start), end(_end)
{
  start = std::min(std::max(start,0),vi.num_frames-1);
  end = std::min(std::max(end,start),vi.num_frames-1);
  frames = end-start+1;
  if (times<0) { // Loop nearly forever
    vi.num_frames = 10000000;
    end = vi.num_frames;
	if (vi.HasAudio()) {
	  if (vi.HasVideo()) {
		aud_start = vi.AudioSamplesFromFrames(start);
		aud_end = vi.AudioSamplesFromFrames(end+1) - 1; // This is the output end sample
		aud_count = vi.AudioSamplesFromFrames(frames); // length of each loop in samples
	  } else {
		// start and end frame numbers are meaningless without video
		aud_start = 0;
		aud_count = vi.num_audio_samples;
		aud_end = Int32x32To64(400000, vi.audio_samples_per_second);
	  }
	  vi.num_audio_samples = aud_end+1;
	}
  }
  else {
    vi.num_frames += (times-1) * frames;
    end = start + times * frames - 1;
	if (vi.HasAudio()) {
	  if (vi.HasVideo()) {
		aud_start = vi.AudioSamplesFromFrames(start);
		aud_end = vi.AudioSamplesFromFrames(end+1) - 1; // This is the output end sample
		aud_count = vi.AudioSamplesFromFrames(frames); // length of each loop in samples
	  } else {
		// start and end frame numbers are meaningless without video
		aud_start = 0;
		aud_count = vi.num_audio_samples;
		aud_end = vi.num_audio_samples * times;
	  }
	  vi.num_audio_samples += (times-1) * aud_count;
	}
  }
}


PVideoFrame Loop::GetFrame(int n, IScriptEnvironment* env)
{
  return child->GetFrame(convert(n), env);
}


bool Loop::GetParity(int n)
{
  return child->GetParity(convert(n));
}
 
void Loop::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) {
  __int64 get_count, get_start;
  const int bpas = vi.BytesPerAudioSample();
  char* samples = (char*)buf;

  while (count > 0) {
	if (start > aud_end) {   // tail of clip
	  get_start = aud_start + aud_count + start - (aud_end+1);
	  get_count = count;
	}
	else {
	  if (start > aud_start) // loop part of clip
		get_start = (start - aud_start) % aud_count + aud_start;
	  else                   // head of clip
		get_start = start;

	  get_count = aud_count - (get_start - aud_start); // count to end of next iteration

	  if (get_start+get_count > aud_end+1) // loop(0) case
		get_count = aud_end+1 - get_start;
	  else if (start+get_count > aud_end)
		get_count = count; // if is last iteration do all of remainder

	  if (get_count > count) get_count = count;
	}

	child->GetAudio(samples, get_start, get_count, env);

	samples += get_count * bpas;  // update dest start pointer
	start   += get_count;
	count   -= get_count;
  }
}

__inline int Loop::convert(int n)
{
  if (n>end) return n - end + start + frames - 1;
  else if (n>=start) return ((n - start) % frames) + start;
  else return n;
}


AVSValue __cdecl Loop::Create(AVSValue args, void*, IScriptEnvironment* env)
{
  if (!args[0].AsClip()->GetVideoInfo().HasVideo() &&
       (args[2].Defined() || args[3].Defined())) {
    env->ThrowError("Loop: cannot use start or end frame numbers without a video track");
  }
	return new Loop(args[0].AsClip(), args[1].AsInt(-1), args[2].AsInt(0), args[3].AsInt(10000000),env);
}







/******************************
 ** Assorted factory methods **
 *****************************/


PClip __cdecl ColorClip(PClip a, int duration, int color, float fps, IScriptEnvironment* env) {
	try {	// HIDE DAMN SEH COMPILER BUG!!!
  if (a->GetVideoInfo().HasVideo()) {
	AVSValue blackness_args[] = { a, duration, color };
	static const char* arg_names[3] = { 0, 0, "color" };
	return env->Invoke("Blackness", AVSValue(blackness_args, 3), arg_names ).AsClip();
  }
  else {
	AVSValue blackness_args[] = { a, duration, color, fps };
	static const char* arg_names[4] = { 0, 0, "color", "fps" };
	return env->Invoke("Blackness", AVSValue(blackness_args, 4), arg_names ).AsClip();
  }
	}
	catch (...) { throw; }
}

AVSValue __cdecl Create_FadeOut0(AVSValue args, void*,IScriptEnvironment* env) {
  const int duration = args[1].AsInt();
  const int fadeclr = args[2].AsInt(0);
  const float fps = args[3].AsFloat(24);
  PClip a = args[0].AsClip();
  PClip b = ColorClip(a,duration,fadeclr,fps,env);
  return new Dissolve(a, b, duration, fps, env);
}

AVSValue __cdecl Create_FadeOut(AVSValue args, void*,IScriptEnvironment* env) {
  const int duration = args[1].AsInt();
  const int fadeclr = args[2].AsInt(0);
  const float fps = args[3].AsFloat(24);
  PClip a = args[0].AsClip();
  PClip b = ColorClip(a,duration+1,fadeclr,fps,env);
  return new Dissolve(a, b, duration, fps, env);
}

AVSValue __cdecl Create_FadeOut2(AVSValue args, void*,IScriptEnvironment* env) {
  const int duration = args[1].AsInt();
  const int fadeclr = args[2].AsInt(0);
  const float fps = args[3].AsFloat(24);
  PClip a = args[0].AsClip();
  PClip b = ColorClip(a,duration+2,fadeclr,fps,env);
  return new Dissolve(a, b, duration, fps, env);
}

AVSValue __cdecl Create_FadeIn0(AVSValue args, void*,IScriptEnvironment* env) {
  const int duration = args[1].AsInt();
  const int fadeclr = args[2].AsInt(0);
  const float fps = args[3].AsFloat(24);
  PClip a = args[0].AsClip();
  PClip b = ColorClip(a,duration,fadeclr,fps,env);
  return new Dissolve(b, a, duration, fps, env);
}

AVSValue __cdecl Create_FadeIn(AVSValue args, void*,IScriptEnvironment* env) {
  const int duration = args[1].AsInt();
  const int fadeclr = args[2].AsInt(0);
  const float fps = args[3].AsFloat(24);
  PClip a = args[0].AsClip();
  PClip b = ColorClip(a,duration+1,fadeclr,fps,env);
  return new Dissolve(b, a, duration, fps, env);
}

AVSValue __cdecl Create_FadeIn2(AVSValue args, void*,IScriptEnvironment* env) {
  const int duration = args[1].AsInt();
  const int fadeclr = args[2].AsInt(0);
  const float fps = args[3].AsFloat(24);
  PClip a = args[0].AsClip();
  PClip b = ColorClip(a,duration+2,fadeclr,fps,env);
  return new Dissolve(b, a, duration, fps, env);
}

AVSValue __cdecl Create_FadeIO0(AVSValue args, void*, IScriptEnvironment* env) {
	try {	// HIDE DAMN SEH COMPILER BUG!!!
  const int duration = args[1].AsInt();
  const int fadeclr = args[2].AsInt(0);
  const float fps = args[3].AsFloat(24);
  PClip a = args[0].AsClip();
  PClip b = ColorClip(a,duration,fadeclr,fps,env);
  AVSValue dissolve_args[] = { b, a, b, duration, fps };
  return env->Invoke("Dissolve", AVSValue(dissolve_args,5)).AsClip();
	}
	catch (...) { throw; }
}

AVSValue __cdecl Create_FadeIO(AVSValue args, void*, IScriptEnvironment* env) {
	try {	// HIDE DAMN SEH COMPILER BUG!!!
  const int duration = args[1].AsInt();
  const int fadeclr = args[2].AsInt(0);
  const float fps = args[3].AsFloat(24);
  PClip a = args[0].AsClip();
  PClip b = ColorClip(a,duration+1,fadeclr,fps,env);
  AVSValue dissolve_args[] = { b, a, b, duration, fps };
  return env->Invoke("Dissolve", AVSValue(dissolve_args,5)).AsClip();
	}
	catch (...) { throw; }
}

AVSValue __cdecl Create_FadeIO2(AVSValue args, void*, IScriptEnvironment* env) {
	try {	// HIDE DAMN SEH COMPILER BUG!!!
  const int duration = args[1].AsInt();
  const int fadeclr = args[2].AsInt(0);
  const float fps = args[3].AsFloat(24);
  PClip a = args[0].AsClip();
  PClip b = ColorClip(a,duration+2,fadeclr,fps,env);
  AVSValue dissolve_args[] = { b, a, b, duration, fps };
  return env->Invoke("Dissolve", AVSValue(dissolve_args,5)).AsClip();
	}
	catch (...) { throw; }
}





PClip new_Splice(PClip _child1, PClip _child2, bool realign_sound, IScriptEnvironment* env) 
{
  return new Splice(_child1, _child2, realign_sound, env);
}

}; // namespace avxsynth
