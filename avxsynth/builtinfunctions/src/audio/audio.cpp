// Avisynth v2.5.  Copyright 2002 Ben Rudiak-Gould et al.
// http://www.avisynth.org

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
#include "common/include/internal.h"
#include "audio.h"

namespace avxsynth
{
    
#define BIGBUFFSIZE (2048*1024) // Use a 2Mb buffer for EnsureVBRMP3Sync seeking & Normalize scanning

/********************************************************************
***** Declare index of new filters for Avisynth's filter engine *****
********************************************************************/

AVSFunction Audio_filters[] = {
                                { "DelayAudio", "cf", DelayAudio::Create },
                                { "AmplifydB", "cf+", Amplify::Create_dB },
                                { "Amplify", "cf+", Amplify::Create },
                                { "AssumeSampleRate", "ci", AssumeRate::Create },
                                { "Normalize", "c[volume]f[show]b", Normalize::Create },
                                { "MixAudio", "cc[clip1_factor]f[clip2_factor]f", MixAudio::Create },
                                { "ResampleAudio", "ci[]i", ResampleAudio::Create },
                                { "ConvertToMono", "c", ConvertToMono::Create },
                                { "EnsureVBRMP3Sync", "c", EnsureVBRMP3Sync::Create },
                                { "MergeChannels", "c+", MergeChannels::Create },
                                { "MonoToStereo", "cc", MergeChannels::Create },
                                { "GetLeftChannel", "c", GetChannel::Create_left },
                                { "GetRightChannel", "c", GetChannel::Create_right },
                                { "GetChannel", "ci+", GetChannel::Create_n },
                                { "GetChannels", "ci+", GetChannel::Create_n },     // Alias to ease use!
                                { "KillVideo", "c", KillVideo::Create },
                                { "KillAudio", "c", KillAudio::Create },
                                { "ConvertAudioTo16bit", "c", ConvertAudio::Create_16bit },   // in convertaudio.cpp
                                { "ConvertAudioTo8bit", "c", ConvertAudio::Create_8bit },
                                { "ConvertAudioTo24bit", "c", ConvertAudio::Create_24bit },
                                { "ConvertAudioTo32bit", "c", ConvertAudio::Create_32bit },
                                { "ConvertAudioToFloat", "c", ConvertAudio::Create_float },
                                { "ConvertAudio", "cii", ConvertAudio::Create_Any }, // For plugins to Invoke()
                                { 0 }
                              };

// Note - floats should not be clipped - they will be clipped, when they are converted back to ints.
// Vdub can handle 8/16 bit, and reads 32bit, but cannot play/convert it. Floats doesn't make sense
// in AVI. So for now convert back to 16 bit always.

// Always! FIXME: Most int64's are often cropped to ints - count is ok to be int, but not start

// For plugins to env->Invoke()

AVSValue __cdecl ConvertAudio::Create_Any(AVSValue args, void*, IScriptEnvironment*) {
  return Create(args[0].AsClip(), args[1].AsInt(), args[2].AsInt());
}

// For explicit conversions

AVSValue __cdecl ConvertAudio::Create_16bit(AVSValue args, void*, IScriptEnvironment*) {
  return Create(args[0].AsClip(), SAMPLE_INT16, SAMPLE_INT16);
}

AVSValue __cdecl ConvertAudio::Create_8bit(AVSValue args, void*, IScriptEnvironment*) {
  return Create(args[0].AsClip(), SAMPLE_INT8, SAMPLE_INT8);
}


AVSValue __cdecl ConvertAudio::Create_32bit(AVSValue args, void*, IScriptEnvironment*) {
  return Create(args[0].AsClip(), SAMPLE_INT32, SAMPLE_INT32);
}

AVSValue __cdecl ConvertAudio::Create_float(AVSValue args, void*, IScriptEnvironment*) {
  return Create(args[0].AsClip(), SAMPLE_FLOAT, SAMPLE_FLOAT);
}

AVSValue __cdecl ConvertAudio::Create_24bit(AVSValue args, void*, IScriptEnvironment*) {
  return Create(args[0].AsClip(), SAMPLE_INT24, SAMPLE_INT24);
}


void FilterUD_mmx(short *Xp, unsigned Ph, int _inc, int _dhb, short *p_Imp, unsigned End);


/*************************************
 *******   Assume SampleRate  ********
 *************************************/

AssumeRate::AssumeRate(PClip _clip, int _rate)
    : GenericVideoFilter(_clip) {
  if (_rate < 0)
    _rate = 0;
  if (vi.SamplesPerSecond() == 0)  // Don't add audio if none is present.
    _rate = 0;

  vi.audio_samples_per_second = _rate;
}

AVSValue __cdecl AssumeRate::Create(AVSValue args, void*, IScriptEnvironment*) {
  return new AssumeRate(args[0].AsClip(), args[1].AsInt());
}





/******************************************
 *******   Convert Audio -> Mono     ******
 *******   Supports int16 & float    ******
 *****************************************/

ConvertToMono::ConvertToMono(PClip _clip)
    : GenericVideoFilter(ConvertAudio::Create(_clip,
                     SAMPLE_INT16 | SAMPLE_FLOAT, SAMPLE_FLOAT)) {
  channels = vi.AudioChannels();
  vi.nchannels = 1;
  tempbuffer_size = 0;
}


void __stdcall ConvertToMono::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) {
  if (tempbuffer_size) {
    if (tempbuffer_size < count) {
      delete[] tempbuffer;
      tempbuffer = new char[count * channels * vi.BytesPerChannelSample()];
      tempbuffer_size = count;
    }
  } else {
    tempbuffer = new char[count * channels * vi.BytesPerChannelSample()];
    tempbuffer_size = count;
  }

  child->GetAudio(tempbuffer, start, count, env);

  if (vi.IsSampleType(SAMPLE_INT16)) {
    signed short* samples = (signed short*)buf;
    signed short* tempsamples = (signed short*)tempbuffer;
    const int rchannels = 65536 / channels;
    
    for (int i = 0; i < count; i++) {
      int tsample = 0;
      for (int j = i*channels ; j < i*channels+channels; j++)
        tsample += tempsamples[j]; // Accumulate samples
      samples[i] = (signed short)((tsample * rchannels + 32768) >> 16); // tsample * (1/channels) + 0.5
    }
  } else if (vi.IsSampleType(SAMPLE_FLOAT)) {
    SFLOAT* samples = (SFLOAT*)buf;
    SFLOAT* tempsamples = (SFLOAT*)tempbuffer;
    const SFLOAT f_rchannels = SFLOAT(1.0 / channels);

    for (int i = 0; i < count; i++) {
      SFLOAT tsample = 0.0f;
      for (int j = i*channels ; j < i*channels+channels; j++)
        tsample += tempsamples[j]; // Accumulate samples
      samples[i] = (tsample * f_rchannels);
    }
  }
}


PClip ConvertToMono::Create(PClip clip) {
  if (!clip->GetVideoInfo().HasAudio())
    return clip;
  if (clip->GetVideoInfo().AudioChannels() == 1)
    return clip;
  else
    return new ConvertToMono(clip);
}

AVSValue __cdecl ConvertToMono::Create(AVSValue args, void*, IScriptEnvironment*) {
  return Create(args[0].AsClip());
}

/******************************************
 *******   Ensure VBR mp3 sync,      ******
 *******    by always reading audio  ******
 *******    sequencial.              ******
 *****************************************/

EnsureVBRMP3Sync::EnsureVBRMP3Sync(PClip _clip)
    : GenericVideoFilter(_clip) {
  last_end = 0;
}


void __stdcall EnsureVBRMP3Sync::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) {

  if (start != last_end) { // Reread!
    __int64 bcount = count;
    __int64 offset = 0;
    char* samples = (char*)buf;
    bool bigbuff=false;

    if (start > last_end)
      offset = last_end; // Skip forward only if the skipped to position is in front of last position.

	if ((count < start-offset) && (vi.BytesFromAudioSamples(count) < BIGBUFFSIZE)) {
	  samples = new char[BIGBUFFSIZE];
	  if (samples) {
	    bigbuff=true;
	    bcount = vi.AudioSamplesFromBytes(BIGBUFFSIZE);
	  }
	  else {
	    samples = (char*)buf; // malloc failed just reuse clients buffer
	  }
	}
    while (offset + bcount < start) { // Read whole blocks of 'bcount' samples
      child->GetAudio(samples, offset, bcount, env);
      offset += bcount;
    } // Read until 'start'
    child->GetAudio(samples, offset, start - offset, env);  // Now we're at 'start'
    offset += start - offset;
    if (bigbuff) delete[] samples;
    if (offset != start)
      env->ThrowError("EnsureVBRMP3Sync [Internal error]: Offset should be %i, but is %i", start, offset);
  }
  child->GetAudio(buf, start, count, env);
  last_end = start + count;
}


PClip EnsureVBRMP3Sync::Create(PClip clip, IScriptEnvironment* env) {
  AVSValue c = new EnsureVBRMP3Sync(clip);
  PClip c2; // TBD = Cache::Create_Cache(c, 0, env).AsClip(); // Very good idea to insert a cache here.
  c2->SetCacheHints(CACHE_AUDIO, 1024*1024);
  return c2;
}

AVSValue __cdecl EnsureVBRMP3Sync::Create(AVSValue args, void*, IScriptEnvironment* env) {
  return Create(args[0].AsClip(), env);
}


/*******************************************
 *******   Mux 'N' sources, so the      ****
 *******   total channels is the sum of ****
 *******   the channels in the 'N' clip ****
 *******************************************/

MergeChannels::MergeChannels(PClip _clip, int _num_children, PClip* _child_array, IScriptEnvironment* env)
    : GenericVideoFilter(_clip), num_children(_num_children), child_array(_child_array) {
  VideoInfo vi2;
  PClip tclip;

  clip_channels = new int[num_children];
  clip_offset = new signed char * [num_children];
  clip_channels[0] = vi.AudioChannels();

  for (int i = 1;i < num_children;i++) {
    tclip = child_array[i];
    child_array[i] = ConvertAudio::Create(tclip, vi.SampleType(), vi.SampleType());  // Clip 2 should now be same type as clip 1.
    vi2 = child_array[i]->GetVideoInfo();

    if (vi.audio_samples_per_second != vi2.audio_samples_per_second) {
      env->ThrowError("MergeChannels: Clips must have same sample rate! Use ResampleAudio()!");  // Could be removed for fun :)
    }
    if (vi.SampleType() != vi2.SampleType())
      env->ThrowError("MergeChannels: Clips must have same sample type! Use ConvertAudio()!");    // Should never happend!
    clip_channels[i] = vi2.AudioChannels();
    vi.nchannels += vi2.AudioChannels();
  }

  tempbuffer_size = 0;
}

MergeChannels::~MergeChannels() {
  if (tempbuffer_size) {
    delete[] tempbuffer;
	tempbuffer_size=0;
  }
  delete[] clip_channels;
  delete[] clip_offset;
  delete[] child_array;
}


void __stdcall MergeChannels::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) {
  if (tempbuffer_size < count) {
    if (tempbuffer_size) delete[] tempbuffer;
    tempbuffer = new signed char[count * vi.BytesPerAudioSample()];
    tempbuffer_size = count;
  }
  // Get audio:
  const int channel_offset = count * vi.BytesPerChannelSample();  // Offset per channel
  int i, c_channel = 0;

  for (i = 0;i < num_children;i++) {
    child_array[i]->GetAudio(tempbuffer + (c_channel*channel_offset), start, count, env);
    clip_offset[i] = tempbuffer + (c_channel * channel_offset);
    c_channel += clip_channels[i];
  }

  // Interleave channels
  char* samples = (char*) buf;
  const int bpcs = vi.BytesPerChannelSample();
  const int bps = vi.BytesPerAudioSample();
  int dst_offset = 0;
  for (i = 0;i < num_children;i++) {
    signed char* src_buf = clip_offset[i];
	const int bpcc = bpcs*clip_channels[i];

	switch (bpcc) {
	
	case 2: { // mono 16 bit
        for (int l = 0, k=dst_offset; l < count; l++, k+=bps) {
          *(short*)(samples+k) = ((short*)src_buf)[l];
        }
        break;
      }
	case 4: { // mono float/32 bit, stereo 16 bit
        for (int l = 0, k=dst_offset; l < count; l++, k+=bps) {
          *(int32_t*)(samples+k) = ((int32_t*)src_buf)[l];
        }
        break;
      }
	case 8: { // stereo float/32 bit
		if (env->GetCPUFlags() & CPUF_MMX) {
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
          __asm {
		    mov eax,[src_buf]
		     mov edi,[samples]
		    mov ecx,dword ptr[count]
		     add edi,[dst_offset]
			test ecx,ecx
		     mov edx,[bps]    ; bytes per strip
			jz done
			 shr ecx,1        ; CF=count&1, count>>=1
			 jnc label        ; count was even

		    movq mm1,[eax]    ; do 1 odd quad
		     add eax,8
		    movq [edi],mm1
		     add edi,edx
			test ecx,ecx
			jz done
			 align 16
label:
		    movq mm0,[eax]    ; do pairs of quads
		     movq mm1,[eax+8]
		    add eax,16
		     movq [edi],mm0
		    movq [edi+edx],mm1
		     lea edi,[edi+edx*2]
			loop label
done:
		    emms
          }
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
        }
		else {
          for (int l = 0, k=dst_offset; l < count; l++, k+=bps) {
            *(__int64*)(samples+k) = ((__int64*)src_buf)[l];
          }
        }
        break;
      }
	default: { // everything else, 1 byte at a time
        for (int l = 0; l < count; l++) {
          for (int k = 0; k < bpcc; k++) {
            samples[dst_offset + (l*bps) + k] = src_buf[(l*bpcc) + k];
          }
        }
      }
    }
    dst_offset += bpcc;
  }
}


AVSValue __cdecl MergeChannels::Create(AVSValue args, void*, IScriptEnvironment* env) {
  int num_args;
  PClip* child_array;

  if (args[0].IsArray()) {
    num_args = args[0].ArraySize();
    if (num_args == 1)
      return args[0];

    child_array = new PClip[num_args];
    for (int i = 0; i < num_args; ++i)
      child_array[i] = args[0][i].AsClip();

    return new MergeChannels(args[0][0].AsClip(), num_args, child_array, env);

  } else {
    num_args = 2;
    child_array = new PClip[num_args];
    child_array[0] = args[0].AsClip();
    child_array[1] = args[1].AsClip();
  }
  return new MergeChannels(args[0].AsClip(), num_args, child_array, env);
}


/***************************************************
 *******   Get left or right                 *******
 *******    channel from a stereo source     *******
 ***************************************************/



GetChannel::GetChannel(PClip _clip, int* _channel, int _numchannels)
    : GenericVideoFilter(_clip), channel(_channel), numchannels(_numchannels)
{
  cbps = vi.BytesPerChannelSample();
  src_bps = vi.BytesPerAudioSample();
  vi.nchannels = numchannels;
  tempbuffer_size = 0;
  dst_bps = vi.BytesPerAudioSample();
}


void __stdcall GetChannel::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) {
  if (tempbuffer_size < count) {
    if (tempbuffer_size) delete[] tempbuffer;
    tempbuffer = new char[count * src_bps];
    tempbuffer_size = count;
  }
  child->GetAudio(tempbuffer, start, count, env);
  
  switch (cbps) {
  case 1: {    // 8 bit
      char* samples = (char*)buf;
      char* tbuff = tempbuffer;
      for (int i = 0; i < count; i++) {
        for (int k = 0; k < numchannels; k++) {
          *(samples++) = tbuff[channel[k]];
        }
        tbuff += src_bps;
      }
	  break;
    }
  case 2: {    // 16 bit
      short* samples = (short*)buf;
      short* tbuff = (short*)tempbuffer;
      for (int i = 0; i < count; i++) {
        for (int k = 0; k < numchannels; k++) {
          *(samples++) = tbuff[channel[k]];
        }
        tbuff += src_bps>>1;
      }
	  break;
    }
  case 4: {    // float/32 bit
      int32_t* samples = (int32_t*)buf;
      int32_t* tbuff = (int32_t*)tempbuffer;
      for (int i = 0; i < count; i++) {
        for (int k = 0; k < numchannels; k++) {
          *(samples++) = tbuff[channel[k]];
        }
        tbuff += src_bps>>2;
      }
	  break;
    }
  default: {  // 24 bit, etc
      char* samples = (char*)buf;
      char* tbuff = tempbuffer;
      for (int i = 0; i < count; i++) {
        for (int k = 0; k < numchannels; k++) {
          int src_o = channel[k] * cbps;
          for (int j = src_o; j < src_o+cbps; j++)
            *(samples++) = tbuff[j];
        }
        tbuff += src_bps;
      }
	  break;
    }
  }
}


PClip GetChannel::Create_left(PClip clip) {

  if (clip->GetVideoInfo().AudioChannels() == 1)
    return clip;

  int* ch = new int[1];
  ch[0] = 0;
  return new GetChannel(clip, ch, 1);
}

PClip GetChannel::Create_right(PClip clip) {
  if (clip->GetVideoInfo().AudioChannels() == 1)
    return clip;

  int* ch = new int[1];
  ch[0] = 1;
  return new GetChannel(clip, ch, 1);
}

PClip GetChannel::Create_n(PClip clip, int* n, int numchannels) {
  return new GetChannel(clip, n, numchannels);
}

AVSValue __cdecl GetChannel::Create_left(AVSValue args, void*, IScriptEnvironment*) {
  return Create_left(args[0].AsClip());
}

AVSValue __cdecl GetChannel::Create_right(AVSValue args, void*, IScriptEnvironment*) {
  return Create_right(args[0].AsClip());
}

AVSValue __cdecl GetChannel::Create_n(AVSValue args, void*, IScriptEnvironment* env) {
  AVSValue args_c = args[1];
  const int num_args = args_c.ArraySize();
  int* child_array = new int[num_args];
  for (int i = 0; i < num_args; ++i) {
    child_array[i] = args_c[i].AsInt() - 1;  // Beware: Channel is 0-based in code and 1 based in scripts
    if (child_array[i] >= args[0].AsClip()->GetVideoInfo().AudioChannels())
      env->ThrowError("GetChannel: Attempted to request a channel that didn't exist!");
    if (child_array[i] < 0)
      env->ThrowError("GetChannel: There are no channels below 1! (first channel is 1)");
  }
  return Create_n(args[0].AsClip(), child_array, num_args);
}

/******************************
 *******   Kill Video  ********
 ******************************/

KillVideo::KillVideo(PClip _clip)
    : GenericVideoFilter(_clip) {
  vi.width = 0;
  vi.height= 0;
  vi.fps_numerator  = 0;
  vi.fps_denominator= 0;
  vi.num_frames = 0;
  vi.pixel_type = 0;
  vi.image_type = 0;
}

AVSValue __cdecl KillVideo::Create(AVSValue args, void*, IScriptEnvironment*) {
  return new KillVideo(args[0].AsClip());
}


/******************************
 *******   Kill Audio  ********
 ******************************/

KillAudio::KillAudio(PClip _clip)
    : GenericVideoFilter(_clip) {
  vi.audio_samples_per_second = 0;
  vi.sample_type = 0;
  vi.num_audio_samples = 0;
  vi.nchannels = 0;
}

AVSValue __cdecl KillAudio::Create(AVSValue args, void*, IScriptEnvironment*) {
  return new KillAudio(args[0].AsClip());
}


/******************************
 *******   Delay Audio   ******
 *****************************/

DelayAudio::DelayAudio(double delay, PClip _child)
    : GenericVideoFilter(_child), delay_samples(__int64(delay * vi.audio_samples_per_second + 0.5)) {
  vi.num_audio_samples += delay_samples;
}


void DelayAudio::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) {
  child->GetAudio(buf, start - delay_samples, count, env);
}


AVSValue __cdecl DelayAudio::Create(AVSValue args, void*, IScriptEnvironment* env) {
  return new DelayAudio(args[1].AsFloat(), args[0].AsClip());
}


/********************************
 *******   Amplify Audio   ******
 *******************************/


Amplify::Amplify(PClip _child, float* _volumes, int* _i_v)
    : GenericVideoFilter(ConvertAudio::Create(_child, SAMPLE_INT16 | SAMPLE_FLOAT | SAMPLE_INT32, SAMPLE_FLOAT)),
volumes(_volumes), i_v(_i_v) { }


Amplify::~Amplify()
{
    if (volumes) { delete[] (float*)volumes; volumes=0; }
    if (i_v)     { delete[] (int*)i_v;     i_v=0;     }
}


void __stdcall Amplify::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) {
  child->GetAudio(buf, start, count, env);
  int channels = vi.AudioChannels();
  int countXchannels = count*channels; 

  if (vi.SampleType() == SAMPLE_INT16) {
// Talk about crap assembler code, who taught this compiler how to do 64bits!
//  short* samples = (short*)buf;
//  for (int i = 0; i < countXchannels; i+=channels) {
//    for (int j = 0; j < channels; j++) {
//      samples[i + j] = Saturate(Int32x32To64(samples[i + j], i_v[j]) + 65536);
//    }
//  }
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
    const short* endsample = (short*)buf + countXchannels;
    const int* iv = i_v;
    __asm {
          mov	 ecx, [iv]
          mov	 edi, [buf]
          align  16
iloop0:
          xor	 esi, esi					; j
jloop0:
          mov	 eax, DWORD PTR [ecx+esi*4]	; i_v[j]
          movsx	 edx, WORD PTR [edi]		; *samples
          inc	 esi						; j++
          imul	 edx
          add	 edi, 2						; samples++
          add	 eax, 65536
          adc	 edx, 0

          cmp	 edx, -1					; if (nh < -1) return MIN_SHORT;
          jge	 notnegsat0
          mov	 eax, -32768
          jmp	 saturate0
notnegsat0:
          test	 edx, edx					; if (nh >  0) return MAX_SHORT;
          jle	 notpossat0
          mov	 eax, 32767
          jmp	 saturate0
notpossat0:
          shrd	 eax, edx, 17				; n>>17
saturate0:
          mov	 WORD PTR [edi-2], ax		; *samples
          cmp	 esi, [channels]			; j < channels
          jl	 jloop0

          cmp	 edi, [endsample]
          jl	 iloop0
    }
#else
    //
    // Unclear as to why the original code in the Amplify::Create and 
    // Amplify::Create_dB converts floating point volumes constant by using 
    // this formula:
    //      i_child_array[i] = int(child_array[i] * 131072.0f + 0.5f);
    // 
    // Until the explanation somehow arrives, it will be treated as the bug,
    // and the correction will be applied here:
    // 
    int32_t nCoefs[2]; // choosing 32-bit allows us to have coefficient values > 1.0
    for(int k = 0; k < channels; k++)
        nCoefs[k] = (int32_t)(volumes[k]*32767.0);

    int16_t* samples = (int16_t*)buf;
    for (int i = 0; i < countXchannels; i+=channels) {
        for (int j = 0; j < channels; j++) {
            int32_t temp = (int32_t)samples[i + j];
            temp *= nCoefs[j]; 
            temp >>= 16;
            if(temp > 32767) 
                temp = 32767;
            else if(temp < -32768)
                temp = -32768;
            samples[i + j] = (int16_t)(temp & 0xFFFF);
        }
    }

#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
    return ;
  }

  if (vi.SampleType() == SAMPLE_INT32) {
//  const int* samples = (int*)buf;
//  for (int i = 0; i < countXchannels; i+=channels) {
//    for (int j = 0;j < channels;j++) {
//      samples[i + j] = Saturate_int32(Int32x32To64(samples[i + j], i_v[j]) + 32768);
//    }
//  }
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
    const int* endsample = (int*)buf + countXchannels;
    const int* iv = i_v;
    __asm {
          mov	 ecx, [iv]
          mov	 edi, [buf]
          align  16
iloop1:
          xor	 esi, esi					; j
jloop1:
          mov	 eax, DWORD PTR [ecx+esi*4]	; i_v[j]
          mov  	 edx, DWORD PTR [edi]		; *samples
          inc	 esi						; j++
          imul	 edx
          add	 edi, 4						; samples++
          add	 eax, 65536
          adc	 edx, 0

          cmp	 edx,0xffff0000				; if (nh < -65536) return MIN_INT;
          jge	 notnegsat1
          mov	 eax, 0x80000000
          jmp	 saturate1
notnegsat1:
          cmp 	 edx,0x0000ffff				; if (nh >  65535) return MAX_INT;
          jle	 notpossat1
          mov	 eax, 0x7fffffff
          jmp	 saturate1
notpossat1:
          shrd	 eax, edx, 17				; n>>17
saturate1:
          mov	 DWORD PTR [edi-4], eax		; *samples
          cmp	 esi, [channels]			; j < channels
          jl	 jloop1

          cmp	 edi, [endsample]
          jl	 iloop1
    }
#else
    //
    // Unclear as to why the original code in the Amplify::Create and 
    // Amplify::Create_dB converts floating point volumes constant by using 
    // this formula:
    //      i_child_array[i] = int(child_array[i] * 131072.0f + 0.5f);
    // 
    // Until the explanation somehow arrives, it will be treated as the bug,
    // and the correction will be applied here:
    // 
    int64_t nCoefs[2]; // choosing 64-bit allows us to have coefficient values > 1.0
    for(int k = 0; k < channels; k++)
        nCoefs[k] = (int64_t)(volumes[k]*2147483647.0);
    
    int32_t* samples = (int32_t*)buf;
    for (int i = 0; i < countXchannels; i+=channels) {
        for (int j = 0; j < channels; j++) {
            int32_t nSample = samples[i + j];
            int64_t temp = (int64_t)samples[i + j];
            temp *= nCoefs[j]; 
            temp >>= 32;
            if(temp > 2147483647) 
                temp = 2147483647;
            else if(temp < -2147483648)
                temp = -2147483648;
            nSample = (int32_t)(temp & 0xFFFFFFFF);
            samples[i + j] = nSample;
        }
    }
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
    return ;
  }
  if (vi.SampleType() == SAMPLE_FLOAT) {
    SFLOAT* samples = (SFLOAT*)buf;
    for (int i = 0; i < countXchannels; i+=channels) {
      for (int j = 0;j < channels;j++) {				// Does not saturate, as other filters do. 
        samples[i + j] = samples[i + j] * volumes[j];	// We should saturate only on conversion.
      }
    }
    return ;
  }
}


AVSValue __cdecl Amplify::Create(AVSValue args, void*, IScriptEnvironment* env) {
  if (!args[0].AsClip()->GetVideoInfo().AudioChannels())
    return args[0];
  AVSValue args_c = args[1];
  const int num_args = args_c.ArraySize();
  const int ch = args[0].AsClip()->GetVideoInfo().AudioChannels();
  float* child_array = new float[ch];
  int* i_child_array = new int[ch];
  for (int i = 0; i < ch; ++i) {
    child_array[i] = args_c[std::min(i, num_args - 1)].AsFloat();
    i_child_array[i] = int(child_array[i] * 131072.0f + 0.5f);
  }
  return new Amplify(args[0].AsClip(), child_array, i_child_array);
}



AVSValue __cdecl Amplify::Create_dB(AVSValue args, void*, IScriptEnvironment* env) {
	try {	// HIDE DAMN SEH COMPILER BUG!!!
  if (!args[0].AsClip()->GetVideoInfo().AudioChannels())
    return args[0];
  AVSValue args_c = args[1];
  const int num_args = args_c.ArraySize();
  const int ch = args[0].AsClip()->GetVideoInfo().AudioChannels();
  float* child_array = new float[ch];
  int* i_child_array = new int[ch];
  for (int i = 0; i < ch; ++i) {
    child_array[i] = dBtoScaleFactor(args_c[std::min(i, num_args - 1)].AsFloat());
    i_child_array[i] = int(child_array[i] * 131072.0f + 0.5f);
  }
  return new Amplify(args[0].AsClip(), child_array, i_child_array);
	}
	catch (...) { throw; }
}


/*****************************
 ***** Normalize audio  ******
 ***** Supports int16,float******
 ******************************/

Normalize::Normalize(PClip _child, float _max_factor, bool _showvalues)
    : GenericVideoFilter(ConvertAudio::Create(_child, SAMPLE_INT16 | SAMPLE_FLOAT, SAMPLE_FLOAT)),
    max_factor(_max_factor), showvalues(_showvalues) {
  max_volume = -1.0f;
}



void __stdcall Normalize::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) {
  if (max_volume < 0.0f) {
    // Read samples into buffer and test them
    if (vi.SampleType() == SAMPLE_INT16) {
      __int64 bcount = count;
      short* samples = (short*)buf;
      bool bigbuff=false;

	  if (vi.BytesFromAudioSamples(count) < BIGBUFFSIZE) {
	    samples = new short[BIGBUFFSIZE/sizeof(short)];
	    if (samples) {
	      bigbuff=true;
	      bcount = vi.AudioSamplesFromBytes(BIGBUFFSIZE);
	    }
	    else {
	      samples = (short*)buf; // malloc failed just reuse clients buffer
	    }
	  }

      const __int64 passes = vi.num_audio_samples / bcount;
	  __int64 negpeaksampleno=-1, pospeaksampleno=-1;
      short i_pos_volume = 0;
      short i_neg_volume = 0;
      const int chanXbcount = bcount * vi.AudioChannels();

      for (__int64 i = 0; i < passes; i++) {
        child->GetAudio(samples, bcount*i, bcount, env);
        for (int j = 0; j < chanXbcount; j++) {
		  const short sample=samples[j];
          if (sample < i_neg_volume) {	// Cope with MIN_SHORT
		    i_neg_volume = sample;
		    negpeaksampleno = chanXbcount*i+j;
			if (sample <= -32767) {
			  i = passes;
			  break;
			}
		  }
		  else if (sample > i_pos_volume) {
			i_pos_volume = sample;
			pospeaksampleno = chanXbcount*i+j;
			if (sample == 32767) {
			  i = passes;
			  break;
			}
		  }
        }
      }
	  // Remaining samples
	  if ((i_pos_volume != 32767) && (i_neg_volume > -32767)) {
		const __int64 rem_samples = vi.num_audio_samples % bcount;
		const int chanXremcount = rem_samples * vi.AudioChannels();

		child->GetAudio(samples, bcount*passes, rem_samples, env);
		for (int j = 0; j < chanXremcount; j++) {
		  const short sample=samples[j];
		  if (sample < i_neg_volume) {	// Cope with MIN_SHORT
			i_neg_volume = sample;
			negpeaksampleno = chanXbcount*passes+j;
		  }
		  else if (sample > i_pos_volume) {
			i_pos_volume = sample;
			pospeaksampleno = chanXbcount*passes+j;
		  }
		}
	  }
	  if (bigbuff) delete[] samples;

	  i_pos_volume = -i_pos_volume; // Remember -ve has 1 more range than +ve, i.e. -32768
	  if (i_neg_volume < i_pos_volume) {
	    i_pos_volume = i_neg_volume;
	    frameno = vi.FramesFromAudioSamples(negpeaksampleno / vi.AudioChannels());
	  }
	  else {
	    frameno = vi.FramesFromAudioSamples(pospeaksampleno / vi.AudioChannels());
	  }
      max_volume = float(i_pos_volume * (-1.0/32768.0));
      max_factor = max_factor / max_volume;

    } else if (vi.SampleType() == SAMPLE_FLOAT) {  // Float
      __int64 bcount = count;
      SFLOAT* samples = (SFLOAT*)buf;
      bool bigbuff=false;

	  if (vi.BytesFromAudioSamples(count) < BIGBUFFSIZE) {
	    samples = new SFLOAT[BIGBUFFSIZE/sizeof(SFLOAT)];
	    if (samples) {
	      bigbuff=true;
	      bcount = vi.AudioSamplesFromBytes(BIGBUFFSIZE);
	    }
	    else {
	      samples = (SFLOAT*)buf; // malloc failed just reuse clients buffer
	    }
	  }

      const int chanXbcount = bcount * vi.AudioChannels();
      const __int64 passes = vi.num_audio_samples / bcount;
	  __int64 peaksampleno=-1;
	  
      for (__int64 i = 0;i < passes;i++) {
        child->GetAudio(samples, bcount*i, bcount, env);
        for (int j = 0;j < chanXbcount;j++) {
		  const SFLOAT sample = fabs(samples[j]);
          if (sample > max_volume) {
		    max_volume = sample;
			peaksampleno = chanXbcount*i+j;
		  }
        }
      }
      // Remaining samples
      const __int64 rem_samples = vi.num_audio_samples % bcount;
      const int chanXremcount = rem_samples * vi.AudioChannels();

      child->GetAudio(samples, bcount*passes, rem_samples, env);
      for (int j = 0;j < chanXremcount;j++) {
		const SFLOAT sample = fabs(samples[j]);
        if (sample > max_volume) {
		  max_volume = sample;
		  peaksampleno = chanXbcount*passes+j;
		}
      }
	  if (bigbuff) delete[] samples;

	  frameno = vi.FramesFromAudioSamples(peaksampleno / vi.AudioChannels());
      max_factor = max_factor / max_volume;
    }
  }

  const int chanXcount = count * vi.AudioChannels();

  if (vi.SampleType() == SAMPLE_INT16) {
    const int factor = (int)(max_factor * 131072.0f + 0.5f);
    child->GetAudio(buf, start, count, env);

//  short* samples = (short*)buf;
//  for (int i = 0; i < chanXcount; ++i) {
//    samples[i] = Saturate(Int32x32To64(samples[i], factor) + 32768);
//  }
    const short* endsample = (short*)buf + chanXcount;
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
    __asm {
          mov	 ecx, [factor]
          mov	 edi, [buf]
          align  16
iloop2:
          movsx	 eax, WORD PTR [edi]		; *samples
          imul	 ecx
          add	 edi, 2						; samples++
          add	 eax, 65536
          adc	 edx, 0

          cmp	 edx, -1					; if (nh < -1) return MIN_SHORT;
          jge	 notnegsat2
          mov	 eax, -32768
          jmp	 saturate2
notnegsat2:
          test	 edx, edx					; if (nh >  0) return MAX_SHORT;
          jle	 notpossat2
          mov	 eax, 32767
          jmp	 saturate2
notpossat2:
          shrd	 eax, edx, 17				; n>>17
saturate2:
          mov	 WORD PTR [edi-2], ax		; *samples

          cmp	 edi, [endsample]
          jl	 iloop2
    }
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
  } else if (vi.SampleType() == SAMPLE_FLOAT) {
    SFLOAT* samples = (SFLOAT*)buf;
    child->GetAudio(buf, start, count, env);
    for (int i = 0; i < chanXcount; ++i) {
      samples[i] = samples[i] * max_factor;
    }
  }
}

PVideoFrame __stdcall Normalize::GetFrame(int n, IScriptEnvironment* env) {
  if (showvalues) {
    PVideoFrame src = child->GetFrame(n, env);
    env->MakeWritable(&src);
    char text[400];

    if (max_volume < 0) {
      sprintf(text, "Normalize: Result not yet calculated!");
    } else {
      double maxdb = 8.685889638 * log(max_factor);
      // maxdb = (20 * log(factor)) / log(10);
      sprintf(text, "Amplify Factor: %8.4f\nAmplify DB: %8.4f\nAt Frame: %d", max_factor, maxdb, frameno);
    }
    ApplyMessage(&src, vi, text, vi.width / 4, 0xf0f080, 0, 0 , env );
    return src;
  }
  return child->GetFrame(n, env);

}


AVSValue __cdecl Normalize::Create(AVSValue args, void*, IScriptEnvironment* env) {

  return new Normalize(args[0].AsClip(), args[1].AsFloat(1.0), args[2].AsBool(false));
}


/*****************************
 ***** Mix audio  tracks ******
 ******************************/

MixAudio::MixAudio(PClip _child, PClip _clip, double _track1_factor, double _track2_factor, IScriptEnvironment* env)
    : GenericVideoFilter(ConvertAudio::Create(_child, SAMPLE_INT16 | SAMPLE_FLOAT, SAMPLE_FLOAT)),
    track1_factor(int(_track1_factor*131072.0 + 0.5)),
    track2_factor(int(_track2_factor*131072.0 + 0.5)),
    t1factor(float(_track1_factor)),
    t2factor(float(_track2_factor)) {

  clip = ConvertAudio::Create(_clip, vi.SampleType(), vi.SampleType());  // Clip 2 should now be same type as clip 1.
  const VideoInfo vi2 = clip->GetVideoInfo();

  if (vi.audio_samples_per_second != vi2.audio_samples_per_second)
    env->ThrowError("MixAudio: Clips must have same sample rate! Use ResampleAudio()!");  // Could be removed for fun :)

  if (vi.AudioChannels() != vi2.AudioChannels())
    env->ThrowError("MixAudio: Clips must have same number of channels! Use ConvertToMono() or MergeChannels()!");

  tempbuffer_size = 0;
}


void __stdcall MixAudio::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) {
  if (tempbuffer_size) {
    if (tempbuffer_size < count) {
      delete[] tempbuffer;
      tempbuffer = new signed char[count * vi.BytesPerAudioSample()];
      tempbuffer_size = count;
    }
  } else {
    tempbuffer = new signed char[count * vi.BytesPerAudioSample()];
    tempbuffer_size = count;
  }

  child->GetAudio(buf, start, count, env);
  clip->GetAudio(tempbuffer, start, count, env);
  unsigned channels = vi.AudioChannels();

  if (vi.SampleType()&SAMPLE_INT16) {
//  short* samples = (short*)buf;
//  short* clip_samples = (short*)tempbuffer;
//  for (unsigned i = 0; i < unsigned(count)*channels; ++i) {
//      samples[i] = Saturate64(Int32x32To64(samples[i], track1_factor) + Int32x32To64(clip_samples[i], track2_factor) + 32768);
//  }
    const short* tbuffer = (short*)tempbuffer;
    const short* endsample = (short*)buf + unsigned(count)*channels;
	const int t1_factor = track1_factor;
	const int t2_factor = track2_factor;

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
    __asm {
		  push   ebx
          mov	 edi, [buf]
          mov	 esi, [tbuffer]
          align  16
iloop3:
          movsx	 eax, WORD PTR [edi]		; *samples
          add	 edi, 2						; samples++
          imul	 [t1_factor]
		  mov    ebx, eax
		  mov    ecx, edx
          movsx	 eax, WORD PTR [esi]		; *clip_samples
          add	 esi, 2						; clip_samples++
          imul	 [t2_factor]
          add	 eax, ebx
          adc	 edx, ecx
          add	 eax, 65536
          adc	 edx, 0

          cmp	 edx, -1					; if (nh < -1) return MIN_SHORT;
          jge	 notnegsat3
          mov	 eax, -32768
          jmp	 saturate3
notnegsat3:
          test	 edx, edx					; if (nh >  0) return MAX_SHORT;
          jle	 notpossat3
          mov	 eax, 32767
          jmp	 saturate3
notpossat3:
          shrd	 eax, edx, 17				; n>>17
saturate3:
          mov	 WORD PTR [edi-2], ax		; *samples

          cmp	 edi, [endsample]
          jl	 iloop3
		  pop    ebx
    }
#else
    int16_t* samples = (int16_t*)buf;
    int16_t* clip_samples = (int16_t*)tempbuffer;
    for (unsigned i = 0; i < unsigned(count)*channels; ++i) {
        samples[i] = (int16_t)(samples[i] * t1factor) + (clip_samples[i] * t2factor);
    }
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
  } else if (vi.SampleType()&SAMPLE_FLOAT) {
    SFLOAT* samples = (SFLOAT*)buf;
    SFLOAT* clip_samples = (SFLOAT*)tempbuffer;
    for (unsigned i = 0; i < unsigned(count)*channels; ++i) {
        samples[i] = (samples[i] * t1factor) + (clip_samples[i] * t2factor);
    }
  }
}



AVSValue __cdecl MixAudio::Create(AVSValue args, void*, IScriptEnvironment* env) {
	try {	// HIDE DAMN SEH COMPILER BUG!!!
  double track1_factor = args[2].AsFloat(0.5);
  double track2_factor = args[3].AsFloat(1.0 - track1_factor);
  return new MixAudio(args[0].AsClip(), args[1].AsClip(), track1_factor, track2_factor, env);
	}
	catch (...) { throw; }
}



/********************************
 *******   Resample Audio   ******
 *******************************/

static int Amasktab[Amask+1];
static SFLOAT fAmasktab[Amask+1];

ResampleAudio::ResampleAudio(PClip _child, int _target_rate_n, int _target_rate_d, IScriptEnvironment* env)
    : GenericVideoFilter(ConvertAudio::Create(_child, SAMPLE_INT16 | SAMPLE_FLOAT, SAMPLE_FLOAT)),
	  target_rate((_target_rate_n + (_target_rate_d>>1))/_target_rate_d),
      factor(_target_rate_n / (double(_target_rate_d) * vi.audio_samples_per_second)) {
	try {	// HIDE DAMN SEH COMPILER BUG!!!
  srcbuffer  = 0;
  fsrcbuffer = 0;

  if ((target_rate == vi.audio_samples_per_second) || (vi.audio_samples_per_second == 0)) {
    skip_conversion = true;
    return ;
  }
  skip_conversion = false;

//vi.num_audio_samples = MulDiv(vi.num_audio_samples, target_rate, vi.audio_samples_per_second);

  // To avoid overflow, implement as (A*B+C/2)/C = (A/C)*B + ((A%C)*B+C>>1)/C
  const __int64 den = Int32x32To64(_target_rate_d, vi.audio_samples_per_second);
  vi.num_audio_samples = (vi.num_audio_samples/den) * _target_rate_n + ((vi.num_audio_samples%den)*_target_rate_n + (den>>1))/den;
  vi.audio_samples_per_second = target_rate;

  if (vi.IsSampleType(SAMPLE_INT16)) {
	double dLpScl;

	// Load interpolate ratio table
	for (int i=0; i<=Amask; i++)
  	  Amasktab[i] = (i<<16) | (Amask+1-i);   /* a is between 0 and 1 */

	// generate filter coefficients
	makeFilter(Imp, &dLpScl, Nwing, 0.90, 9);
	Imp[Nwing] = 0; // for "interpolation" beyond last coefficient

	/* Account for increased filter gain when using factors less than 1 */
	if (factor < 1)
	  dLpScl = dLpScl * factor;

	// Attenuate resampler scale factor by 0.95 to reduce probability of clipping
	LpScl = int(dLpScl * 0.95 + 0.5);

	// Scale guard bits so intermediate result fits in short
	mNhg   = Nhg;
	while (dLpScl < 16384.0) {
	  dLpScl *= 2.0;
	  mNhg += 1;
	}
	mLpScl = int(dLpScl + 0.5);   // Must be 16384 <= mLpScl <= 32767
  }
  else { // SAMPLE_FLOAT

	// Load interpolate ratio table
	for (int i=0; i<=Amask; i++)
	  fAmasktab[i] = float(i) / (Amask+1);   /* a is between 0 and 1 */

	/* Account for increased filter gain when using factors less than 1 */
	if (factor < 1)
	  makeFilter(fImp, factor, Nwing, 0.90, 9);  // generate filter coefficients
	else
	  makeFilter(fImp, 1.0,    Nwing, 0.90, 9);  // generate filter coefficients

	fImp[Nwing] = 0.0; // for "interpolation" beyond last coefficient

  }

  /* Calc reach of LP filter wing & give some creeping room */
  Xoff = int(((Nmult + 1) / 2.0) * std::max(1.0, 1.0 / factor)) + 10;

  /* The previous algorithm was causing quite a noticable click or pop at the
   * end of each mouthful due to accumulated creep between pos+N*dtb at the
   * end of one call to (start/factor*(1<<Np)+0.5) in the next.
   */
  double dt = (1 << Np) / factor;              /* Output sampling period          */
  dtb = int(dt);                               /* Yes! Truncated not rounded      */
  dt -= dtb;                                   /* 0 <= SamplingPeriodDeficit < 1  */
  dtbe = unsigned((1 << 31) * dt + 0.5);       /* Prevent creep, bump dtb every (2^31)/dtbe samples */

  double dh = std::min(double(Npc), factor * Npc);  /* Filter sampling period */
  dhb = int(dh * (1 << Na) + 0.5);

//  child->SetCacheHints(CACHE_AUDIO, 0); // No longer needed, we recycle our Xoff extra samples

	}
	catch (...) { throw; }
}


void __stdcall ResampleAudio::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) {
  if (skip_conversion) {
    child->GetAudio(buf, start, count, env);
    return ;
  }
  __int64 src_start = __int64(start / factor * (1 << Np) + 0.5);
  __int64 src_end = __int64(((start + count) / factor) * (1 << Np) + 0.5);
  const __int64 source_samples = ((src_end - src_start) >> Np) + 2 * Xoff + 1;
  const int source_bytes = vi.BytesFromAudioSamples(source_samples);

  __int64 pos = (int(src_start & Pmask)) + (Xoff << Np);
  int ch = vi.AudioChannels();
  unsigned dtberror = 0;

  if (vi.IsSampleType(SAMPLE_INT16)) {

	if (!srcbuffer || source_bytes > srcbuffer_size) {
	  if (srcbuffer)
		delete[] srcbuffer;
	  srcbuffer = new short[source_bytes >> 1];
	  srcbuffer_size = source_bytes;
	  last_samples= 0;
	  last_start = 0;
	}

	const int offset = int((src_start >> Np) - Xoff - last_start);  // Difference from last time
	last_start = (src_start >> Np) - Xoff;                          // Start for next time
	
	int overlap = int(last_samples - offset);                       // How many samples already fetched
	if ((offset < 0) || (overlap <= 0))                             // Is there any overlap?
	  overlap = 0;
	else if (offset*ch >= 2)  // Assume 32bit separation is okay    // Yes, copy to start of buffer
	  memcpy(srcbuffer, srcbuffer+offset*ch, overlap*ch<<1);  // fast
	else if (offset > 0)
	  memmove(srcbuffer, srcbuffer+offset*ch, overlap*ch<<1); // slow

	last_samples= std::max((__int64_t)overlap, source_samples);                     // Samples for next time

    if (source_samples-overlap > 0)                                 // Get the rest of the source samples
	  child->GetAudio(&srcbuffer[overlap*ch], last_start+overlap, source_samples-overlap, env);

	short* dst = (short*)buf;

	short* dst_end = &dst[count * ch];

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
    if (env->GetCPUFlags() & CPUF_MMX) {
	  static int r_Na     = 1 << (Na-1);
	  static int r_Nhxn   = 1 << (Nhxn-1);
	  static int r_NLpScl = 1 << (NLpScl-1);
	  const int inc = ch * sizeof(short);
	  int posNp = int(pos >> Np);

// MM7 - Accumulate the left/right wing inner product of the current sample pair
// MM6 - Rounding constant 1 << (Na-1),       (64)
// MM5 - Rounding constant 1 << (Nhxn-1),   (8192)
// MM4 - Scaled scaling factor, mLpScl
// MM3 - Rounding constant 1 << (NLpScl-1), (4096)
// MM2 - Scaled number of guard bits, mNhg
	  __asm {
		movd       mm6, [r_Na]          ; 1 << (Na - 1)
		movd       mm5, [r_Nhxn]        ; 1 << (Nhxn - 1)
		punpckldq  mm6, mm6             ; 00000040 00000040
		mov        eax, this
		punpckldq  mm5, mm5             ; 00002000 00002000
		movd       mm4, [eax].mLpScl    ; 00000000 LpScl
		movd       mm3, [r_NLpScl]      ; 1 << (NLpScl-1)
		punpckldq  mm4, mm4             ; LpScl | LpScl
		movd       mm2, [eax].mNhg      ; Number of guard bits
		punpckldq  mm3, mm3             ; 00001000 00001000
	  }

        while (dst < dst_end) {
		for (int q = 0; q < ch; q+=2) { // do 2 channels at once
		  bool single = (q+1 >= ch);
		  short* Xp = &srcbuffer[posNp * ch];

		  __asm pxor mm7, mm7;  // 2 channel samples are accumulated in MM7

		  FilterUD_mmx(Xp + ch + q, (unsigned)(-pos) & Pmask,  inc, dhb, Imp, Nwing);  /* Perform right-wing inner product */
		  FilterUD_mmx(Xp      + q, (unsigned)( pos) & Pmask, -inc, dhb, Imp, Nwing);  /* Perform left-wing inner product */
		  
		  __asm {
		    psrad      mm7, mm2              ; scaled Nhg guard bits
		     mov       eax, [dst]
			pmaddwd    mm7, mm4              ; Normalize for unity filter gain
			 test      byte ptr[single], 1   ; doing 1 sample or 2 samples?
			paddd      mm7, mm3              ; round
			 jnz       dosingle
			psrad      mm7, NLpScl           ; strip guard bits
			 add       eax, 4                ; dst+=2
			packssdw   mm7, mm7              ; pack with signed saturation ready for output
			 mov       [dst], eax
			movd       [eax-4], mm7          ; deposit 2 output samples
			 jmp       done1
			align      16
dosingle:
			psrad      mm7, NLpScl           ; strip guard bits
			 add       eax, 2                ; dst+=
			packssdw   mm7, mm7              ; pack with signed saturation ready for output
			 mov       [dst], eax
			movd       edx, mm7
			mov        [eax-2], dx           ; deposit 1 output sample
			align      16
done1:
		  }
		} // for (int q = 0
		__asm { // Don't be a creep ;-)
		  mov       edx, this
		   mov      eax, dtberror            ; time increment error accumulator
		  mov       ecx, [edx].dtb           ; time increment
		   add      eax, [edx].dtbe          ; add error per cycle
		  mov       edx, dword ptr pos+4
		   cmp      eax, 0x80000000          ; accumulated 1 full error yet?
		   jb       nofix
		  inc       ecx                      ; += 1
		   sub      eax, 0x80000000          ; -= 1 full error
nofix:
		  add       ecx, dword ptr pos       ; Move to next sample
		   mov      dtberror, eax
		  adc       edx, 0
		   mov      dword ptr pos, ecx
		  shrd      ecx, edx, Np             ; posNp = pos >> Np
		   mov      dword ptr pos+4, edx
		  mov       posNp, ecx
		}
	  } // while (dst
	  __asm emms;
	}
	else 
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
    {
	  while (dst < dst_end) {
		for (int q = 0; q < ch; q++) {
		  short* Xp = &srcbuffer[(pos >> Np) * ch];
#if 1
		  __int64 v64 = FilterUD(Xp + q, (short)(pos & Pmask), - ch);  /* Perform left-wing inner product  */
		  v64 += FilterUD(Xp + ch + q, (short)(( -pos) & Pmask), ch);  /* Perform right-wing inner product */
          v64 += 1 << (Nh - 1);                                        /* Round only once!                 */
		  int v32 = int(v64 >> Nh);                                    /* Make guard bits once!            */
		  v32 *= LpScl;                                                /* Normalize for unity filter gain  */
		  *dst++ = IntToShort(v32, NLpScl);                            /* strip guard bits, deposit output */
#else
		  int v = FilterUD(Xp + q, (short)(pos & Pmask), - ch);  /* Perform left-wing inner product */
		  v += FilterUD(Xp + ch + q, (short)(( -pos) & Pmask), ch);  /* Perform right-wing inner product */
		  v >>= Nhg;      /* Make guard bits */
		  v *= LpScl;     /* Normalize for unity filter gain */
		  *dst++ = IntToShort(v, NLpScl);   /* strip guard bits, deposit output */
#endif
		}
		if ((dtberror += dtbe) >= (1 << 31)) { // Don't be a creep ;-)
		  dtberror -= (1 << 31);
		  pos += dtb + 1;   /* Move to next sample by time increment + error adjustment */
		}
		else {
		  pos += dtb;       /* Move to next sample by time increment */
		}
	  }
	}
  }
  else { // SAMPLE_FLOAT

	if (!fsrcbuffer || source_bytes > srcbuffer_size) {
	  if (fsrcbuffer)
		delete[] fsrcbuffer;
	  fsrcbuffer = new SFLOAT[source_bytes >> 2];
	  srcbuffer_size = source_bytes;
	  last_samples= 0;
	  last_start = 0;
	}

	const int offset = int((src_start >> Np) - Xoff - last_start);
	last_start = (src_start >> Np) - Xoff;
	
	int overlap = int(last_samples - offset);
	if ((offset < 0) || (overlap <= 0))
	  overlap = 0;
	else if (offset > 0)
	  memcpy(fsrcbuffer, fsrcbuffer+offset*ch, overlap*ch<<2);
	last_samples= std::max((__int64_t)overlap, source_samples);

    if (source_samples-overlap > 0)
	  child->GetAudio(&fsrcbuffer[overlap*ch], last_start+overlap, source_samples-overlap, env);

	SFLOAT* dst = (SFLOAT*)buf;

	SFLOAT* dst_end = &dst[count * ch];

	while (dst < dst_end) {
	  for (int q = 0; q < ch; q++) {
		SFLOAT* Xp = &fsrcbuffer[(pos >> Np) * ch];
		SFLOAT v = FilterUD(Xp +      q, (short)(   pos  & Pmask), - ch);  /* Perform left-wing inner product */
		v       += FilterUD(Xp + ch + q, (short)(( -pos) & Pmask),   ch);  /* Perform right-wing inner product */
		*dst++ = v;     /* deposit output */
	  }
	  if ((dtberror += dtbe) >= (1 << 31)) { // Don't be a creep ;-)
		dtberror -= (1 << 31);
		pos += dtb + 1;   /* Move to next sample by time increment + error adjustment */
	  }
	  else {
		pos += dtb;       /* Move to next sample by time increment */
	  }
	}

  }
}


AVSValue __cdecl ResampleAudio::Create(AVSValue args, void*, IScriptEnvironment* env) {
  return new ResampleAudio(args[0].AsClip(), args[1].AsInt(), args[2].AsInt(1), env);
}


// FilterUD MMX SAMPLE_INT16 Version -- approx 3.25 times faster than original (2.4x than new)
/*
 * MMx registers transfered across calls
 * MM7 - Accumulate the left/right wing inner product of the current sample pair
 * MM6 - Rounding constant 1 << (Na-1),     (64)
 * MM5 - Rounding constant 1 << (Nhxn-1), (8192)
 *
 * Uses MM0, MM1
 */
void FilterUD_mmx(short *Xp, unsigned Ph, int _inc, int _dhb, short *p_Imp, unsigned End) {

  unsigned Ho  = (Ph * (unsigned)_dhb) >> Np;
  
  if (_inc > 0) {   // If doing right wing drop extra coeff, so when Ph is
    End--;          // 0.5, we don't do one too many mult's
    if (Ph == 0)    // If the phase is zero then we've already skipped the
      Ho += _dhb;   // first sample, so we must also skip ahead in Imp[]
  }
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE  
__asm {
	mov			edi,[Xp]
	mov			esi,[Ho]
	mov			ecx,[p_Imp]

	mov			edx,esi		; Fold into end for improved pairing
	mov			eax,Amask
	shr			edx,Na				; Ho >> Na
	and			eax,esi				; Ho & Amask
	cmp			edx,[End]
	movd		mm1,Amasktab[eax*4]	; 0000 0000 eax 128-eax
	jae			donone

	align		16
loop1:
	 movd		mm0,[ecx+edx*2]		; 0000 0000 Imp[Ho>>Na7+1] Imp[Ho>>Na7]
	add			esi,[_dhb]			; Ho += dhb
	 pmaddwd	mm0,mm1				; 00000000 Imp[h+1]*a + Imp[h]*(128-a)
	movd		mm1,[edi]			; 0000 0000 *(Xp+1) *Xp
	 paddd		mm0,mm6				; += round
	add			edi,[_inc]			; Xp += Inc
	 pslld      mm0,16-Na			; <<= 16-Na
	mov			eax,Amask
	 psrld		mm0,16				; 0000 0000 0000 coeff
	punpcklwd	mm1,mm1				; *(Xp+1) *(Xp+1) *Xp *Xp 
	 mov 		edx,esi
	punpckldq	mm0,mm0				; 0000 coeff 0000 coeff
	 and		eax,esi				; Ho & Amask
	pmaddwd		mm0,mm1				; *(Xp+1)*coeff | *Xp*coeff
	 shr		edx,Na				; Ho >> Na
	paddd		mm0,mm5				; += round
	 movd		mm1,Amasktab[eax*4]	; 0000 0000 eax 128-eax
	psrad		mm0,Nhxn			; >>=Nhxn
	 cmp		edx,[End]
	paddd		mm7,mm0				; v += t
	 jb			loop1
donone:
  }
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
}


// FilterUD SAMPLE_INT16 Version
__int64 ResampleAudio::FilterUD(short *Xp, short Ph, short Inc) {
  __int64 v = 0;
  unsigned Ho = (Ph * (unsigned)dhb) >> Np;
  unsigned End = Nwing;

  if (Inc > 0)          /* If doing right wing...              */
  {                     /* ...drop extra coeff, so when Ph is  */
    End--;              /*    0.5, we don't do too many mult's */
    if (Ph == 0)        /* If the phase is zero...             */
      Ho += dhb;        /* ...then we've already skipped the   */
  }                     /*    first sample, so we must also    */
                        /*    skip ahead in Imp[] and ImpD[]   */
  while ((Ho >> Na) < End) {
    int t = Imp[Ho >> Na];                              /* Get IR sample */
#if 1
	// It's 37% faster and more accurate to accumulate 64 bits
	// than stuffing around testing, rounding and shifting
    const int a = Ho & Amask;                           /* a is logically between 0 and 1 */
    const int r = 1 << (Na-1);                          /* Round */
    t += ((int(Imp[(Ho>>Na)+1]) - t) * a + r) >> Na;    /* t is now interp'd filter coeff */
    t *= *Xp;                                           /* Mult coeff by input sample */
#else
    int a = Ho & Amask;   /* a is logically between 0 and 1 */
    t += ((int(Imp[(Ho >> Na) + 1]) - t) * a) >> Na; /* t is now interp'd filter coeff */
    t *= *Xp;     /* Mult coeff by input sample */
    if (t & 1 << (Nhxn - 1))  /* Round, if needed */
      t += 1 << (Nhxn - 1);
    t >>= Nhxn;       /* Leave some guard bits, but come back some */
#endif
    v += t;           /* The filter output */
    Ho += dhb;        /* IR step */
    Xp += Inc;        /* Input signal step. NO CHECK ON BOUNDS */
  }
  return (v);
}

// FilterUD SAMPLE_FLOAT Version  -- Approx same speed as new int16 and SSRC on P4 (40% on P2)
SFLOAT ResampleAudio::FilterUD(SFLOAT *Xp, short Ph, short Inc) {
  SFLOAT v = 0;
  unsigned Ho = (Ph * (unsigned)dhb) >> Np;
  unsigned End = Nwing;
  if (Inc > 0)        /* If doing right wing...              */
  {                   /* ...drop extra coeff, so when Ph is  */
    End--;            /*    0.5, we don't do too many mult's */
    if (Ph == 0)      /* If the phase is zero...             */
      Ho += dhb;      /* ...then we've already skipped the   */
  }                   /*    first sample, so we must also    */
                      /*    skip ahead in fImp[]             */
  while ((Ho >> Na) < End) {
    SFLOAT t = fImp[Ho >> Na];      /* Get IR sample */
    t += (fImp[(Ho >> Na) + 1] - t) * fAmasktab[Ho & Amask]; /* t is now interpolated filter coeff */
    t *= *Xp;         /* Mult coeff by input sample */
    v += t;           /* The filter output */
    Ho += dhb;        /* IR step */
    Xp += Inc;        /* Input signal step. NO CHECK ON BOUNDS */
  }
  return (v);
}




/********************************
 *******   Helper methods *******
 ********************************/

double Izero(double x) {
  double sum, u, halfx, temp;
  int n;

  sum = u = n = 1;
  halfx = x / 2.0;
  do {
    temp = halfx / (double)n;
    n += 1;
    temp *= temp;
    u *= temp;
    sum += u;
  } while (u >= IzeroEPSILON*sum);
  return (sum);
}


void LpFilter(double c[], int N, double frq, double Beta, int Num) {
  double IBeta, temp, inm1;
  int i;

  /* Calculate ideal lowpass filter impulse response coefficients: */
  c[0] = 2.0 * frq;
  for (i = 1; i < N; i++) {
    temp = PI * (double)i / (double)Num;
    c[i] = sin(2.0 * temp * frq) / temp; /* Analog sinc function, cutoff = frq */
  }

  /*
   * Calculate and Apply Kaiser window to ideal lowpass filter.
   * Note: last window value is IBeta which is NOT zero.
   * You're supposed to really truncate the window here, not ramp
   * it to zero. This helps reduce the first sidelobe.
   */
  IBeta = 1.0 / Izero(Beta);
  inm1 = 1.0 / ((double)(N - 1));
  for (i = 1; i < N; i++) {
    temp = (double)i * inm1;
    c[i] *= Izero(Beta * sqrt(1.0 - temp * temp)) * IBeta;
  }
}


/* ERROR return codes:
 *    0 - no error
 *    1 - Nwing too large (Nwing is > MAXNWING)
 *    2 - Froll is not in interval [0:1)
 *    3 - Beta is < 1.0
 *
 */

// makeFilter SAMPLE_INT16 Version
int makeFilter( short Imp[], double *dLpScl, unsigned short Nwing, double Froll, double Beta) {
  static const int MAXNWING = 8192;
  static double ImpR[MAXNWING];

  double DCgain, Scl, Maxh;
  short Dh;
  int i;

  if (Nwing > MAXNWING)                      /* Check for valid parameters */
    return (1);
  if ((Froll <= 0) || (Froll > 1))
    return (2);
  if (Beta < 1)
    return (3);

  /*
   * Design Kaiser-windowed sinc-function low-pass filter
   */
  LpFilter(ImpR, (int)Nwing, 0.5*Froll, Beta, Npc);

  /* Compute the DC gain of the lowpass filter, and its maximum coefficient
   * magnitude. Scale the coefficients so that the maximum coeffiecient just
   * fits in Nh-bit fixed-point, and compute LpScl as the NLpScl-bit (signed)
   * scale factor which when multiplied by the output of the lowpass filter
   * gives unity gain. */
  DCgain = 0;
  Dh = Npc;                       /* Filter sampling period for factors>=1 */
  for (i = Dh; i < Nwing; i += Dh)
    DCgain += ImpR[i];
  DCgain = 2 * DCgain + ImpR[0];    /* DC gain of real coefficients */

  for (Maxh = i = 0; i < Nwing; i++)
    Maxh = std::max(Maxh, fabs(ImpR[i]));

  Scl = ((1 << (Nh - 1)) - 1) / Maxh;     /* Map largest coeff to 16-bit maximum */
  *dLpScl = fabs((1 << (NLpScl + Nh)) / (DCgain * Scl));

  /* Scale filter coefficients for Nh bits and convert to integer */
  if (ImpR[0] < 0)                /* Need pos 1st value for LpScl storage */
    Scl = -Scl;
  for (i = 0; i < Nwing; i++)         /* Scale & round them */
    Imp[i] = int(ImpR[i] * Scl + 0.5);

  return (0);
}


// makeFilter SAMPLE_FLOAT Version
int makeFilter( SFLOAT fImp[], double dLpScl, unsigned short Nwing, double Froll, double Beta) {
  static const int MAXNWING = 8192;
  static double ImpR[MAXNWING];

  double DCgain, Scl;
  int i;

  if (Nwing > MAXNWING)                      /* Check for valid parameters */
    return (1);
  if ((Froll <= 0) || (Froll > 1))
    return (2);
  if (Beta < 1)
    return (3);

  /*
   * Design Kaiser-windowed sinc-function low-pass filter
   */
  LpFilter(ImpR, (int)Nwing, 0.5*Froll, Beta, Npc);

  /* Compute the DC gain of the lowpass filter, and its maximum coefficient
   * magnitude. Scale the coefficients so that the maximum coeffiecient just
   * fits in Nh-bit fixed-point, and compute LpScl as the NLpScl-bit (signed)
   * scale factor which when multiplied by the output of the lowpass filter
   * gives unity gain. */
  DCgain = 0;
  /* Npc is filter sampling period for factors>=1 */
  for (i = Npc; i < Nwing; i += Npc)
    DCgain += ImpR[i];
  DCgain = 2 * DCgain + ImpR[0];    /* DC gain of real coefficients */

  Scl = dLpScl / DCgain;

  /* Scale filter coefficients for unity gain and convert to float */
  if (ImpR[0] < 0)                /* Need pos 1st value for LpScl storage */
    Scl = -Scl;
  for (i = 0; i < Nwing; i++)         /* Scale them */
    fImp[i] = ImpR[i] * Scl;

  return (0);
}

}; // namespace avxsynth
