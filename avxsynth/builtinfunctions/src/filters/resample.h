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

#ifndef __Resample_H__
#define __Resample_H__

#include "internal.h"
#if 0
#include "resample_functions.h" // doesn't exist in source tree
#include "transform.h"
#include "../core/softwire_helpers.h" // doesn't exist in source tree
#endif

namespace avxsynth {

    
AVSValue __cdecl Create_PointResize(AVSValue args, void*, IScriptEnvironment* env);

AVSValue __cdecl Create_BilinearResize(AVSValue args, void*, IScriptEnvironment* env);

AVSValue __cdecl Create_BicubicResize(AVSValue args, void*, IScriptEnvironment* env);

AVSValue __cdecl Create_LanczosResize(AVSValue args, void*, IScriptEnvironment* env);

AVSValue __cdecl Create_Lanczos4Resize(AVSValue args, void*, IScriptEnvironment* env);

AVSValue __cdecl Create_BlackmanResize(AVSValue args, void*, IScriptEnvironment* env);

AVSValue __cdecl Create_Spline16Resize(AVSValue args, void*, IScriptEnvironment* env);

AVSValue __cdecl Create_Spline36Resize(AVSValue args, void*, IScriptEnvironment* env);

AVSValue __cdecl Create_Spline64Resize(AVSValue args, void*, IScriptEnvironment* env);

AVSValue __cdecl Create_GaussianResize(AVSValue args, void*, IScriptEnvironment* env);    
    
#if 0
class FilteredResizeH : public GenericVideoFilter, public  CodeGenerator
/**
  * Class to resize in the horizontal direction using a specified sampling filter
  * Helper for resample functions
 **/
{
public:
  FilteredResizeH( PClip _child, double subrange_left, double subrange_width, int target_width, 
                   ResamplingFunction* func, IScriptEnvironment* env );
  virtual ~FilteredResizeH(void);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  DynamicAssembledCode GenerateResizer(int gen_plane, IScriptEnvironment* env);
private:
  DynamicAssembledCode assemblerY;
  DynamicAssembledCode assemblerUV;
  
  int* /*const*/ pattern_luma;
  int* /*const*/ pattern_chroma;
  int original_width;
  bool use_dynamic_code;

  BYTE *tempY, *tempUV;
// These must be properly set when running the filter:
  BYTE *gen_srcp;
  BYTE *gen_dstp;
  int gen_src_pitch, gen_dst_pitch;
// These are used by the filter:
  int gen_h, gen_x;
  BYTE* gen_temp_destp;
};

 

class FilteredResizeV : public GenericVideoFilter 
/**
  * Class to resize in the vertical direction using a specified sampling filter
  * Helper for resample functions
 **/
{
public:
  FilteredResizeV( PClip _child, double subrange_top, double subrange_height, int target_height,
                   ResamplingFunction* func, IScriptEnvironment* env );
  virtual ~FilteredResizeV(void);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);


private:
  int* /*const*/ resampling_pattern;
  int* /*const*/ resampling_patternUV;
  int *yOfs;
  int *yOfsUV;
  int pitch_gY;
  int pitch_gUV;
};


/*** Resample factory methods ***/

class FilteredResize
/**
  * Helper for resample functions
 **/
{
public:
static PClip CreateResizeH( PClip clip, double subrange_left, double subrange_width, int target_width, 
                            ResamplingFunction* func, IScriptEnvironment* env );

static PClip CreateResizeV( PClip clip, double subrange_top, double subrange_height, int target_height, 
                            ResamplingFunction* func, IScriptEnvironment* env );

static PClip CreateResize( PClip clip, int target_width, int target_height, const AVSValue* args, 
                           ResamplingFunction* f, IScriptEnvironment* env );

static AVSValue __cdecl Create_PointResize(AVSValue args, void*, IScriptEnvironment* env);

static AVSValue __cdecl Create_BilinearResize(AVSValue args, void*, IScriptEnvironment* env);

static AVSValue __cdecl Create_BicubicResize(AVSValue args, void*, IScriptEnvironment* env);

// 09-14-2002 - Vlad59 - Lanczos3Resize - 
static AVSValue __cdecl Create_LanczosResize(AVSValue args, void*, IScriptEnvironment* env);

static AVSValue __cdecl Create_Lanczos4Resize(AVSValue args, void*, IScriptEnvironment* env);

static AVSValue __cdecl Create_BlackmanResize(AVSValue args, void*, IScriptEnvironment* env);

static AVSValue __cdecl Create_Spline16Resize(AVSValue args, void*, IScriptEnvironment* env);

static AVSValue __cdecl Create_Spline36Resize(AVSValue args, void*, IScriptEnvironment* env);

static AVSValue __cdecl Create_Spline64Resize(AVSValue args, void*, IScriptEnvironment* env);

static AVSValue __cdecl Create_GaussianResize(AVSValue args, void*, IScriptEnvironment* env);
};

#endif

}; // namespace avxsynth

#endif // __Resample_H__
