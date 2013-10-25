// Avisynth v4.0  Copyright 2007 Ben Rudiak-Gould et al.
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

#ifndef __Cache_H__
#define __Cache_H__

#include "internal.h"

namespace avxsynth {
/********************************************************************
********************************************************************/



class Cache : public GenericVideoFilter
/**
  * Manages a video frame cache
 **/
{
  friend class ScriptEnvironment;

public:
  Cache(PClip _child, IScriptEnvironment* env);
  ~Cache();
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  void __stdcall SetCacheHints(int cachehints,int frame_range);
  static AVSValue __cdecl Create_Cache(AVSValue args, void*, IScriptEnvironment* env);
  void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env);

protected:
  Cache *nextCache, **priorCache;

  enum PC_Keys {
    PC_Nil=0,
    PC_UnlockOld,
    PC_UnlockAll,
    PC_UnProtect,
    PC_UnProtectAll
  };

  void PokeCache(int key, int size, IScriptEnvironment* env);

private:
  enum {GetMyThis = 0x8666 };

  struct CachedVideoFrame;
  void RegisterVideoFrame(CachedVideoFrame *i, const PVideoFrame& frame, int n, IScriptEnvironment* env);
  void FillZeros(void* buf, int start_offset, int count);
  void ResetCache(IScriptEnvironment* env);
  void ReturnVideoFrameBuffer(CachedVideoFrame *i, IScriptEnvironment* env);
  CachedVideoFrame* GetACachedVideoFrame(const PVideoFrame& frame, IScriptEnvironment* env);
  VideoFrame* BuildVideoFrame(CachedVideoFrame *i, int n);
  void LockVFB(CachedVideoFrame *i);
  void UnlockVFB(CachedVideoFrame *i);
  void ProtectVFB(CachedVideoFrame *i, int n);
  void UnProtectVFB(CachedVideoFrame *i);
  PVideoFrame __stdcall childGetFrame(int n, IScriptEnvironment* env);

  struct CachedVideoFrame 
  {
    CachedVideoFrame *prev, *next;
    VideoFrameBuffer* vfb;
    int sequence_number;
    int offset, pitch, row_size, height, offsetU, offsetV, pitchUV;
    int frame_number;
    int faults;  // the number of times this frame was requested and found to be stale(modified)
    bool vfb_locked;
    bool vfb_protected;

    CachedVideoFrame() { 
        next=prev=this; 
        vfb=0; 
        frame_number=-1; 
        vfb_locked=false;
        vfb_protected=false;
    }
  };
  CachedVideoFrame video_frames;

  // Video cache
  int h_policy;
  int h_span;
  long protectcount;

  // Audio cache:
  int h_audiopolicy;
  char * cache;
  int samplesize;
  int maxsamplecount;
  __int64 cache_start;
  __int64 cache_count;

  // For audio cache prediction
  __int64 ac_expected_next;
  int ac_currentscore;
  int ac_too_small_count;

  // Cached range limits
  int minframe, maxframe;
  int cache_init;   // The Initial cache size
  int cache_limit;  // 16 time the current maximum number of CachedVideoFrame entries
  int fault_rate;   // A decaying average of 100 times the peak fault count, used to control vfb auto-locking
  int miss_count;   // Count of consecutive cache misses

  unsigned long Tick;
  // These are global to all Cache instances
  static unsigned long Clock;
  static long cacheDepth;
};

}; // namespace avxsynth
#endif  // __Cache_H__
