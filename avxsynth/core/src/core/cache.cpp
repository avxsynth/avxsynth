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


#include "stdafx.h"
#include "cache.h"
#include "avxlog.h"

namespace avxsynth { 
  
#ifdef MODULE_NAME
#undef MODULE_NAME
#endif

#define MODULE_NAME core::cache

// Global statistics counters
typedef struct {
  unsigned long resets;
  unsigned long vfb_found;
  unsigned long vfb_modified;
  unsigned long vfb_stolen;
  unsigned long vfb_notfound;
  unsigned long vfb_never;
  long          vfb_locks;
  long          vfb_protects;
  char tag[72];
} CACHE_STATS;

CACHE_STATS g_Cache_stats = {0, 0, 0, 0, 0, 0, 0, 0, "resets, vfb_[found,modified,stolen,notfound,never,locks,protects]"};


AVSFunction Cache_filters[] = {
  { "Cache", "c", Cache::Create_Cache },
  { "InternalCache", "c", Cache::Create_Cache },                    
  { 0 }
};
 

enum {CACHE_SCALE_FACTOR      =  16}; // Granularity fraction for cache size - 1/16th
enum {MAX_CACHE_MISSES        = 100}; // Consecutive misses before a reset
enum {MAX_CACHED_VIDEO_FRAMES = 200}; // Maximum number of VFB's that are tracked
enum {MAX_CACHE_RANGE         =  21}; // Maximum CACHE_RANGE span i.e. number of protected VFB's


/*******************************
 *******   Cache filter   ******
 ******************************/


unsigned long Cache::Clock = 1;
long Cache::cacheDepth = 0;


Cache::Cache(PClip _child, IScriptEnvironment* env) 
 : GenericVideoFilter(_child), nextCache(NULL), priorCache(NULL), cache(0), Tick(0)
{ 
  h_policy = CACHE_ALL;  // Since hints are not used per default, this is to describe the lowest default cache mode.
  h_audiopolicy = CACHE_NOTHING;  // Don't cache audio per default.

  cache_limit = CACHE_SCALE_FACTOR / 2;  // Start half way towards 1 buffer
  cache_init  = 0;
  fault_rate = 0;
  miss_count = 0x80000000; // Hugh negative

//h_total_frames = 0; // Hint cache not init'ed
  h_span = 0;
  protectcount = 0;

  ac_expected_next = 0;
  ac_too_small_count = 0;
  ac_currentscore = 100;

  maxframe = -1;
  minframe = vi.num_frames;

  samplesize = vi.BytesPerAudioSample();

  // Join all the rest of the caches
  env->ManageCache(MC_RegisterCache, this);
}


// Generic interface to poke all cache instances
void Cache::PokeCache(int key, int data, IScriptEnvironment* env)
{
  switch (key) {
    case PC_Nil: { // Do Nothing!
      return;
    }
    case PC_UnlockOld: { // Unlock head vfb
      // Release the head vfb if it is locked and this
      // instance is not with the current GetFrame chain.
      if ((Tick != Clock) && video_frames.next->vfb_locked) {
        UnlockVFB(video_frames.next);
        AVXLOG_INFO("Cache:%x: PokeCache UnlockOld vfb %x, frame %d",
              this, video_frames.next->vfb, video_frames.next->frame_number);
      }
      break;
    }
    case PC_UnlockAll: { // Unlock all vfb's if the vfb size is big enough to satisfy
      for (CachedVideoFrame* i = video_frames.next; i != &video_frames; i = i->next) {
        if (i->vfb_locked && i->vfb->data_size >= data) {
          UnlockVFB(i);
          AVXLOG_INFO("Cache:%x: PokeCache UnlockAll vfb %x, frame %d",
                this, video_frames.next->vfb, video_frames.next->frame_number);
        }
      }
      break;
    }
    case PC_UnProtect: {   // Unprotect 1 vfb if this instance is
      if (Tick != Clock) { // not with the current GetFrame chain.
        for (CachedVideoFrame* i = video_frames.next; i != &video_frames; i = i->next) {
          // Unprotect the youngest because it might be the easiest
          // to regenerate from parent caches that are still current.
          // And to give it a fair chance we also promote it.
          if (i->vfb_protected && i->vfb->data_size >= data) {
            UnlockVFB(i);
            UnProtectVFB(i);
            env->ManageCache(MC_PromoteVideoFrameBuffer, i->vfb);
            AVXLOG_INFO("Cache:%x: PokeCache UnProtect vfb %x, frame %d",
                  this, video_frames.next->vfb, video_frames.next->frame_number);
            break;
          }
        }
      }
      break;
    }
    case PC_UnProtectAll: { // Unprotect all vfb's
      for (CachedVideoFrame* i = video_frames.next; i != &video_frames; i = i->next) {
        if (i->vfb_protected && i->vfb->data_size >= data) {
          UnlockVFB(i);
          UnProtectVFB(i);
          env->ManageCache(MC_PromoteVideoFrameBuffer, i->vfb);
          AVXLOG_INFO("Cache:%x: PokeCache UnProtectAll vfb %x, frame %d",
                this, video_frames.next->vfb, video_frames.next->frame_number);
        }
      }
      break;
    }
    default:
      return;
  }
  // Poke the next Cache in the chain
  if (nextCache) nextCache->PokeCache(key, data, env);
}


/*********** V I D E O   C A C H E ************/


// Any vfb's that are still current we give back for earlier reuse
void Cache::ReturnVideoFrameBuffer(CachedVideoFrame *i, IScriptEnvironment* env)
{
	if (i->vfb_protected) UnProtectVFB(i);
	if (i->vfb_locked) UnlockVFB(i);

	// A vfb purge has occured
	if (!i->vfb) return;

	// if vfb is not current (i.e ours) leave it alone
	if (i->vfb->GetSequenceNumber() != i->sequence_number) return;

	// return vfb to vfb pool for immediate reuse
    env->ManageCache(MC_ReturnVideoFrameBuffer, i->vfb);
}


// If the cache is not being used reset it
void Cache::ResetCache(IScriptEnvironment* env)
{
  ++g_Cache_stats.resets;
  minframe = vi.num_frames;
  maxframe = -1;
  CachedVideoFrame *i, *j;

  AVXLOG_DEBUG("Cache:%x: Cache Reset, cache_limit %d, cache_init %d", this, cache_limit, CACHE_SCALE_FACTOR*cache_init);

  int count=0;
  for (i = video_frames.next; i != &video_frames; i = i->next) {
	if (++count >= cache_init) goto purge_old_frame;

    const int ifn = i->frame_number;
    if (ifn < minframe) minframe = ifn;
    if (ifn > maxframe) maxframe = ifn;
  }
  return;

purge_old_frame:

  // Truncate the tail of the chain
  j = i->next;
  video_frames.prev = i;
  i->next = &video_frames;
  
  // Delete the excess CachedVideoFrames
  while (j != &video_frames) {
	i = j->next;
	ReturnVideoFrameBuffer(j, env); // Return old vfb to vfb pool for early reuse
	delete j;
	j = i;
  }
  cache_limit = CACHE_SCALE_FACTOR*cache_init;
}


// This seems to be an excelent place for catching out of bounds violations
// on VideoFrameBuffer's. Well at least some of them, VFB's have a small
// variable margin due to alignment constraints. Here we catch those accesses
// that violate that, for those are the ones that are going to cause problems.


PVideoFrame __stdcall Cache::childGetFrame(int n, IScriptEnvironment* env) 
{ 
  InterlockedIncrement(&cacheDepth);
  PVideoFrame result = child->GetFrame(n, env);
  InterlockedDecrement(&cacheDepth);

  if (!result)
    env->ThrowError("Cache: Filter returned NULL PVideoFrame");

#ifdef _DEBUG
  int *p=(int *)(result->vfb->data);
  if ((p[-4] != 0xDEADBEAF)
   || (p[-3] != 0xDEADBEAF)
   || (p[-2] != 0xDEADBEAF)
   || (p[-1] != 0xDEADBEAF)) {
    env->ThrowError("Debug Cache: Write before start of VFB! Addr=%x", p);
  }
  p=(int *)(result->vfb->data + result->vfb->data_size);
  if ((p[0] != 0xDEADBEAF)
   || (p[1] != 0xDEADBEAF)
   || (p[2] != 0xDEADBEAF)
   || (p[3] != 0xDEADBEAF)) {
    env->ThrowError("Debug Cache: Write after end of VFB! Addr=%x", p);
  }
#endif
  return result;
}


// Unfortunatly the code for "return child->GetFrame(n,env);" seems highly likely 
// to generate code that relies on the contents of the ebx register being preserved
// across the call. By inserting a "mov ebx,ebx" before the call the compiler can
// be convinced the ebx register has been changed and emit altermate code that is not
// dependant on the ebx registers contents.
//
// The other half of the problem is when using inline assembler, __asm, that uses the
// ebx register together with a medium amount of C++ code around it, the optimizer
// dutifully restructures the emited C++ code to remove all it's ebx references, forgets
// about the __asm ebx references, and thinks it's okay to removes the push/pop ebx 
// from the entry prologue.
//
// Together they are a smoking gun!


PVideoFrame __stdcall Cache::GetFrame(int n, IScriptEnvironment* env) 
{ 
  if (cacheDepth == 0) Clock+=1;
  Tick = Clock;

  n = std::min(vi.num_frames-1, std::max(0,n));  // Inserted to avoid requests beyond framerange.

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE  
  __asm {emms} // Protection from rogue filter authors
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE

  if (h_policy == CACHE_NOTHING) { // don't want a cache. Typically filters that only ever seek forward.
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
    __asm mov ebx,ebx  // Hack! prevent compiler from trusting ebx contents across call
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
    return childGetFrame(n, env);
  }

  if (video_frames.next->vfb_locked) {  // release the head vfb if it is locked
	UnlockVFB(video_frames.next);
	AVXLOG_INFO("Cache:%x: unlocking vfb %x for frame %d", this, video_frames.next->vfb, video_frames.next->frame_number);
  }

  CachedVideoFrame* cvf=0;

  // look for a cached copy of the frame
  if (n>=minframe && n<=maxframe) { // Is this frame in the range we are tracking
	miss_count = 0; // Start/Reset cache miss counter

	int c=0;
	int imaxframe = -1;
	int iminframe = vi.num_frames;
	bool found = false;

	// Scan the cache for a candidate
	for (CachedVideoFrame* i = video_frames.next; i != &video_frames; i = i->next) {
	  ++c;
	  const int ifn = i->frame_number;

	  // Ahah! Our frame
	  if (ifn == n) {
		found = true;

		if (!i->vfb_locked)
		  LockVFB(i);  // Lock to be sure this frame isn't updated.

		// We have a hit make sure cache_limit is at least this wide
		if (cache_limit < c * CACHE_SCALE_FACTOR) cache_limit = c * CACHE_SCALE_FACTOR;

		// And it hasn't been screwed, Excellent!
		if (i->sequence_number == i->vfb->GetSequenceNumber()) {
		  ++g_Cache_stats.vfb_found;
		  return BuildVideoFrame(i, n); // Success!
		}

		// Damn! The contents have changed
		// record the details, update the counters
		++i->faults;
		fault_rate += 30 + c; // Bias by number of cache entries searched
		if (100*i->faults > fault_rate) fault_rate = 100*i->faults;
		if (fault_rate > 300) fault_rate = 300;
		AVXLOG_INFO("Cache:%x: stale frame %d, requests %d", this, n, i->faults);

		// Modified by a parent
		if (i->sequence_number == i->vfb->GetSequenceNumber()-1) {
		  ++g_Cache_stats.vfb_modified;
		}
		// vfb has been stolen
		else {
		  ++g_Cache_stats.vfb_stolen;
		  AVXLOG_INFO("Cache:%x: stolen vfb %x, frame %d", this, i->vfb, n);
		}

		if (i->vfb_protected) UnProtectVFB(i);
		UnlockVFB(i);

		cvf = i; // Remember this entry!
		break;
	  } // if (ifn == n) 

	  if (ifn < iminframe) iminframe = ifn;
	  if (ifn > imaxframe) imaxframe = ifn;

	  // Unprotect any frames out of CACHE_RANGE scope
	  if ((i->vfb_protected) && (abs(ifn-n) >= h_span)) {
		UnProtectVFB(i);
		AVXLOG_INFO("Cache:%x: A: Unprotect vfb %x for frame %d", this, i->vfb, ifn);
	  }

	  if (i->vfb_locked) {  // release the vfb if it is locked
		UnlockVFB(i);
		AVXLOG_INFO("Cache:%x: B. unlock vfb %x for frame %d", this, i->vfb, ifn);
	  }
	} // for (CachedVideoFrame* i =

	if (!found) { // Cache miss - accumulate towards extra buffers
	  ++g_Cache_stats.vfb_notfound;
	  int span = 1 + imaxframe-iminframe - cache_limit/CACHE_SCALE_FACTOR;
	  if (span > CACHE_SCALE_FACTOR)
		cache_limit += CACHE_SCALE_FACTOR; // Add upto one whole buffer at a time
	  else if (span >  0)
		cache_limit += span; // Add span 16ths towards another buffer

	  maxframe = imaxframe; // update the limits from what is currently cached
	  minframe = iminframe;
	}

	if (cache_limit > CACHE_SCALE_FACTOR*MAX_CACHED_VIDEO_FRAMES) cache_limit = CACHE_SCALE_FACTOR*MAX_CACHED_VIDEO_FRAMES;

	AVXLOG_DEBUG("Cache:%x: size %d, limit %d, fault %d", this, c, cache_limit, fault_rate);

  } // if (n>=minframe
  else { // This frame is not in the range we are currently tracking
	++g_Cache_stats.vfb_never;
	if (++miss_count > MAX_CACHE_MISSES) {
	  ResetCache(env);  // The cache isn't being accessed, reset it!
	  miss_count = 0x80000000; // Hugh negative
	}
  } // if (n>=minframe ... else
  
  if (fault_rate > 0) --fault_rate;  // decay fault rate

  AVXLOG_DEBUG("Cache:%x: generating frame %d, cache from %d to %d", this, n, minframe, maxframe);

  // not cached; make the filter generate it.
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
  __asm mov ebx,ebx  // Hack! prevent compiler from trusting ebx contents across call
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
  PVideoFrame result = childGetFrame(n, env);

  // When we have the possibility of cacheing, promote
  // the vfb to the head of the LRU list, CACHE_RANGE
  // frames are NOT promoted hence they are fair game
  // for reuse as soon as they are unprotected. This
  // is a fair price to pay for their protection. If
  // a 2nd filter is hiting the cache outside the radius
  // of protection then we do promote protected frames.

  if (cache_limit/CACHE_SCALE_FACTOR > h_span)
	env->ManageCache(MC_PromoteVideoFrameBuffer, result->vfb);
  else
	env->ManageCache(MC_ManageVideoFrameBuffer, result->vfb);

  if (!cvf) cvf=GetACachedVideoFrame(result, env);

  RegisterVideoFrame(cvf, result, n, env);

  // If this is a CACHE_RANGE frame protect it
  if (h_span) {
	ProtectVFB(cvf, n);
  }
  // If we have asked for a same stale frame twice, lock frames.
  else if (  (fault_rate >  100) // Generated frames are subject to locking at a lower fault rate
	 && (fault_rate != 130) ) {  // Throw an unlocked probing frame at the world occasionally
	                             // Once we start locking frames we cannot tell if locking is
								 // still required. So probing is a cheap way to validate this.
	LockVFB(cvf);                // Increment to be sure this frame isn't modified.
	AVXLOG_INFO("Cache:%x: lock vfb %x, gened frame %d", this, cvf->vfb, n);
  }

  return result;
}


VideoFrame* Cache::BuildVideoFrame(CachedVideoFrame *i, int n)
{
  Relink(&video_frames, i, video_frames.next);   // move the matching cache entry to the front of the list
  VideoFrame* result = new VideoFrame(i->vfb, i->offset, i->pitch, i->row_size, i->height, i->offsetU, i->offsetV, i->pitchUV);

  // If we have asked for any same stale frame twice, leave frames locked.
  if (  (fault_rate <= 160)     // Reissued frames are not subject to locking at the lower fault rate
     || (fault_rate == 190) ) { // Throw an unlocked probing frame at the world occasionally
	UnlockVFB(i);
	AVXLOG_INFO("Cache:%x: using cached copy of frame %d", this, n);
  }
  else {
	AVXLOG_INFO("Cache:%x: lock vfb %x, cached frame %d", this, i->vfb, n);
  }
  return result;
}



Cache::CachedVideoFrame* Cache::GetACachedVideoFrame(const PVideoFrame& frame, IScriptEnvironment* env)
{
  CachedVideoFrame *i, *j;
  int count=0;

  // scan forwards for the last protected CachedVideoFrame
  for (i = video_frames.next; i != &video_frames; i = i->next) {
    if (i->vfb == frame->vfb) return i; // Don't fill cache with 100's copies of a Blankclip vfb
	count += 1;
	if ((count > h_span) && (count >= cache_limit/CACHE_SCALE_FACTOR)) break;
  }

  // scan backwards for a used CachedVideoFrame
  for (j = video_frames.prev; j != i; j = j->prev) {
    if (j->vfb == frame->vfb) return j; // Don't fill cache with 100's copies of a Blankclip vfb
    if (j->sequence_number != j->vfb->GetSequenceNumber()) return j;
    ReturnVideoFrameBuffer(j, env); // Return out of range vfb to vfb pool for early reuse
	++count;
  }

  if (count >= MAX_CACHED_VIDEO_FRAMES) return video_frames.prev; // to many entries just steal the oldest CachedVideoFrame

  return new CachedVideoFrame; // need a new one
}


void Cache::RegisterVideoFrame(CachedVideoFrame *i, const PVideoFrame& frame, int n, IScriptEnvironment* env) 
{
  ReturnVideoFrameBuffer(i, env); // Return old vfb to vfb pool for early reuse

  // copy all the info
  i->vfb = frame->vfb;
  i->sequence_number = frame->vfb->GetSequenceNumber();
  i->offset = frame->offset;
  i->offsetU = frame->offsetU;
  i->offsetV = frame->offsetV;
  i->pitch = frame->pitch;
  i->pitchUV = frame->pitchUV;
  i->row_size = frame->row_size;
  i->height = frame->height;
  // Keep any fault history
  if (i->frame_number != n) {
    i->frame_number = n;
    i->faults = 0;
  }

  // move the newly-registered frame to the front
  Relink(&video_frames, i, video_frames.next);

  // update the limits for cached entries
  if (n < minframe) minframe = n;
  if (n > maxframe) maxframe = n;
}


void Cache::LockVFB(CachedVideoFrame *i)
{
  if (!!i->vfb && !i->vfb_locked) {
	i->vfb_locked = true;
	InterlockedIncrement(&i->vfb->refcount);
	++g_Cache_stats.vfb_locks;
  }
}


void Cache::UnlockVFB(CachedVideoFrame *i)
{
  if (!!i->vfb && i->vfb_locked) {
	i->vfb_locked = false;
	InterlockedDecrement(&i->vfb->refcount);
	--g_Cache_stats.vfb_locks;
  }
}

void Cache::ProtectVFB(CachedVideoFrame *i, int n)
{
  CachedVideoFrame* j = video_frames.prev;

  if (!!i->vfb && !i->vfb_protected) {
	InterlockedIncrement(&protectcount);
	i->vfb_protected = true;
	InterlockedIncrement(&i->vfb->refcount);
	++g_Cache_stats.vfb_protects;
  }

  // Unprotect any frames out of CACHE_RANGE scope
  while ((protectcount > h_span) && (j != &video_frames)){
	if ( (j != i) && (j->vfb_protected) && (abs(n - j->frame_number) >= h_span) ) {
      UnProtectVFB(j);
      AVXLOG_INFO("Cache:%x: B: Unprotect vfb %x for frame %d", this, j->vfb, j->frame_number);
	  break;
	}
	j = j->prev;
  }
}


void Cache::UnProtectVFB(CachedVideoFrame *i)
{
  if (!!i->vfb && i->vfb_protected) {
	InterlockedDecrement(&i->vfb->refcount);
	i->vfb_protected = false;
	InterlockedDecrement(&protectcount);
	--g_Cache_stats.vfb_protects;
  }
}

/*********** A U D I O   C A C H E ************/

void Cache::FillZeros(void* buf, int start_offset, int count) {

    const int bps = vi.BytesPerAudioSample();
    unsigned char* byte_buf = (unsigned char*)buf;
    memset(byte_buf + start_offset * bps, 0, count * bps);
}

void __stdcall Cache::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) {
  if (count <= 0) 
    return;

  if ( (!vi.HasAudio()) || (start+count <= 0) || (start >= vi.num_audio_samples)) {
    // Completely skip.
    FillZeros(buf, 0, count);
    return;
  }

  if (start < 0) {  // Partial initial skip
    FillZeros(buf, 0, -start);  // Fill all samples before 0 with silence.
    count += start;  // Subtract start bytes from count.
    buf = ((BYTE*)buf) - (int)(start*vi.BytesPerAudioSample());   
    start = 0;
  }

  if (start+count > vi.num_audio_samples) {  // Partial ending skip
    FillZeros(buf, (vi.num_audio_samples - start), count - (vi.num_audio_samples - start));  // Fill end samples
    count = (vi.num_audio_samples - start);
  }

  if (start < ac_expected_next)
    ac_currentscore-=25;    // revisiting old ground - a cache could help
  else if (start > ac_expected_next)
    ac_currentscore-=5;     // skiping chunks - a cache may not help
  else // (start == ac_expected_next)
    ac_currentscore +=5;    // strict linear reading - why bother with a cache

  ac_currentscore = std::max(std::min(ac_currentscore, 450), -10000000);

  if (h_audiopolicy == CACHE_NOTHING && ac_currentscore <=0) {
    SetCacheHints(CACHE_AUDIO_AUTO, 0);
    AVXLOG_INFO("CacheAudio:%x: Automatically adding audiocache!", this);
  }

  if (h_audiopolicy == CACHE_AUDIO_AUTO  && (ac_currentscore > 400) ) {
    SetCacheHints(CACHE_AUDIO_NONE, 0);
    AVXLOG_INFO("CacheAudio:%x: Automatically deleting cache!", this);
  }

  ac_expected_next = start + count;

  if (h_audiopolicy == CACHE_NOTHING) {
    child->GetAudio(buf, start, count, env);
    return;  // We are ok to return now!
  }

#ifdef _DEBUG
  char dbgbuf[255];
  sprintf(dbgbuf, "CA:Get st:%.6d co:%.6d .cst:%.6d cco:%.6d, sc:%d",
                    int(start), int(count), int(cache_start), int(cache_count), int(ac_currentscore));
  AVXLOG_INFO("%s", dbgbuf);
#endif

  int shiftsamples;

  while (count>maxsamplecount) {    //is cache big enough?
    AVXLOG_INFO("CA:%x:Cache too small->caching last audio", this);
    ac_too_small_count++;

// But the current cache might have 99% of what was requested??

    if ((ac_too_small_count > 2) && (maxsamplecount < vi.AudioSamplesFromBytes(4096*1024))) {  // Max size = 4MB!
      //automatically upsize cache!
      int new_size = (vi.BytesFromAudioSamples(count)+8192) & -8192;
      new_size = std::min(4096*1024, new_size);
      AVXLOG_INFO("CacheAudio:%x: Autoupsizing buffer to %d bytes!", this, new_size);
      SetCacheHints(h_audiopolicy, new_size); // updates maxsamplecount!!
      ac_too_small_count = 0;
    }
    else {
      child->GetAudio(buf, start, count, env);

      cache_count = std::min(count, (__int64)maxsamplecount); // Remember maxsamplecount gets updated
      cache_start = start+count-cache_count;
      BYTE *buff=(BYTE *)buf;
      buff += vi.BytesFromAudioSamples(cache_start - start);
      memcpy(cache, buff, vi.BytesFromAudioSamples(cache_count));

      return;
    }
  }

  if ((start < cache_start) || (start >= cache_start+maxsamplecount)) { //first sample is before cache or beyond linear reach -> restart cache
    AVXLOG_INFO("CA:%x: Restart", this);

    cache_count = std::min(count, (__int64)maxsamplecount);
    cache_start = start;
    child->GetAudio(cache, cache_start, cache_count, env);
  }
  else {
    if (start+count > cache_start+cache_count) { // Does the cache fail to cover the request?
      if (start+count > cache_start+maxsamplecount) {  // Is cache shifting necessary?
        shiftsamples = (start+count) - (cache_start+maxsamplecount);

        if ( (start - cache_start)/2 > shiftsamples ) {  //shift half cache if possible
          shiftsamples = (start - cache_start)/2;
        }
        if (shiftsamples >= cache_count) {
          shiftsamples = cache_count; // Preserve linear access
        }
        else {
          memmove(cache, cache+shiftsamples*samplesize,(cache_count-shiftsamples)*samplesize);
        }
        cache_start = cache_start + shiftsamples;
        cache_count = cache_count - shiftsamples;
      }
      // Read just enough to complete the current request, append it to the cache
      child->GetAudio(cache + cache_count*samplesize, cache_start + cache_count, start+count-(cache_start+cache_count), env);
      cache_count += start+count-(cache_start+cache_count);
    }
  }

  //copy cache to buf
  memcpy(buf,cache + (start - cache_start)*samplesize, count*samplesize);

}

/*********** C A C H E   H I N T S ************/

void __stdcall Cache::SetCacheHints(int cachehints,size_t frame_range) {

  // Hack to detect if we are a cache, respond with our this pointer
  if ((cachehints == GetMyThis) && (frame_range != 0)) {
	*(int *)frame_range = (size_t)this;
	return;
  }

  AVXLOG_INFO("Cache:%x: Setting cache hints (hints:%d, range:%d )", this, cachehints, frame_range);

  if (cachehints == CACHE_AUDIO || cachehints == CACHE_AUDIO_AUTO) {

    if (!vi.HasAudio())
      return;

    // Range means for audio.
    // 0 == Create a default buffer (64kb).
    // Positive. Allocate X bytes for cache.

    if (h_audiopolicy != CACHE_NOTHING && (frame_range == 0))   // We already have a policy - no need for a default one.
      return;

    h_audiopolicy = cachehints;

    if (frame_range == 0)
      frame_range=64*1024;

    char *oldcache = cache;
    cache = new char[frame_range];
    if (cache) {
      maxsamplecount = frame_range/samplesize;
      if (oldcache) {
        // Keep old cache contents
        cache_count = std::min(cache_count, (__int64)maxsamplecount);
        memcpy(cache, oldcache, vi.BytesFromAudioSamples(cache_count));
        delete[] oldcache;
      }
      else {
        cache_start=0;
        cache_count=0;  
      }
    }
    else {
      cache = oldcache;
    }
    return;
  }

  if (cachehints == CACHE_AUDIO_NONE) {
    if (cache) {
      delete[] cache;
      cache = 0;
    }
    h_audiopolicy = CACHE_NOTHING;
  }


  if (cachehints == CACHE_ALL) {
	int _cache_limit;

    h_policy = CACHE_ALL;  // This is default operation, a simple LRU cache

    if (frame_range >= MAX_CACHED_VIDEO_FRAMES)
      _cache_limit = CACHE_SCALE_FACTOR * MAX_CACHED_VIDEO_FRAMES;
    else
      _cache_limit = CACHE_SCALE_FACTOR * frame_range;

	if (_cache_limit > cache_limit)  // The max of all requests
	  cache_limit = _cache_limit;

    cache_init  = cache_limit/CACHE_SCALE_FACTOR;
    return;
  }

  if (cachehints == CACHE_NOTHING) {

    h_policy = CACHE_NOTHING;  // filter requested no caching.
    return;
  }

  if (cachehints == CACHE_RANGE) {

    h_policy = CACHE_RANGE;  // An explicit cache of radius "frame_range" around the current frame, n.

    if (frame_range <= (size_t)h_span)  // Use the largest size when we have multiple clients
      return;

    h_span = frame_range;
	if (h_span > MAX_CACHE_RANGE) h_span=MAX_CACHE_RANGE;
  }
} 

/*********** C L E A N U P ************/

Cache::~Cache() {
  // Excise me from the linked list
  _ASSERTE(*priorCache == this);
  if (nextCache) nextCache->priorCache = priorCache;
  *priorCache = nextCache;
  
  if (cache) {
    delete[] cache;
    cache = 0;
  }

  CachedVideoFrame *k, *j;
  for (k = video_frames.next; k!=&video_frames;)
  {
	  j = k->next;
	  if (k->vfb_protected) UnProtectVFB(k);
	  if (k->vfb_locked) UnlockVFB(k);
	  delete k;
	  k = j;
  }
}

/*********** C R E A T E ************/

AVSValue __cdecl Cache::Create_Cache(AVSValue args, void*, IScriptEnvironment* env) 
{
  PClip p=0;

  if (args.IsClip())
	  p = args.AsClip();
  else
	  p = args[0].AsClip();

  if (p) {
	size_t q = 0;
	
	// Check if "p" is a cache instance
	p->SetCacheHints(GetMyThis, (size_t)&q);

	// Do not cache another cache!
	if (q != (size_t)(void*)p) return new Cache(p, env);
  }
  return p;
}

}; // namespace avxsynth
