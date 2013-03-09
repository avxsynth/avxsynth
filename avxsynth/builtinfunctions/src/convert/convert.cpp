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

#include "common/include/convert/convert.h"
#include "convert_rgb.h"
#include "convert_yv12.h"
#include "convert_yuy2.h"
#include <string>

namespace avxsynth {

/********************************************************************
***** Declare index of new filters for Avisynth's filter engine *****
********************************************************************/

AVSFunction Convert_filters[] = {
  { "ConvertToRGB", "c[matrix]s[interlaced]b", ConvertToRGB::Create },       // matrix can be "rec601", rec709", "PC.601" or "PC.709"
  { "ConvertToRGB24", "c[matrix]s[interlaced]b", ConvertToRGB::Create24 },
  { "ConvertToRGB32", "c[matrix]s[interlaced]b", ConvertToRGB::Create32 },
  { "ConvertToYV12", "c[interlaced]b[matrix]s", ConvertToYV12::Create },
  { "ConvertToYUY2", "c[interlaced]b[matrix]s", ConvertToYUY2::Create },
  { "ConvertBackToYUY2", "c[matrix]s", ConvertBackToYUY2::Create },
  { 0 }
};





/****************************************
 *******   Convert to RGB / RGBA   ******
 ***************************************/

ConvertToRGB::ConvertToRGB( PClip _child, bool rgb24, const char* matrix,
                            IScriptEnvironment* env )
  : GenericVideoFilter(_child)
{
  theMatrix = Rec601;
  is_yv12=false;
  if (matrix) {
    if (!strcasecmp(matrix, "rec709"))
      theMatrix = Rec709;
    else if (!strcasecmp(matrix, "PC.601"))
      theMatrix = PC_601;
    else if (!strcasecmp(matrix, "PC.709"))
      theMatrix = PC_709;
    else if (!strcasecmp(matrix, "rec601"))
      theMatrix = Rec601;
    else
      env->ThrowError("ConvertToRGB: invalid \"matrix\" parameter (must be matrix=\"Rec601\", \"Rec709\", \"PC.601\" or \"PC.709\")");
  }
  
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
  use_mmx = (env->GetCPUFlags() & CPUF_MMX) != 0;
#else
  use_mmx = 0;
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
  
  if ((theMatrix != Rec601) && ((vi.width & 3) != 0))
    env->ThrowError("ConvertToRGB: Rec.709 and PC Levels support require MMX and horizontal width a multiple of 4");
  vi.pixel_type = rgb24 ? VideoInfo::CS_BGR24 : VideoInfo::CS_BGR32;
}


PVideoFrame __stdcall ConvertToRGB::GetFrame(int n, IScriptEnvironment* env)
{
  PVideoFrame src = child->GetFrame(n, env);
  const int src_pitch = src->GetPitch();
  const BYTE* srcp = src->GetReadPtr();

  int src_rowsize = std::min(src_pitch, (src->GetRowSize()+7) & -8);
  // assumption: is_yuy2
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE  
  if (use_mmx && ((src_rowsize & 7) == 0) && (src_rowsize >= 16)) {
	VideoInfo vi2 = vi;
	vi2.width=src_rowsize / 2;
	PVideoFrame dst = env->NewVideoFrame(vi2,-2); // force pitch == rowsize
	BYTE* dstp = dst->GetWritePtr();

    (vi.IsRGB24() ? mmx_YUY2toRGB24 : mmx_YUY2toRGB32)(srcp, dstp,
      srcp + vi.height * src_pitch, src_pitch, src_rowsize, theMatrix);

	if (vi.width & 3) {  // Did we extend off the right edge of picture?
	  const int dst_pitch = dst->GetPitch();
	  const int x2 = (vi.width-2) * 2;
	  const int xE = (vi.width-1) * (vi2.BitsPerPixel()>>3);
	  srcp += vi.height * src_pitch;
	  for (int y=vi.height; y>0; --y) {
		srcp -= src_pitch;
		YUV2RGB(srcp[x2+2], srcp[x2+1], srcp[x2+3], &dstp[xE]);
		dstp += dst_pitch;
	  }
	}
	return env->Subframe(dst,0, dst->GetPitch(), vi2.BytesFromPixels(vi.width), vi.height);
  }
  else 
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE   
  {
	PVideoFrame dst = env->NewVideoFrame(vi);
	const int dst_pitch = dst->GetPitch();
	BYTE* dstp = dst->GetWritePtr();

	if (vi.IsRGB32()) {
	  srcp += vi.height * src_pitch;
	  for (int y=vi.height; y>0; --y) {
		srcp -= src_pitch;
		for (int x=0; x<vi.width; x+=2) {
		  YUV2RGB(srcp[x*2+0], srcp[x*2+1], srcp[x*2+3], &dstp[x*4], theMatrix);
		  dstp[x*4+3] = 255;
		  YUV2RGB(srcp[x*2+2], srcp[x*2+1], srcp[x*2+3], &dstp[x*4+4], theMatrix);
		  dstp[x*4+7] = 255;
		}
		dstp += dst_pitch;
	  }
	}
	else if (vi.IsRGB24()) {
	  srcp += vi.height * src_pitch;
	  for (int y=vi.height; y>0; --y) {
		srcp -= src_pitch;
		for (int x=0; x<vi.width; x+=2) {
		  YUV2RGB(srcp[x*2+0], srcp[x*2+1], srcp[x*2+3], &dstp[x*3], theMatrix);
		  YUV2RGB(srcp[x*2+2], srcp[x*2+1], srcp[x*2+3], &dstp[x*3+3], theMatrix);
		}
		dstp += dst_pitch;
	  }
	}
    return dst;
  }
}


AVSValue __cdecl ConvertToRGB::Create(AVSValue args, void*, IScriptEnvironment* env)
{
  PClip clip = args[0].AsClip();
  const char* const matrix = args[1].AsString(0);
  const VideoInfo& vi = clip->GetVideoInfo();
  if (vi.IsYUV()) {
    if (vi.IsPlanar()) {
      return new ConvertToRGB(new ConvertToYUY2(clip,false,args[2].AsBool(false),NULL,env), false, matrix, env);
    }
    return new ConvertToRGB(clip, false, matrix, env);
  } else {
    return clip;
  }
}


AVSValue __cdecl ConvertToRGB::Create32(AVSValue args, void*, IScriptEnvironment* env)
{
  PClip clip = args[0].AsClip();
  const char* const matrix = args[1].AsString(0);
  const VideoInfo vi = clip->GetVideoInfo();
  if (vi.IsYUV()) {
    if (vi.IsPlanar()) {
       return new ConvertToRGB(new ConvertToYUY2(clip,false,args[2].AsBool(false),NULL,env), false, matrix, env);
    }
    return new ConvertToRGB(clip, false, matrix, env);
  } else if (vi.IsRGB24())
    return new RGB24to32(clip);
  else
    return clip;
}


AVSValue __cdecl ConvertToRGB::Create24(AVSValue args, void*, IScriptEnvironment* env)
{
  PClip clip = args[0].AsClip();
  const char* const matrix = args[1].AsString(0);
  const VideoInfo& vi = clip->GetVideoInfo();
  if (vi.IsYUV()) {
    if (vi.IsPlanar()) {
       return new ConvertToRGB(new ConvertToYUY2(clip,false,args[2].AsBool(false),NULL,env), true, matrix, env);
    }
    return new ConvertToRGB(clip, true, matrix, env);
  } else if (vi.IsRGB32())
    return new RGB32to24(clip);
  else
    return clip;
}

/**********************************
 *******   Convert to YV12   ******
 *********************************/


ConvertToYV12::ConvertToYV12(PClip _child, bool _interlaced, IScriptEnvironment* env)
  : GenericVideoFilter(_child),
  interlaced(_interlaced)
{
  if (vi.width & 1)
    env->ThrowError("ConvertToYV12: Image width must be multiple of 2");
  if (interlaced && (vi.height & 3))
    env->ThrowError("ConvertToYV12: Interlaced image height must be multiple of 4");
  if ((!interlaced) && (vi.height & 1))
    env->ThrowError("ConvertToYV12: Image height must be multiple of 2");
  isYUY2=isRGB32=isRGB24=false;
  if (vi.IsYUY2()) isYUY2 = true;
  if (vi.IsRGB32()) isRGB32 = true;
  if (vi.IsRGB24()) isRGB24 = true;

  vi.pixel_type = VideoInfo::CS_YV12;

#if 0  
  if ((env->GetCPUFlags() & CPUF_MMX) == 0)
    env->ThrowError("ConvertToYV12: YV12 support require a MMX capable processor.");
#endif  
}

PVideoFrame __stdcall ConvertToYV12::GetFrame(int n, IScriptEnvironment* env) {
  PVideoFrame src = child->GetFrame(n, env);
  PVideoFrame dst = env->NewVideoFrame(vi);

  if (isYUY2) {
    if (interlaced) {
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE              
        if ((env->GetCPUFlags() & CPUF_INTEGER_SSE)) 
        {
            isse_yuy2_i_to_yv12(src->GetReadPtr(), src->GetRowSize(), src->GetPitch(),
                dst->GetWritePtr(PLANAR_Y), dst->GetWritePtr(PLANAR_U), dst->GetWritePtr(PLANAR_V), dst->GetPitch(PLANAR_Y),dst->GetPitch(PLANAR_U),
                src->GetHeight());
        } 
        else if (env->GetCPUFlags() & CPUF_MMX) 
        {
            mmx_yuy2_i_to_yv12(src->GetReadPtr(), src->GetRowSize(), src->GetPitch(),
                dst->GetWritePtr(PLANAR_Y), dst->GetWritePtr(PLANAR_U), dst->GetWritePtr(PLANAR_V), dst->GetPitch(PLANAR_Y),dst->GetPitch(PLANAR_U),
            src->GetHeight());
        }
        else
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
        {
            yuy2_i_to_yv12(src->GetReadPtr(), src->GetRowSize(), src->GetPitch(),
                dst->GetWritePtr(PLANAR_Y), dst->GetWritePtr(PLANAR_U), dst->GetWritePtr(PLANAR_V), dst->GetPitch(PLANAR_Y),dst->GetPitch(PLANAR_U),
                src->GetHeight());                                          
        }        
    } 
    else 
    {
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE                      
        if ((env->GetCPUFlags() & CPUF_INTEGER_SSE)) 
        {
            isse_yuy2_to_yv12(src->GetReadPtr(), src->GetRowSize(), src->GetPitch(),
                dst->GetWritePtr(PLANAR_Y), dst->GetWritePtr(PLANAR_U), dst->GetWritePtr(PLANAR_V), dst->GetPitch(PLANAR_Y),dst->GetPitch(PLANAR_U),
                src->GetHeight());
        } 
        else if (env->GetCPUFlags() & CPUF_MMX) 
        {
            mmx_yuy2_to_yv12(src->GetReadPtr(), src->GetRowSize(), src->GetPitch(),
                dst->GetWritePtr(PLANAR_Y), dst->GetWritePtr(PLANAR_U), dst->GetWritePtr(PLANAR_V), dst->GetPitch(PLANAR_Y),dst->GetPitch(PLANAR_U),
                src->GetHeight());
        }
        else
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE   
        {
            yuy2_to_yv12(src->GetReadPtr(), src->GetRowSize(), src->GetPitch(),
                dst->GetWritePtr(PLANAR_Y), dst->GetWritePtr(PLANAR_U), dst->GetWritePtr(PLANAR_V), dst->GetPitch(PLANAR_Y),dst->GetPitch(PLANAR_U),
                src->GetHeight());                                          
        }        

    }
  }

  return dst;

}

AVSValue __cdecl ConvertToYV12::Create(AVSValue args, void*, IScriptEnvironment* env)
{
  PClip clip = args[0].AsClip();
  const VideoInfo& vi = clip->GetVideoInfo();
  if (vi.IsRGB()) {
  	return new ConvertToYV12(new ConvertToYUY2(clip,false,false,args[2].AsString(0),env),args[1].AsBool(false),env);
  } else {
    if (args[2].Defined())
      env->ThrowError("ConvertToYV12: invalid \"matrix\" parameter (RGB data only)");
    if (vi.IsYUY2())
      return  new ConvertToYV12(clip,args[1].AsBool(false),env);
  }
  return clip;
}

}; // namespace avxsynth



