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

#include "transform.h"

namespace avxsynth {

/********************************************************************
***** Declare index of new filters for Avisynth's filter engine *****
********************************************************************/

AVSFunction Transform_filters[] = {
  { "FlipVertical", "c", FlipVertical::Create },     
  { "FlipHorizontal", "c", FlipHorizontal::Create },     
  { "Crop", "ciiii[align]b", Crop::Create },              // left, top, width, height *OR*
                                                  //  left, top, -right, -bottom (VDub style)
  { "CropBottom", "ci", Create_CropBottom },      // bottom amount
  { "AddBorders", "ciiii[color]i", AddBorders::Create },  // left, top, right, bottom [,color]
  { "Letterbox", "cii[x1]i[x2]i[color]i", Create_Letterbox },       // top, bottom, [left], [right] [,color]
  { 0 }
};










/********************************
 *******   Flip Vertical   ******
 ********************************/

PVideoFrame FlipVertical::GetFrame(int n, IScriptEnvironment* env) {
  PVideoFrame src = child->GetFrame(n, env);
  PVideoFrame dst = env->NewVideoFrame(vi);
  const BYTE* srcp = src->GetReadPtr();
  BYTE* dstp = dst->GetWritePtr();
  int row_size = src->GetRowSize();
  int src_pitch = src->GetPitch();
  int dst_pitch = dst->GetPitch();
  env->BitBlt(dstp, dst_pitch, srcp + (vi.height-1) * src_pitch, -src_pitch, row_size, vi.height);
  if (src->GetPitch(PLANAR_U)) {
    srcp = src->GetReadPtr(PLANAR_U);
    dstp = dst->GetWritePtr(PLANAR_U);
    row_size = src->GetRowSize(PLANAR_U);
    src_pitch = src->GetPitch(PLANAR_U);
    dst_pitch = dst->GetPitch(PLANAR_U);
    env->BitBlt(dstp, dst_pitch, srcp + (src->GetHeight(PLANAR_U)-1) * src_pitch, -src_pitch, row_size, src->GetHeight(PLANAR_U));
    srcp = src->GetReadPtr(PLANAR_V);
    dstp = dst->GetWritePtr(PLANAR_V);
    env->BitBlt(dstp, dst_pitch, srcp + (src->GetHeight(PLANAR_U)-1) * src_pitch, -src_pitch, row_size, src->GetHeight(PLANAR_U));
  }
  return dst;
}


AVSValue __cdecl FlipVertical::Create(AVSValue args, void*, IScriptEnvironment* env) 
{
  return new FlipVertical(args[0].AsClip());
}



/********************************
 *******   Flip Horizontal   ******
 ********************************/

PVideoFrame FlipHorizontal::GetFrame(int n, IScriptEnvironment* env) {
  PVideoFrame src = child->GetFrame(n, env);
  PVideoFrame dst = env->NewVideoFrame(vi);
  const BYTE* srcp = src->GetReadPtr();
  BYTE* dstp = dst->GetWritePtr();
  int row_size = src->GetRowSize();
  int src_pitch = src->GetPitch();
  int dst_pitch = dst->GetPitch();
  int h = src->GetHeight();
  int bpp = vi.BytesFromPixels(1);
  if (vi.IsYUY2()) { // Avoid flipping UV in YUY2 mode.
    srcp+=row_size;
    srcp-=4;
    for (int y=0; y<h;y++) {
      for (int x=0; x<row_size; x+=4) {
        dstp[x] = srcp[-x+2];
        dstp[x+1] = srcp[-x+1];
        dstp[x+2] = srcp[-x];
        dstp[x+3] = srcp[-x+3];
      }
      srcp += src_pitch;
      dstp += dst_pitch;
    }
    return dst;
  }
  if (vi.IsPlanar()) {  //For planar always 1bpp
    srcp+=row_size-1;
    for (int y=0; y<h;y++) { // Loop planar luma.
      for (int x=0; x<row_size; x++) {
        dstp[x] = srcp[-x];
      }
      srcp += src_pitch;
      dstp += dst_pitch;
    }

    if (src->GetPitch(PLANAR_U)) {
      srcp = src->GetReadPtr(PLANAR_U);
      dstp = dst->GetWritePtr(PLANAR_U);
      row_size = src->GetRowSize(PLANAR_U);
      src_pitch = src->GetPitch(PLANAR_U);
      dst_pitch = dst->GetPitch(PLANAR_U);
      h = src->GetHeight(PLANAR_U);
      srcp+=row_size-1;
      for (int y=0; y<h;y++) {
        for (int x=0; x<row_size; x++) {
          dstp[x] = srcp[-x];
        }
        srcp += src_pitch;
        dstp += dst_pitch;
      }
      srcp = src->GetReadPtr(PLANAR_V);
      dstp = dst->GetWritePtr(PLANAR_V);
      srcp+=row_size-1;
      for (int y=0; y<h;y++) {
        for (int x=0; x<row_size; x++) {
          dstp[x] = srcp[-x];
        }
        srcp += src_pitch;
        dstp += dst_pitch;
      }
    }
    return dst;
  }
  srcp+=row_size-bpp;
  if (vi.IsRGB32()) {
    for (int y=0; y<h;y++) { // Loop for RGB
      for (int x=0; x<row_size/4; x++) {
          ((int*)dstp)[x] = ((int*)srcp)[-x];
      }
      srcp += src_pitch;
      dstp += dst_pitch;
    }
    return dst;
  }
  for (int y=0; y<h;y++) { // Loop for RGB
    for (int x=0; x<row_size; x+=bpp) {
      for (int i=0;i<bpp;i++) {
        dstp[x+i] = srcp[-x+i];
      }
    }
    srcp += src_pitch;
    dstp += dst_pitch;
  }
  return dst;
}


AVSValue __cdecl FlipHorizontal::Create(AVSValue args, void*, IScriptEnvironment* env) 
{
  return new FlipHorizontal(args[0].AsClip());
}





/******************************
 *******   Crop Filter   ******
 *****************************/

Crop::Crop(int _left, int _top, int _width, int _height, int _align, PClip _child, IScriptEnvironment* env)
 : GenericVideoFilter(_child), align(_align), xsub(0), ysub(0)
{
  /* Negative values -> VDub-style syntax
     Namely, Crop(a, b, -c, -d) will crop c pixels from the right and d pixels from the bottom.  
     Flags on 0 values too since AFAICT it's much more useful to this syntax than the standard one. */
  if ( (_left<0) || (_top<0) )
    env->ThrowError("Crop: Top and Left must be more than 0");

  if (_width <= 0)
      _width = vi.width - _left + _width;
  if (_height <= 0)
      _height = vi.height - _top + _height;

  if (_width <=0)
    env->ThrowError("Crop: Destination width is 0 or less.");

  if (_height<=0)
    env->ThrowError("Crop: Destination height is 0 or less.");
  if (vi.IsYUV()) {
    // YUY2 can only crop to even pixel boundaries horizontally
    if (_left&1)
      env->ThrowError("Crop: YUV images can only be cropped by even numbers (left side).");
    if (_width&1)
      env->ThrowError("Crop: YUV images can only be cropped by even numbers (right side).");
    if (vi.IsYV12()) {
      xsub=1;
      ysub=1;
      if (_top&1)
        env->ThrowError("Crop: YV12 images can only be cropped by even numbers (top).");
      if (_height&1)
        env->ThrowError("Crop: YV12 images can only be cropped by even numbers (bottom).");
    }
  } else {
    // RGB is upside-down
    _top = vi.height - _height - _top;
  }


  if (_left + _width > vi.width || _top + _height > vi.height)
    env->ThrowError("Crop: you cannot use crop to enlarge or 'shift' a clip");

  left_bytes = vi.BytesFromPixels(_left);
  top = _top;
  vi.width = _width;
  vi.height = _height;

  if (align) {
    align = (env->GetCPUFlags() & CPUF_SSE2) ? 15 : 7;
  }

}


PVideoFrame Crop::GetFrame(int n, IScriptEnvironment* env) 
{
  PVideoFrame frame = child->GetFrame(n, env);

  const BYTE* srcpY = frame->GetReadPtr(PLANAR_Y) + top *  frame->GetPitch(PLANAR_Y) + left_bytes;
  const BYTE* srcpU = frame->GetReadPtr(PLANAR_U) + (top>>ysub) *  frame->GetPitch(PLANAR_U) + (left_bytes>>xsub);
  const BYTE* srcpV = frame->GetReadPtr(PLANAR_V) + (top>>ysub) *  frame->GetPitch(PLANAR_V) + (left_bytes>>xsub);

  int _align;

  if (frame->GetPitch(PLANAR_U) && (!vi.IsYV12() || env->PlanarChromaAlignment(IScriptEnvironment::PlanarChromaAlignmentTest)))
    _align = align & ((size_t)srcpY|(size_t)srcpU|(size_t)srcpV);
  else
    _align = align & (size_t)srcpY;

  if (_align) {
    PVideoFrame dst = env->NewVideoFrame(vi, align+1);

    env->BitBlt(dst->GetWritePtr(PLANAR_Y), dst->GetPitch(PLANAR_Y), srcpY,
      frame->GetPitch(PLANAR_Y), dst->GetRowSize(PLANAR_Y), dst->GetHeight(PLANAR_Y));

    env->BitBlt(dst->GetWritePtr(PLANAR_U), dst->GetPitch(PLANAR_U), srcpU,
      frame->GetPitch(PLANAR_U), dst->GetRowSize(PLANAR_U), dst->GetHeight(PLANAR_U));

    env->BitBlt(dst->GetWritePtr(PLANAR_V), dst->GetPitch(PLANAR_V), srcpV,
      frame->GetPitch(PLANAR_V), dst->GetRowSize(PLANAR_V), dst->GetHeight(PLANAR_V));

    return dst;
  }

  if (!frame->GetPitch(PLANAR_U))
    return env->Subframe(frame, top * frame->GetPitch() + left_bytes, frame->GetPitch(), vi.RowSize(), vi.height);
  else
    return env->SubframePlanar(frame, top * frame->GetPitch() + left_bytes, frame->GetPitch(), vi.RowSize(), vi.height,
                                      (top>>ysub) * frame->GetPitch(PLANAR_U) + (left_bytes>>xsub),
                                      (top>>ysub) * frame->GetPitch(PLANAR_V) + (left_bytes>>xsub),
                                      frame->GetPitch(PLANAR_U));
}


AVSValue __cdecl Crop::Create(AVSValue args, void*, IScriptEnvironment* env) 
{
  return new Crop( args[1].AsInt(), args[2].AsInt(), args[3].AsInt(), args[4].AsInt(), args[5].AsBool(false) ? 1 : 0, 
                   args[0].AsClip(), env );
}





/******************************
 *******   Add Borders   ******
 *****************************/

AddBorders::AddBorders(int _left, int _top, int _right, int _bot, int _clr, PClip _child, IScriptEnvironment* env)
 : GenericVideoFilter(_child), left(std::max(0,_left)), top(std::max(0,_top)), right(std::max(0,_right)), bot(std::max(0,_bot)), clr(_clr), xsub(0), ysub(0)
{
  if (vi.IsYUV()) {
    // YUY2 can only add even amounts
    left = left & -2;
    right = (right+1) & -2;
    if (vi.IsYV12()) {
      xsub=1;
      ysub=1;
      top=top& -2;
      bot=(bot+1)& -2;
    }

  } else {
    // RGB is upside-down
    int t = top; top = bot; bot = t;
  }
  vi.width += left+right;
  vi.height += top+bot;
}




PVideoFrame AddBorders::GetFrame(int n, IScriptEnvironment* env) 
{
  PVideoFrame src = child->GetFrame(n, env);
  PVideoFrame dst = env->NewVideoFrame(vi);

  const BYTE* srcp = src->GetReadPtr();
  BYTE* dstp = dst->GetWritePtr();
  const int src_pitch = src->GetPitch();
  const int dst_pitch = dst->GetPitch();
  const int src_row_size = src->GetRowSize();
  const int dst_row_size = dst->GetRowSize();
  const int src_height = src->GetHeight();

  const int initial_black = top * dst_pitch + vi.BytesFromPixels(left);
  const int middle_black = dst_pitch - src_row_size;
  const int final_black = bot * dst_pitch + vi.BytesFromPixels(right) 
                          + (dst_pitch - dst_row_size);
  if (vi.IsPlanar()) {
    const unsigned int colr = RGB2YUV(clr);
    const unsigned char YBlack=(colr>>16)&0xff;
    const unsigned char UBlack=(colr>>8)&0xff;
    const unsigned char VBlack=(colr)&0xff;

    BitBlt(dstp+initial_black, dst_pitch, srcp, src_pitch, src_row_size, src_height);
    for (int a=0; a<initial_black; a++)
      *(unsigned char*)(dstp+a) = YBlack;
    dstp += initial_black + src_row_size;
    for (int y=src_height-1; y>0; --y) {
      for (int b=0; b<middle_black; b++)
        *(unsigned char*)(dstp+b) = YBlack;
      dstp += dst_pitch;
    }
    for (int c=0; c<final_black; c++)
      *(unsigned char*)(dstp+c) = YBlack;

    if (src->GetPitch(PLANAR_U)) {
      const int initial_blackUV = (top>>ysub) * dst->GetPitch(PLANAR_U) + (left>>xsub);
      const int middle_blackUV  = dst->GetPitch(PLANAR_U) - src->GetRowSize(PLANAR_U);
      const int final_blackUV   = (bot>>ysub) * dst->GetPitch(PLANAR_U) + (right>>xsub)
                                + (dst->GetPitch(PLANAR_U)- dst->GetRowSize(PLANAR_U));

      BitBlt(dst->GetWritePtr(PLANAR_U)+initial_blackUV, dst->GetPitch(PLANAR_U), src->GetReadPtr(PLANAR_U), src->GetPitch(PLANAR_U), src->GetRowSize(PLANAR_U), src->GetHeight(PLANAR_U));
      dstp = dst->GetWritePtr(PLANAR_U);
      for (int a=0; a<initial_blackUV; a++)
        *(unsigned char*)(dstp+a) = UBlack;
      dstp += initial_blackUV + src->GetRowSize(PLANAR_U);
      for (int y=src->GetHeight(PLANAR_U)-1; y>0; --y) {
        for (int b=0; b<middle_blackUV; b++)
          *(unsigned char*)(dstp+b) = UBlack;
        dstp += dst->GetPitch(PLANAR_U);
      }
      for (int c=0; c<final_blackUV; c ++)
        *(unsigned char*)(dstp+c) = UBlack;

      BitBlt(dst->GetWritePtr(PLANAR_V)+initial_blackUV, dst->GetPitch(PLANAR_V), src->GetReadPtr(PLANAR_V), src->GetPitch(PLANAR_V), src->GetRowSize(PLANAR_V), src->GetHeight(PLANAR_V));
      dstp = dst->GetWritePtr(PLANAR_V);
      for (int a=0; a<initial_blackUV; a++)
        *(unsigned char*)(dstp+a) = VBlack;
      dstp += initial_blackUV + src->GetRowSize(PLANAR_U);
      for (int y=src->GetHeight(PLANAR_U)-1; y>0; --y) {
        for (int b=0; b<middle_blackUV; b++)
          *(unsigned char*)(dstp+b) = VBlack;
        dstp += dst->GetPitch(PLANAR_U);
      }
      for (int c=0; c<final_blackUV; c++)
        *(unsigned char*)(dstp+c) = VBlack;
    }
  }
  else if (vi.IsYUY2()) {
    const unsigned int colr = RGB2YUV(clr);
    const uint32_t black = (colr>>16) * 0x010001 + ((colr>>8)&255) * 0x0100 + (colr&255) * 0x01000000;

    BitBlt(dstp+initial_black, dst_pitch, srcp, src_pitch, src_row_size, src_height);
    for (int a=0; a<initial_black; a += 4)
      *(unsigned __int32*)(dstp+a) = black;
    dstp += initial_black + src_row_size;
    for (int y=src_height-1; y>0; --y) {
      for (int b=0; b<middle_black; b += 4)
        *(unsigned __int32*)(dstp+b) = black;
      dstp += dst_pitch;
    }
    for (int c=0; c<final_black; c += 4)
      *(unsigned __int32*)(dstp+c) = black;
  }
  else if (vi.IsRGB24()) {
    const int ofs = dst_pitch - dst_row_size;
    const unsigned char  clr0 = (clr & 0xFF);
    const unsigned short clr1 = (clr >> 8);

    BitBlt(dstp+initial_black, dst_pitch, srcp, src_pitch, src_row_size, src_height);
    for (int i=0; i<initial_black; i+=3) {
      dstp[i] = clr0; *(uint16_t*)(dstp+i+1) = clr1;
      if (i % dst_pitch >= dst_row_size - 3) i += ofs;
    } //for i
    dstp += initial_black + src_row_size;
    for (int y=src_height-1; y>0; --y) {
      for (int i=0; i<middle_black; i+=3) {
        dstp[i] = clr0; *(uint16_t*)(dstp+i+1) = clr1;
        if (i == vi.BytesFromPixels(right)-3) i += ofs;
      } // for i
      dstp += dst_pitch;
    } // for y
    for (int i=0; i<final_black; i+=3) {
      dstp[i] = clr0; *(uint16_t*)(dstp+i+1) = clr1;
      if (i % dst_pitch == vi.BytesFromPixels(right)-3) i += ofs;
    } // for i
  } // if vi.IsRGB24
  else {
    BitBlt(dstp+initial_black, dst_pitch, srcp, src_pitch, src_row_size, src_height);
    for (int i=0; i<initial_black; i+=4)
      *(unsigned __int32*)(dstp+i) = clr;
    dstp += initial_black + src_row_size;
    for (int y=src_height-1; y>0; --y) {
      for (int i=0; i<middle_black; i+=4)
        *(unsigned __int32*)(dstp+i) = clr;
      dstp += dst_pitch;
    } // for y
    for (int i=0; i<final_black; i+=4)
      *(unsigned __int32*)(dstp+i) = clr;
  } // end else
  return dst;
}



AVSValue __cdecl AddBorders::Create(AVSValue args, void*, IScriptEnvironment* env) 
{
  return new AddBorders( args[1].AsInt(), args[2].AsInt(), args[3].AsInt(), 
                         args[4].AsInt(), args[5].AsInt(0), args[0].AsClip(), env);
}





/******************************
 *******   Fill Border   ******
 *****************************/


 /*  This function fills up the right side of the picture on planar images with duplicates of the rightmost pixel
  *   TODO: Implement fast ISSE routines
  */

FillBorder::FillBorder(PClip _clip) : GenericVideoFilter(_clip) {
}

PVideoFrame __stdcall FillBorder::GetFrame(int n, IScriptEnvironment* env) {

  PVideoFrame src = child->GetFrame(n, env);
  if (src->GetRowSize(PLANAR_Y)==src->GetRowSize(PLANAR_Y_ALIGNED)) return src;  // No need to fill extra pixels
  
  unsigned char* Ydata = src->GetWritePtr(PLANAR_U) - (src->GetOffset(PLANAR_U)-src->GetOffset(PLANAR_Y)); // Nasty hack, to avoid "MakeWritable" - never, EVER do this at home!
  unsigned char* Udata = src->GetWritePtr(PLANAR_U);
  unsigned char* Vdata = src->GetWritePtr(PLANAR_V);

  int fillp=src->GetRowSize(PLANAR_Y_ALIGNED) - src->GetRowSize(PLANAR_Y);
  int h=src->GetHeight(PLANAR_Y);

  Ydata = &Ydata[src->GetRowSize(PLANAR_Y)-1];
  for (int y=0;y<h;y++){
    for (int x=1;x<=fillp;x++) {
      Ydata[x]=Ydata[0];
    }
    Ydata+=src->GetPitch(PLANAR_Y);
  }

  fillp=src->GetRowSize(PLANAR_U_ALIGNED) - src->GetRowSize(PLANAR_U);
  Udata = &Udata[src->GetRowSize(PLANAR_U)-1];
  Vdata = &Vdata[src->GetRowSize(PLANAR_V)-1];
  h=src->GetHeight(PLANAR_U);

  for (int y=0;y<h;y++){
    for (int x=1;x<=fillp;x++) {
      Udata[x]=Udata[0];
      Vdata[x]=Vdata[0];
    }
    Udata+=src->GetPitch(PLANAR_U);
    Vdata+=src->GetPitch(PLANAR_V);
  }
  return src;
}
 

PClip FillBorder::Create(PClip clip) 
{
  if (!clip->GetVideoInfo().IsPlanar()) {  // If not planar, already ok.
    return clip;
  }
  else 
    return new FillBorder(clip);
}





/**********************************
 *******   Factory Methods   ******
 *********************************/


AVSValue __cdecl Create_Letterbox(AVSValue args, void*, IScriptEnvironment* env) 
{
  PClip clip = args[0].AsClip();
  int top = args[1].AsInt();
  int bot = args[2].AsInt();
  int left = args[3].AsInt(0); 
  int right = args[4].AsInt(0);
  int color = args[5].AsInt(0);
  const VideoInfo& vi = clip->GetVideoInfo();
  if ( (top<0) || (bot<0) || (left<0) || (right<0) ) 
    env->ThrowError("LetterBox: You cannot specify letterboxing less than 0.");
  if (top+bot>=vi.height) // Must be >= otherwise it is interpreted wrong by crop()
    env->ThrowError("LetterBox: You cannot specify letterboxing that is bigger than the picture (height).");  
  if (right+left>=vi.width) // Must be >= otherwise it is interpreted wrong by crop()
    env->ThrowError("LetterBox: You cannot specify letterboxing that is bigger than the picture (width).");
  if (vi.IsYUY2() && (left&1))
    env->ThrowError("LetterBox: Width must be divideable with 2 (Left side)");
  if (vi.IsYUY2() && (right&1))
    env->ThrowError("LetterBox: Width must be divideable with 2 (Right side)");

  return new AddBorders(left, top, right, bot, color, new Crop(left, top, vi.width-left-right, vi.height-top-bot, 0, clip, env), env);
}


AVSValue __cdecl Create_CropBottom(AVSValue args, void*, IScriptEnvironment* env) 
{
  PClip clip = args[0].AsClip();
  const VideoInfo& vi = clip->GetVideoInfo();

  if (args[1].AsInt() >= vi.height) // Must be >= otherwise it is interpreted wrong by crop()
    env->ThrowError("CropBottom: You cannot specify a crop that is greater than the picture height.");  

  return new Crop(0, 0, vi.width, vi.height - args[1].AsInt(), 0, clip, env);
}
 
}; // namespace avxsynth
