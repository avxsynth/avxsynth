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


#ifndef __Internal_H__
#define __Internal_H__

#include <string.h>
#include "core/avxsynth.h"

namespace avxsynth {
#define AVS_VERSION 4.00
#define AVS_AVX_SYNTH "AvxSynth 4.0 (Linux port of AviSynth 2.58),\n"
#define AVS_COPYRIGHT "\xC2\xA9 2000-2012 Ben Rudiak-Gould, et al.\n(http://www.avxsynth.org))\n"
#define AVS_VERSTR "build:"__DATE__" ["__TIME__"]"

extern const char _AVS_VERSTR[], _AVS_COPYRIGHT[];

// env->ManageCache() Non user keys definition
// Define user accessible keys in avisynth.h
//
enum {MC_ReturnVideoFrameBuffer =0xFFFF0001};
enum {MC_ManageVideoFrameBuffer =0xFFFF0002};
enum {MC_PromoteVideoFrameBuffer=0xFFFF0003};
enum {MC_RegisterCache          =0xFFFF0004};

typedef AVSValue (__cdecl *AVXSYNTH_PLUGIN_APPLY_FUNCTION)(AVSValue args, void* user_data, IScriptEnvironment* env);

struct AVSFunction {
  const char* name;
  const char* param_types;
  AVXSYNTH_PLUGIN_APPLY_FUNCTION apply;
  void* user_data;
};

PClip Create_MessageClip(const char* message, int width, int height,
  int pixel_type, bool shrink, int textcolor, int halocolor, int bgcolor,
  IScriptEnvironment* env);

PClip new_Splice(PClip _child1, PClip _child2, bool realign_sound, IScriptEnvironment* env);
PClip new_SeparateFields(PClip _child, IScriptEnvironment* env);
PClip new_AssumeFrameBased(PClip _child);

void BitBlt(BYTE* dstp, int dst_pitch, const BYTE* srcp, 
            int src_pitch, int row_size, int height);

void asm_BitBlt_ISSE(BYTE* dstp, int dst_pitch, const BYTE* srcp, int src_pitch, int row_size, int height);
void asm_BitBlt_MMX(BYTE* dstp, int dst_pitch, const BYTE* srcp, int src_pitch, int row_size, int height);

long GetCPUFlags();


class _PixelClip {
  enum { buffer=320 };
  BYTE clip[256+buffer*2];
public:
  _PixelClip() {  
    memset(clip, 0, buffer);
    for (int i=0; i<256; ++i) clip[i+buffer] = (BYTE)i;
    memset(clip+buffer+256, 255, buffer);
  }
  BYTE operator()(int i) { return clip[i+buffer]; }
};

extern _PixelClip PixelClip;


template<class ListNode>
static __inline void Relink(ListNode* newprev, ListNode* me, ListNode* newnext) {
  if (me == newprev || me == newnext) return;
  me->next->prev = me->prev;
  me->prev->next = me->next;
  me->prev = newprev;
  me->next = newnext;
  me->prev->next = me->next->prev = me;
}



/*** Inline helper methods ***/


static __inline BYTE ScaledPixelClip(int i) {
  return PixelClip((i+32768) >> 16);
}

static __inline int RGB2YUV(int rgb) 
{
  const int cyb = int(0.114*219/255*65536+0.5);
  const int cyg = int(0.587*219/255*65536+0.5);
  const int cyr = int(0.299*219/255*65536+0.5);

  // y can't overflow
  int y = (cyb*(rgb&255) + cyg*((rgb>>8)&255) + cyr*((rgb>>16)&255) + 0x108000) >> 16;
  int scaled_y = (y - 16) * int(255.0/219.0*65536+0.5);
  int b_y = ((rgb&255) << 16) - scaled_y;
  int u = ScaledPixelClip((b_y >> 10) * int(1/2.018*1024+0.5) + 0x800000);
  int r_y = (rgb & 0xFF0000) - scaled_y;
  int v = ScaledPixelClip((r_y >> 10) * int(1/1.596*1024+0.5) + 0x800000);
  return ((y*256+u)*256+v) | (rgb & 0xff000000);
}

static __inline bool IsClose(int a, int b, unsigned threshold) 
  { return (unsigned(a-b+threshold) <= threshold*2); }



}; // namespace avxsynth
#endif  // __Internal_H__
