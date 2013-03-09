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

#ifndef __Edit_H__
#define __Edit_H__

#include "common/include/internal.h"
#include "builtinfunctions/src/filters/merge.h"

namespace avxsynth {
	
/********************************************************************
********************************************************************/

class Trim : public GenericVideoFilter 
/**
  * Class to select a range of frames from a longer clip
 **/
{
public:
  Trim(int _firstframe, int _lastframe, bool _padaudio, PClip _child, IScriptEnvironment* env);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env);
  bool __stdcall GetParity(int n);

  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);  

private:
  int firstframe;
  __int64 audio_offset;
};




class FreezeFrame : public GenericVideoFilter 
/**
  * Class to display a single frame for the duration of several
 **/
{
public:
  FreezeFrame(int _first, int _last, int _source, PClip _child);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  bool __stdcall GetParity(int n);

  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env); 

private:
  const int first, last, source;
};




class DeleteFrame : public GenericVideoFilter 
/**
  * Class to delete a frame
 **/
{  
public:
  DeleteFrame(int _frame, PClip _child);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  bool __stdcall GetParity(int n);

  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);

private:
  const int frame;
};




class DuplicateFrame : public GenericVideoFilter 
/**
  * Class to duplicate a frame
 **/
{  
public:
  DuplicateFrame(int _frame, PClip _child);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  bool __stdcall GetParity(int n);

  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);

private:
  const int frame;
};




class Splice : public GenericVideoFilter 
/**
  * Class to splice together video clips
 **/
{
public:
  Splice(PClip _child1, PClip _child2, bool realign_sound, IScriptEnvironment* env);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env);
  bool __stdcall GetParity(int n);

  static AVSValue __cdecl CreateUnaligned(AVSValue args, void*, IScriptEnvironment* env);
  static AVSValue __cdecl CreateAligned(AVSValue args, void*, IScriptEnvironment* env);

private:
  PClip child2;
  int video_switchover_point;
  __int64 audio_switchover_point;
};




class Dissolve : public GenericVideoFilter 
/**
  * Class to smoothly transition from one video clip to another
 **/
{
public:
  Dissolve(PClip _child1, PClip _child2, int _overlap, double fps, IScriptEnvironment* env);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env);
  bool __stdcall GetParity(int n);

  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);

private:
  PClip child2;
  const int overlap;
  int video_fade_start, video_fade_end;
  __int64 audio_fade_start, audio_fade_end;
  int audio_overlap;
  BYTE* audbuffer;
  int audbufsize;
  void EnsureBuffer(int minsize);
};




class AudioDub : public IClip {  
/**
  * Class to mux the audio track of one clip with the video of another
 **/
public:
  AudioDub(PClip child1, PClip child2, size_t mode, IScriptEnvironment* env);
  const VideoInfo& __stdcall GetVideoInfo();
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env);
  bool __stdcall GetParity(int n);
  void __stdcall SetCacheHints(int cachehints,size_t frame_range) { };

  static AVSValue __cdecl Create(AVSValue args, void* mode, IScriptEnvironment* env);

private:
  /*const*/ PClip vchild, achild;
  VideoInfo vi;
};




class Reverse : public GenericVideoFilter 
/**
  * Class to play a clip backwards
 **/
{
public:
  Reverse(PClip _child);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  bool __stdcall GetParity(int n);
  void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env);

  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);
};




class Loop : public GenericVideoFilter {
/**
  * Class to loop over a range of frames
**/
public:
	Loop(PClip _child, int times, int _start, int _end, IScriptEnvironment* env);
	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
	bool __stdcall GetParity(int n);
  void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env);

	static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);
private:
	int   frames,    start,     end;
  __int64 aud_count, aud_start, aud_end;
	int convert(int n);
};




/**** A few factory methods ****/

AVSValue __cdecl Create_FadeOut0(AVSValue args, void*, IScriptEnvironment* env);
AVSValue __cdecl Create_FadeOut(AVSValue args, void*, IScriptEnvironment* env);
AVSValue __cdecl Create_FadeOut2(AVSValue args, void*, IScriptEnvironment* env);
AVSValue __cdecl Create_FadeIn0(AVSValue args, void*, IScriptEnvironment* env);
AVSValue __cdecl Create_FadeIn(AVSValue args, void*, IScriptEnvironment* env);
AVSValue __cdecl Create_FadeIn2(AVSValue args, void*,IScriptEnvironment* env);
AVSValue __cdecl Create_FadeIO0(AVSValue args, void*,IScriptEnvironment* env);
AVSValue __cdecl Create_FadeIO(AVSValue args, void*,IScriptEnvironment* env);
AVSValue __cdecl Create_FadeIO2(AVSValue args, void*,IScriptEnvironment* env);

PClip new_Splice(PClip _child1, PClip _child2, bool realign_sound, IScriptEnvironment* env);

}; // namespace avxsynth
#endif  // __Edit_H__
