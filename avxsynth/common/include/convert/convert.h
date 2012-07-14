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

#ifndef __Convert_H__
#define __Convert_H__

#include <internal.h>

namespace avxsynth {
  
/*****************************************************
 *******   Colorspace Single-Byte Conversions   ******
 ****************************************************/

#define KEEP_THREE_DECIMALS(x)    (((int)(1000.0*(x) + 0.5))/1000.0)

inline void YUV2RGB(int y, int u, int v, BYTE* out, int matrix) 
{
    switch(matrix)
    {
    case 0: // Rec601
    default:
        {
#if 0 // By the book
            /*
            http://www.fourcc.org/fccyvrgb.php
            YUV to RGB Conversion

            B ́ = Y601                  + 1.732(U – 128)
            G ́ = Y601 – 0.698(V – 128) – 0.336(U – 128)
            R ́ = Y601 + 1.371(V – 128)

            */
            const int cy  = int((255.0/219.0)*65536+0.5);
            const int crv = int(1.371*65536+0.5);
            const int cgv = int(0.698*65536+0.5);
            const int cgu = int(0.336*65536+0.5);
            const int cbu = int(1.732*65536+0.5);

            int scaled_y = (y - 16) * cy;

            out[0] = ScaledPixelClip(scaled_y + (u-128) * cbu); // blue
            out[1] = ScaledPixelClip(scaled_y - (u-128) * cgu - (v-128) * cgv); // green
            out[2] = ScaledPixelClip(scaled_y + (v-128) * crv); // red

#else // infered from avisynth MMX code
            const float cy  = KEEP_THREE_DECIMALS(255.0/219.0);
            const float crv = KEEP_THREE_DECIMALS(((1-0.299)*255.0)/112.0);
            const float cgv = KEEP_THREE_DECIMALS((1-0.299)*(0.299/0.587)*(255.0/112.0));
            const float cgu = KEEP_THREE_DECIMALS((1-0.114)*(0.114/0.587)*(255.0/112.0));
            const float cbu = KEEP_THREE_DECIMALS(((1-0.114)*255.0)/112.0);

            int scaled_y = (y - 16) * cy;
            
            out[0] = BYTE(scaled_y + (u-128) * cbu + 0.5); // blue
            out[1] = BYTE(scaled_y - (u-128) * cgu - (v-128) * cgv + 0.5); // green
            out[2] = BYTE(scaled_y + (v-128) * crv + 0.5); // red
#endif
        }        
        break;
    case 1: // Rec709=1
        {
            /*
            B ́ = Y709                   + 1.816(Cb – 128)
            G ́ = Y709 – 0.459(Cr – 128) – 0.183(Cb – 128)
            R ́ = Y709 + 1.540(Cr – 128)
            */
            const int cy  = int((255.0/219.0)*65536+0.5);
            const int crv = int(1.540*65536+0.5);
            const int cgv = int(0.459*65536+0.5);
            const int cgu = int(0.183*65536+0.5);
            const int cbu = int(1.816*65536+0.5);

            int scaled_y = (y - 16) * cy;
            
            out[0] = ScaledPixelClip(scaled_y + (u-128) * cbu); // blue
            out[1] = ScaledPixelClip(scaled_y - (u-128) * cgu - (v-128) * cgv); // green
            out[2] = ScaledPixelClip(scaled_y + (v-128) * crv); // red   
        }
        break;
    case 3: // PC_601
        {  
#if 0 // By the book
            /*
            http://www.fourcc.org/fccyvrgb.php
            YUV to RGB Conversion

            B = 1.164(Y - 16)                   + 2.018(U - 128)
            G = 1.164(Y - 16) - 0.813(V - 128) - 0.391(U - 128)
            R = 1.164(Y - 16) + 1.596(V - 128) 
            */
            const int cy  = int((255.0/219.0)*65536+0.5);
            const int crv = int(1.596*65536+0.5);
            const int cgv = int(0.813*65536+0.5);
            const int cgu = int(0.391*65536+0.5);
            const int cbu = int(2.018*65536+0.5);

            int scaled_y = (y - 16) * cy;

            out[0] = ScaledPixelClip(scaled_y + (u-128) * cbu); // blue
            out[1] = ScaledPixelClip(scaled_y - (u-128) * cgu - (v-128) * cgv); // green
            out[2] = ScaledPixelClip(scaled_y + (v-128) * crv); // red
#else // infered from avisynth MMX code
            const float cy  = 1.0; // KEEP_THREE_DECIMALS(255.0/219.0);
            const float crv = KEEP_THREE_DECIMALS(((1-0.299)*255.0)/127.0);
            const float cgv = KEEP_THREE_DECIMALS((1-0.299)*(0.299/0.587)*(255.0/127.0));
            const float cgu = KEEP_THREE_DECIMALS((1-0.114)*(0.114/0.587)*(255.0/127.0));
            const float cbu = KEEP_THREE_DECIMALS(((1-0.114)*255.0)/127.0);

            int scaled_y = y * cy;
            
            out[0] = BYTE(scaled_y + (u-128) * cbu + 0.5); // blue
            out[1] = BYTE(scaled_y - (u-128) * cgu - (v-128) * cgv + 0.5); // green
            out[2] = BYTE(scaled_y + (v-128) * crv + 0.5); // red
#endif
        }
        break;
    case 7: // PC_709=7
        {
            /*
            B ́ = 1.164(Y709 – 16)                  + 2.115(U – 128)
            G ́ = 1.164(Y709 – 16) – 0.534(V – 128) – 0.213(U – 128)
            R ́ = 1.164(Y709 – 16) + 1.793(V – 128) 
            */
            const int cy  = int((255.0/219.0)*65536+0.5);
            const int crv = int(1.793*65536+0.5);
            const int cgv = int(0.534*65536+0.5);
            const int cgu = int(0.213*65536+0.5);
            const int cbu = int(2.115*65536+0.5);

            int scaled_y = (y - 16) * cy;

            out[0] = ScaledPixelClip(scaled_y + (u-128) * cbu); // blue
            out[1] = ScaledPixelClip(scaled_y - (u-128) * cgu - (v-128) * cgv); // green
            out[2] = ScaledPixelClip(scaled_y + (v-128) * crv); // red
        }
        break;
    }
}

// in convert_a.asm
extern "C" 
{
  extern void __cdecl mmx_YUY2toRGB24(const BYTE* src, BYTE* dst, const BYTE* src_end, int src_pitch, int row_size, int matrix);
  extern void __cdecl mmx_YUY2toRGB32(const BYTE* src, BYTE* dst, const BYTE* src_end, int src_pitch, int row_size, int matrix);
}






/********************************************************
 *******   Colorspace GenericVideoFilter Classes   ******
 *******************************************************/


class ConvertToRGB : public GenericVideoFilter 
/**
  * Class to handle conversion to RGB & RGBA
 **/
{
public:
  ConvertToRGB(PClip _child, bool rgb24, const char* matrix, IScriptEnvironment* env);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  
  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);  
  static AVSValue __cdecl Create32(AVSValue args, void*, IScriptEnvironment* env);
  static AVSValue __cdecl Create24(AVSValue args, void*, IScriptEnvironment* env);

private:
  bool use_mmx, is_yv12;
  int yv12_width, theMatrix;
  enum {Rec601=0, Rec709=1, PC_601=3, PC_709=7 };	// Note! convert_a.asm assumes these values
};

class ConvertToYV12 : public GenericVideoFilter 
/**
  * Class for conversions to YV12
 **/
{
public:
  ConvertToYV12(PClip _child, bool _interlaced, IScriptEnvironment* env);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);

  static AVSValue __cdecl Create(AVSValue args,void*, IScriptEnvironment* env);

private:
  bool isYUY2, isRGB32, isRGB24, interlaced;
};



}; //namespace avxsynth


#endif  // __Convert_H__
