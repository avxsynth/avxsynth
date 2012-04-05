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

#ifndef __Combine_H__
#define __Combine_H__

#include "../internal.h"

namespace avxsynth {
	
/********************************************************************
********************************************************************/


class StackVertical : public IClip 
/**
  * Class to stack clips vertically
 **/
{
public:
  StackVertical(PClip _child1, PClip _child2, IScriptEnvironment* env);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  
  inline void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) 
    { child1->GetAudio(buf, start, count, env); }
  inline const VideoInfo& __stdcall GetVideoInfo() 
    { return vi; }
  inline bool __stdcall GetParity(int n) 
    { return child1->GetParity(n); }
  void __stdcall SetCacheHints(int cachehints,size_t frame_range) { };

  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);

private:
  /*const*/ PClip child1, child2;
  VideoInfo vi;

};



class StackHorizontal : public IClip 
/**
  * Class to stack clips vertically
 **/
{  
public:
  StackHorizontal(PClip _child1, PClip _child2, IScriptEnvironment* env);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);

  inline void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) 
    { child1->GetAudio(buf, start, count, env); }
  inline const VideoInfo& __stdcall GetVideoInfo() 
    { return vi; }
  inline bool __stdcall GetParity(int n) 
    { return child1->GetParity(n); }

  void __stdcall SetCacheHints(int cachehints,size_t frame_range) { };
  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);

private:
  const PClip child1, child2;
  VideoInfo vi;

};




class ShowFiveVersions : public IClip 
/**
  * Class to show every pulldown combination
 **/
{  
public:
  ShowFiveVersions(PClip* children, IScriptEnvironment* env);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);

  inline void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) 
    { child[0]->GetAudio(buf, start, count, env); }
  inline const VideoInfo& __stdcall GetVideoInfo() 
    { return vi; }
  inline bool __stdcall GetParity(int n) 
    { return child[0]->GetParity(n); }
  void __stdcall SetCacheHints(int cachehints,size_t frame_range) { };

  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);

private:
  PClip child[5];
  VideoInfo vi;

};



class Animate : public IClip 
/**
  * Class to allow recursive animation of multiple clips (see docs)  *
 **/
{
  enum { cache_size = 3 };
  PClip cache[cache_size];
  int cache_stage[cache_size];
  const int first, last;
  AVSValue *args_before, *args_after, *args_now;
  int num_args;
  const char* name;
  bool range_limit;
public:
  Animate( PClip context, int _first, int _last, const char* _name, const AVSValue* _args_before, 
           const AVSValue* _args_after, int _num_args, bool _range_limit, IScriptEnvironment* env );
  virtual ~Animate() 
    { delete[] args_before; }
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env);

  inline const VideoInfo& __stdcall GetVideoInfo() 
    { return cache[0]->GetVideoInfo(); }
  bool __stdcall GetParity(int n);

  void __stdcall SetCacheHints(int cachehints,size_t frame_range) { };

  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);
  static AVSValue __cdecl Create_Range(AVSValue args, void*, IScriptEnvironment* env);
};

}; // namespace avxsynth
#endif  // __Combine_H__
