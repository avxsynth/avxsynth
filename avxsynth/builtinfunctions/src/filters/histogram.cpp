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

#include "histogram.h"
#include "info.h"


namespace avxsynth {

/********************************************************************
***** Declare index of new filters for Avisynth's filter engine *****
********************************************************************/

AVSFunction Histogram_filters[] = {
  { "Histogram", "c[mode]s", Histogram::Create },   // src clip
  { 0 }
};




/***********************************
 *******   Histogram Filter   ******
 **********************************/

Histogram::Histogram(PClip _child, Mode _mode, IScriptEnvironment* env)
  : GenericVideoFilter(_child), mode(_mode)
{

  if (mode == ModeClassic) {
    if (!vi.IsYUV())
      env->ThrowError("Histogram: YUV data only");
    vi.width += 256;
  }

  if (mode == ModeLevels) {
    if (!vi.IsPlanar())
      env->ThrowError("Histogram: Levels mode only available in PLANAR.");
    vi.width += 256;
    vi.height = std::max(256,vi.height);
  }

  if (mode == ModeColor) {
    if (!vi.IsPlanar())
      env->ThrowError("Histogram: Color mode only available in PLANAR.");
    vi.width += 256;
    vi.height = std::max(256,vi.height);
  }

  if (mode == ModeColor2) {
    if (!vi.IsPlanar())
      env->ThrowError("Histogram: Color2 mode only available in PLANAR.");

    vi.width += 256;
    vi.height = std::max(256,vi.height);

    for (int y=0; y<24; y++) { // just inside the big circle
      deg15c[y] = (int) ( 126.0*cos(y*3.14159/12.) + 0.5) + 127;
	  deg15s[y] = (int) (-126.0*sin(y*3.14159/12.) + 0.5) + 127;
    }
  }

  if (mode == ModeLuma) {
    if (!vi.IsYUV())
      env->ThrowError("Histogram: Luma mode only available in YUV.");
  }

  if ((mode == ModeStereo)||(mode == ModeOverlay)) {

    child->SetCacheHints(CACHE_AUDIO,4096*1024);

    if (!vi.HasVideo()) {
      mode = ModeStereo; // force mode to ModeStereo.
      vi.fps_numerator = 25;
      vi.fps_denominator = 1;
      vi.num_frames = vi.FramesFromAudioSamples(vi.num_audio_samples);
    }
    if (mode == ModeOverlay)  {
      vi.height = std::max(512, vi.height);
      vi.width = std::max(512, vi.width);
      if(!vi.IsPlanar())
        env->ThrowError("Histogram: StereoOverlay must be YV12");
    } else {
      vi.pixel_type = VideoInfo::CS_YV12;
      vi.height = 512;
      vi.width = 512;
    }
    if (!vi.HasAudio())
      env->ThrowError("Histogram: Stereo mode requires samples!");
    if (vi.AudioChannels() != 2)
      env->ThrowError("Histogram: Stereo mode only works on two audio channels.");

     aud_clip = ConvertAudio::Create(child,SAMPLE_INT16,SAMPLE_INT16);
  }

  if (mode == ModeAudioLevels) {
    child->SetCacheHints(CACHE_AUDIO,4096*1024);
    if (!vi.IsPlanar())
      env->ThrowError("Histogram: Audiolevels mode only available in planar YUV.");

    aud_clip = ConvertAudio::Create(child,SAMPLE_INT16,SAMPLE_INT16);

  }

}

PVideoFrame __stdcall Histogram::GetFrame(int n, IScriptEnvironment* env)
{
  switch (mode) {
  case ModeClassic:
    return DrawModeClassic(n, env);
  case ModeLevels:
    return DrawModeLevels(n, env);
  case ModeColor:
    return DrawModeColor(n, env);
  case ModeColor2:
    return DrawModeColor2(n, env);
  case ModeLuma:
    return DrawModeLuma(n, env);
  case ModeStereo:
    return DrawModeStereo(n, env);
  case ModeOverlay:
    return DrawModeOverlay(n, env);
  case ModeAudioLevels:
    return DrawModeAudioLevels(n, env);
  }
  return DrawModeClassic(n, env);
}

__inline void MixLuma(BYTE &src, int value, int alpha) {
  src += ((value - (int)src) * alpha) >> 8;
}

PVideoFrame Histogram::DrawModeAudioLevels(int n, IScriptEnvironment* env) {
  PVideoFrame src = child->GetFrame(n, env);
  env->MakeWritable(&src);
  const int w = src->GetRowSize();
  const int h = src->GetHeight();
  const int channels = vi.AudioChannels();

  int bar_w = 60;  // Must be divideable by 4 (for subsampling)
  int total_width = (1+channels*2)*bar_w; // Total width in pixels.

  if (total_width > w) {
    bar_w = ((w / (1+channels*2)) / 4)* 4;
  }
  total_width = (1+channels*2)*bar_w; // Total width in pixels.
  int bar_h = vi.height;

  // Get audio for current frame.
  const __int64 start = vi.AudioSamplesFromFrames(n);
  const int count = (int)(vi.AudioSamplesFromFrames(1));
  signed short* samples = new signed short[count*channels];

  aud_clip->GetAudio(samples, std::max((__int64)0,start), count, env);

  // Find maximum volume and rms.
  int*     channel_max = new int[channels];
  __int64* channel_rms = new __int64[channels];

  const int c = count*channels;
  {for (int ch = 0; ch<channels; ch++) {
    int max_vol=0;
    __int64 rms_vol=0;
	
    for (int i=ch; i < c; i+=channels) {
      int sample = samples[i];
	  sample *= sample;
      rms_vol += sample;
      max_vol = std::max(max_vol, sample);
    }
    channel_max[ch] = max_vol;
    channel_rms[ch] = rms_vol;
  }}

  // Draw bars
  BYTE* srcpY = src->GetWritePtr(PLANAR_Y);
  int Ypitch = src->GetPitch(PLANAR_Y);
  BYTE* srcpU = src->GetWritePtr(PLANAR_U);
  BYTE* srcpV = src->GetWritePtr(PLANAR_V);
  int UVpitch = src->GetPitch(PLANAR_U);
  int xSubS = 1; // vi.GetPlaneWidthSubsampling(PLANAR_U);
  int ySubS = 1; // vi.GetPlaneHeightSubsampling(PLANAR_U);

  // Draw Dotted lines
  const int lines = 16;  // Line every 6dB  (96/6)
  int lines_y[lines];
  float line_every = (float)bar_h / (float)lines;
  char text[32];
  for (int i=0; i<lines; i++) {
    lines_y[i] = (int)(line_every*i);
    if (!(i&1)) {
      snprintf(text, sizeof(text), "%3ddB", -i*6);
      DrawString(src, 0, i ? lines_y[i]-10 : 0, text);
    }
  }
  for (int x=bar_w-16; x<total_width-bar_w+16; x++) {
    if (!(x&12)) {
      for (int i=0; i<lines; i++) {
        srcpY[x+lines_y[i]*Ypitch] = 200;
      }
    }
  }

  {for (int ch = 0; ch<channels; ch++) {
    int max = channel_max[ch];
    double ch_db = 96;
    if (max > 0) ch_db = -8.685889638/2. * log((double)max/(32768.0*32768.0));

    __int64 rms = channel_rms[ch] / count;
    double ch_rms = 96;
    if (rms > 0) ch_rms = -8.685889638/2. * log((double)rms/(32768.0*32768.0));

    int x_pos = ((ch*2)+1)*bar_w+8;
    int x_end = x_pos+bar_w-8;
    int y_pos = (int)(((double)bar_h*ch_db) / 96.0);
    int y_mid = (int)(((double)bar_h*ch_rms) / 96.0);
    int y_end = src->GetHeight(PLANAR_Y);
    // Luma                          Red   Blue
    int y_val = (max>=32767*32767) ?  78 :  90;
    int a_val = (max>=32767*32767) ?  96 : 128;
    {for (int y = y_pos; y<y_mid; y++) {
      for (int x = x_pos; x < x_end; x++) {
        MixLuma(srcpY[x+y*Ypitch], y_val, a_val);
      }
    }} //                      Yellow Green
    y_val = (max>=32767*32767) ? 216 : 137;
    a_val = (max>=32767*32767) ? 160 : 128;
    {for (int y = y_mid; y<y_end; y++) {
      for (int x = x_pos; x < x_end; x++) {
        MixLuma(srcpY[x+y*Ypitch], y_val, a_val);
      }
    }}
    // Chroma
    x_pos >>= xSubS;
    x_end >>= xSubS;
    y_pos >>= ySubS;
    y_mid >>= ySubS;
    y_end = src->GetHeight(PLANAR_U);//Red  Blue
    BYTE u_val = (max>=32767*32767) ?  92 : 212;
    BYTE v_val = (max>=32767*32767) ? 233 : 114;
    {for (int y = y_pos; y<y_mid; y++) {
      for (int x = x_pos; x < x_end; x++) {
        srcpU[x+y*UVpitch] = u_val;
        srcpV[x+y*UVpitch] = v_val;
      }
    }} //                      Yellow Green
    u_val = (max>=32767*32767) ?  44 :  58;
    v_val = (max>=32767*32767) ? 142 :  40;
    {for (int y = y_mid; y<y_end; y++) {
      for (int x = x_pos; x < x_end; x++) {
        srcpU[x+y*UVpitch] = u_val;
        srcpV[x+y*UVpitch] = v_val;
      }
    }}
    // Draw text
    snprintf(text, sizeof(text), "%6.2fdB", (float)-ch_db);
    DrawString(src, ((ch*2)+1)*bar_w, vi.height-40, text);
    snprintf(text, sizeof(text), "%6.2fdB", (float)-ch_rms);
    DrawString(src, ((ch*2)+1)*bar_w, vi.height-20, text);

  }}

  delete[] channel_max;
  delete[] channel_rms;
  delete[] samples;
  return src;
}

PVideoFrame Histogram::DrawModeOverlay(int n, IScriptEnvironment* env) {
  PVideoFrame dst = env->NewVideoFrame(vi);

  __int64 start = vi.AudioSamplesFromFrames(n);
  __int64 end = vi.AudioSamplesFromFrames(n+1);
  __int64 count = end-start;
  signed short* samples = new signed short[(int)count*vi.AudioChannels()];

  int w = dst->GetRowSize();
  int h = dst->GetHeight();
  int imgSize = h*dst->GetPitch();
  BYTE* dstp = dst->GetWritePtr();
  int p = dst->GetPitch(PLANAR_Y);

  PVideoFrame src = child->GetFrame(n, env);
  if ((src->GetHeight()<dst->GetHeight()) || (src->GetRowSize() < dst->GetRowSize())) {
    memset(dstp, 16, imgSize);
    int imgSizeU = dst->GetHeight(PLANAR_U) * dst->GetPitch(PLANAR_U);
    if (imgSizeU) {
      memset(dst->GetWritePtr(PLANAR_U), 128, imgSizeU);
      memset(dst->GetWritePtr(PLANAR_V), 128, imgSizeU);
    }
  }

  env->BitBlt(dstp, dst->GetPitch(), src->GetReadPtr(), src->GetPitch(), src->GetRowSize(), src->GetHeight());
  env->BitBlt(dst->GetWritePtr(PLANAR_U), dst->GetPitch(PLANAR_U), src->GetReadPtr(PLANAR_U), src->GetPitch(PLANAR_U), src->GetRowSize(PLANAR_U), src->GetHeight(PLANAR_U));
  env->BitBlt(dst->GetWritePtr(PLANAR_V), dst->GetPitch(PLANAR_V), src->GetReadPtr(PLANAR_V), src->GetPitch(PLANAR_V), src->GetRowSize(PLANAR_V), src->GetHeight(PLANAR_V));

  BYTE* _dstp = dstp;
  for (int iY = 0; iY<512; iY++) {
    for (int iX = 0; iX<512; iX++) {
      _dstp[iX] >>= 1;
    }
    _dstp+=p;
  }

  aud_clip->GetAudio(samples, std::max((__int64)0,start), count, env);

  int c = (int)count;
  for (int i=1; i < c;i++) {
    int l1 = (int)samples[i*2-2];
    int r1 = (int)samples[i*2-1];
    int l2 = (int)samples[i*2];
    int r2 = (int)samples[i*2+1];
    for (int s = 0 ; s < 8; s++) {  // 8 times supersampling (linear)
      int l = (l1*s) + (l2*(8-s));
      int r = (r1*s) + (r2*(8-s));
      int y = 256+((l+r)>>11);
      int x = 256+((l-r)>>11);
      int v = dstp[x+y*p]+48;
      dstp[x+y*p] = std::min(v,235);
    }
  }

  int y_off = p*256;
  for (int x = 0; x < 512; x+=16)
    dstp[y_off + x] = (dstp[y_off + x] > 127) ? 16 : 235;

  for (int y = 0; y < 512;y+=16)
    dstp[y*p+256] = (dstp[y*p+256]>127) ? 16 : 235 ;

  delete[] samples;
  return dst;
}


PVideoFrame Histogram::DrawModeStereo(int n, IScriptEnvironment* env) {
  PVideoFrame src = env->NewVideoFrame(vi);
  env->MakeWritable(&src);
  __int64 start = vi.AudioSamplesFromFrames(n);
  __int64 end = vi.AudioSamplesFromFrames(n+1);
  __int64 count = end-start;
  signed short* samples = new signed short[(int)count*vi.AudioChannels()];

  int w = src->GetRowSize();
  int h = src->GetHeight();
  int imgSize = h*src->GetPitch();
  BYTE* srcp = src->GetWritePtr();
  memset(srcp, 16, imgSize);
  int p = src->GetPitch();

  aud_clip->GetAudio(samples, std::max((__int64)0,start), count, env);

  int c = (int)count;
  for (int i=1; i < c;i++) {
    int l1 = (int)samples[i*2-2];
    int r1 = (int)samples[i*2-1];
    int l2 = (int)samples[i*2];
    int r2 = (int)samples[i*2+1];
    for (int s = 0 ; s < 8; s++) {  // 8 times supersampling (linear)
      int l = (l1*s) + (l2*(8-s));
      int r = (r1*s) + (r2*(8-s));
      int y = 256+((l+r)>>11);
      int x = 256+((l-r)>>11);
      int v = srcp[x+y*512]+48;
      srcp[x+y*512] = std::min(v,235);
    }
  }

  int y_off = p*256;
  for (int x = 0; x < 512; x+=16)
    srcp[y_off + x] = (srcp[y_off + x] > 127) ? 16 : 235;

  for (int y = 0; y < 512;y+=16)
    srcp[y*p+256] = (srcp[y*p+256]>127) ? 16 : 235 ;

  srcp = src->GetWritePtr(PLANAR_U);
  imgSize = src->GetHeight(PLANAR_U) * src->GetPitch(PLANAR_U);
  memset(srcp, 128, imgSize);
  srcp = src->GetWritePtr(PLANAR_V);
  memset(srcp, 128, imgSize);

  delete[] samples;
  return src;
}


PVideoFrame Histogram::DrawModeLuma(int n, IScriptEnvironment* env) {
  PVideoFrame src = child->GetFrame(n, env);
  env->MakeWritable(&src);
  int w = src->GetRowSize();
  int h = src->GetHeight();
  int imgsize = h*src->GetPitch();
  BYTE* srcp = src->GetWritePtr();
  if (vi.IsYUY2()) {
    for (int i=0; i<imgsize; i+=2) {
      int p = srcp[i];
      p<<=4;
      srcp[i+1] = 128;
      srcp[i] = (p&256) ? (255-(p&0xff)) : p&0xff;
    }
  } else {
    for (int i=0; i<imgsize; i++) {
      int p = srcp[i];
      p<<=4;
      srcp[i] = (p&256) ? (255-(p&0xff)) : p&0xff;
    }
  }
  if (vi.IsPlanar()) {
    srcp = src->GetWritePtr(PLANAR_U);
    imgsize = src->GetHeight(PLANAR_U) * src->GetPitch(PLANAR_U);
    memset(srcp, 128, imgsize);
    srcp = src->GetWritePtr(PLANAR_V);
    memset(srcp, 128, imgsize);
  }
  return src;
}


PVideoFrame Histogram::DrawModeColor2(int n, IScriptEnvironment* env) {
  PVideoFrame dst = env->NewVideoFrame(vi);
  BYTE* pdst = dst->GetWritePtr();
  PVideoFrame src = child->GetFrame(n, env);

  int imgSize = dst->GetHeight()*dst->GetPitch();

  if (src->GetHeight()<dst->GetHeight()) {
    memset(dst->GetWritePtr(PLANAR_Y), 16, imgSize);
    int imgSizeU = dst->GetHeight(PLANAR_U) * dst->GetPitch(PLANAR_U);
    memset(dst->GetWritePtr(PLANAR_U), 128, imgSizeU);
    memset(dst->GetWritePtr(PLANAR_V), 128, imgSizeU);
  }


  env->BitBlt(pdst, dst->GetPitch(), src->GetReadPtr(), src->GetPitch(), src->GetRowSize(), src->GetHeight());
  if (vi.IsPlanar()) {
    env->BitBlt(dst->GetWritePtr(PLANAR_U), dst->GetPitch(PLANAR_U), src->GetReadPtr(PLANAR_U), src->GetPitch(PLANAR_U), src->GetRowSize(PLANAR_U), src->GetHeight(PLANAR_U));
    env->BitBlt(dst->GetWritePtr(PLANAR_V), dst->GetPitch(PLANAR_V), src->GetReadPtr(PLANAR_V), src->GetPitch(PLANAR_V), src->GetRowSize(PLANAR_V), src->GetHeight(PLANAR_V));

    unsigned char* pdstb = pdst;
    unsigned char* pdstbU = dst->GetWritePtr(PLANAR_U);
    unsigned char* pdstbV = dst->GetWritePtr(PLANAR_V);
    pdstb += src->GetRowSize(PLANAR_Y);

    int swidth = 1; // vi.GetPlaneWidthSubsampling(PLANAR_U);
    int sheight = 1; // vi.GetPlaneHeightSubsampling(PLANAR_U);

    int p = dst->GetPitch(PLANAR_Y);
    int p2 = dst->GetPitch(PLANAR_U);

    // Erase all - luma
    for (int y=0; y<dst->GetHeight(PLANAR_Y); y++) {
      memset(&pdstb[y*dst->GetPitch(PLANAR_Y)], 16, 256);
    }

    // Erase all - chroma
    pdstbU = dst->GetWritePtr(PLANAR_U);
    pdstbU += src->GetRowSize(PLANAR_U);
    pdstbV = dst->GetWritePtr(PLANAR_V);
    pdstbV += src->GetRowSize(PLANAR_V);

    for (int y=0; y<dst->GetHeight(PLANAR_U); y++) {
      memset(&pdstbU[y*dst->GetPitch(PLANAR_U)], 128, (256>>swidth));
      memset(&pdstbV[y*dst->GetPitch(PLANAR_V)], 128, (256>>swidth));
    }


    // plot valid grey ccir601 square
    pdstb = pdst;
    pdstb += src->GetRowSize(PLANAR_Y);

    memset(&pdstb[(16*p)+16], 128, 225);
    memset(&pdstb[(240*p)+16], 128, 225);
    for (int y=17; y<240; y++) {
      pdstb[16+y*p] = 128;
      pdstb[240+y*p] = 128;
    }

    // plot circles
    pdstb = pdst;
    pdstbU = dst->GetWritePtr(PLANAR_U);
    pdstbV = dst->GetWritePtr(PLANAR_V);
    pdstb += src->GetRowSize(PLANAR_Y);
    pdstbU += src->GetRowSize(PLANAR_U);
    pdstbV += src->GetRowSize(PLANAR_V);

    // six hues in the color-wheel:
    // LC[3j,3j+1,3j+2], RC[3j,3j+1,3j+2] in YRange[j]+1 and YRange[j+1]
    int YRange[8] = {-1, 26, 104, 127, 191, 197, 248, 256};
    // 2x green, 2x yellow, 3x red
    int LC[21] = {145,54,34, 145,54,34, 210,16,146, 210,16,146, 81,90,240, 81,90,240, 81,90,240};
    // cyan, 4x blue, magenta, red:
    int RC[21] = {170,166,16, 41,240,110, 41,240,110, 41,240,110, 41,240,110, 106,202,222, 81,90,240};

    // example boundary of cyan and blue:
    // red = std::min(r,g,b), blue if g < 2/3 b, green if b < 2/3 g.
    // cyan between green and blue.
    // thus boundary of cyan and blue at (r,g,b) = (0,170,255), since 2/3*255 = 170.
    // => yuv = (127,190,47); hue = -52 degr; sat = 103
    // => u'v' = (207,27) (same hue, sat=128)
    // similar for the other hues.
    // luma

    float innerF = 124.9f;  // .9 is for better visuals in subsampled mode
    float thicknessF = 1.5f;
    float oneOverThicknessF = 1.0f/thicknessF;
    float outerF = innerF + thicknessF*2.0f;
    float centerF = innerF + thicknessF;
    int innerSq = (int)(innerF*innerF);
    int outerSq = (int)(outerF*outerF);
    int activeY = 0;
    int xRounder = (1<<swidth) / 2;
    int yRounder = (1<<sheight) / 2;

    for (int y=-127; y<128; y++) {
      if (y+127 > YRange[activeY+1]) activeY++;
      for (int x=-127; x<=0; x++) {
        int distSq = x*x+y*y;
        if (distSq <= outerSq && distSq >= innerSq) {
          int interp = (int)(256.0f - ( 255.9f * (oneOverThicknessF * fabs(sqrt((float)distSq)- centerF))));
          // 255.9 is to account for float inprecision, which could cause underflow.

          int xP = 127 + x;
          int yP = 127 + y;

          pdstb[xP+yP*p] = (interp*LC[3*activeY])>>8; // left upper half
          pdstb[255-xP+yP*p] = (interp*RC[3*activeY])>>8; // right upper half

          xP = (xP+xRounder) >> swidth;
          yP = (yP+yRounder) >> sheight;

          interp = std::min(256,interp);
          int invInt = (256-interp);

          pdstbU[xP+yP*p2] = (pdstbU[xP+yP*p2] * invInt + interp * LC[3*activeY+1])>>8; // left half
          pdstbV[xP+yP*p2] = (pdstbV[xP+yP*p2] * invInt + interp * LC[3*activeY+2])>>8; // left half

          xP = ((255)>>swidth) -xP;
          pdstbU[xP+yP*p2] = (pdstbU[xP+yP*p2] * invInt + interp * RC[3*activeY+1])>>8; // right half
          pdstbV[xP+yP*p2] = (pdstbV[xP+yP*p2] * invInt + interp * RC[3*activeY+2])>>8; // right half
        }
      }
    }

    // plot white 15 degree marks
    pdstb = pdst;
    pdstb += src->GetRowSize(PLANAR_Y);

    for (int y=0; y<24; y++) {
      pdstb[deg15c[y]+deg15s[y]*p] = 235;
    }

    // plot vectorscope
    pdstb = pdst;
    pdstb += src->GetRowSize(PLANAR_Y);

    const int src_height = src->GetHeight(PLANAR_Y);
    const int src_width = src->GetRowSize(PLANAR_Y);
    const int src_pitch = src->GetPitch(PLANAR_Y);

    const int src_heightUV = src->GetHeight(PLANAR_U);
    const int src_widthUV = src->GetRowSize(PLANAR_U);
    const int src_pitchUV = src->GetPitch(PLANAR_U);

    const BYTE* pY = src->GetReadPtr(PLANAR_Y);
    const BYTE* pU = src->GetReadPtr(PLANAR_U);
    const BYTE* pV = src->GetReadPtr(PLANAR_V);
    int Xadd = 1<<swidth;
    int Yadd = 1<<sheight;

    for (int y=0; y<src_heightUV; y++) {
      for (int x=0; x<src_widthUV; x++) {
        int uval = pU[x];
        int vval = pV[x];
        pdstb[uval+vval*p] = pY[x<<swidth];
        pdstbU[(uval>>swidth)+(vval>>sheight)*p2] = uval;
        pdstbV[(uval>>swidth)+(vval>>sheight)*p2] = vval;
      }
      pY += (src_pitch<<sheight);
      pU += src_pitchUV;
      pV += src_pitchUV;
    }

  }

  return dst;
}


PVideoFrame Histogram::DrawModeColor(int n, IScriptEnvironment* env) {
  PVideoFrame dst = env->NewVideoFrame(vi);
  BYTE* p = dst->GetWritePtr();
  PVideoFrame src = child->GetFrame(n, env);

  int imgSize = dst->GetHeight()*dst->GetPitch();

  if (src->GetHeight()<dst->GetHeight()) {
    memset(p, 16, imgSize);
    int imgSizeU = dst->GetHeight(PLANAR_U) * dst->GetPitch(PLANAR_U);
    memset(dst->GetWritePtr(PLANAR_U), 128, imgSizeU);
    memset(dst->GetWritePtr(PLANAR_V), 128, imgSizeU);
  }


  env->BitBlt(p, dst->GetPitch(), src->GetReadPtr(), src->GetPitch(), src->GetRowSize(), src->GetHeight());
  if (vi.IsPlanar()) {
    env->BitBlt(dst->GetWritePtr(PLANAR_U), dst->GetPitch(PLANAR_U), src->GetReadPtr(PLANAR_U), src->GetPitch(PLANAR_U), src->GetRowSize(PLANAR_U), src->GetHeight(PLANAR_U));
    env->BitBlt(dst->GetWritePtr(PLANAR_V), dst->GetPitch(PLANAR_V), src->GetReadPtr(PLANAR_V), src->GetPitch(PLANAR_V), src->GetRowSize(PLANAR_V), src->GetHeight(PLANAR_V));
    int histUV[256*256] = {0};

    const BYTE* pU = src->GetReadPtr(PLANAR_U);
    const BYTE* pV = src->GetReadPtr(PLANAR_V);

    int w = src->GetRowSize(PLANAR_U);
    int p = src->GetPitch(PLANAR_U);

    for (int y = 0; y < src->GetHeight(PLANAR_U); y++) {
      for (int x = 0; x < w; x++) {
        int u = pU[y*p+x];
        int v = pV[y*p+x];
        histUV[v*256+u]++;
      }
    }

    // Plot Histogram on Y.
    int maxval = 1;

    // Should we adjust the divisor (maxval)??

    unsigned char* pdstb = dst->GetWritePtr(PLANAR_Y);
    pdstb += src->GetRowSize(PLANAR_Y);

    // Erase all
    for (int y=256;y<dst->GetHeight();y++) {
      int p = dst->GetPitch(PLANAR_Y);
      for (int x=0;x<256;x++) {
        pdstb[x+y*p] = 16;
      }
    }

    for (int y=0;y<256;y++) {
      for (int x=0;x<256;x++) {
        int disp_val = histUV[x+y*256]/maxval;
        if (y<16 || y>240 || x<16 || x>240)
          disp_val -= 16;

        pdstb[x] = std::min(235, 16 + disp_val);

      }
      pdstb += dst->GetPitch(PLANAR_Y);
    }

    // Draw colors.
    pdstb = dst->GetWritePtr(PLANAR_U);
    pdstb += src->GetRowSize(PLANAR_U);

	int swidth = 1; // vi.GetPlaneWidthSubsampling(PLANAR_U);
	int sheight = 1; // vi.GetPlaneHeightSubsampling(PLANAR_U);

    // Erase all
    for (int y=(256>>sheight); y<dst->GetHeight(PLANAR_U); y++) {
      memset(&pdstb[y*dst->GetPitch(PLANAR_U)], 128, (256>>swidth)-1);
    }

    for (int y=0; y<(256>>sheight); y++) {
      for (int x=0; x<(256>>swidth); x++) {
        pdstb[x] = x<<swidth;
      }
      pdstb += dst->GetPitch(PLANAR_U);
    }

    pdstb = dst->GetWritePtr(PLANAR_V);
    pdstb += src->GetRowSize(PLANAR_V);

    // Erase all
    for (int y=(256>>sheight); y<dst->GetHeight(PLANAR_U); y++) {
      memset(&pdstb[y*dst->GetPitch(PLANAR_V)], 128, (256>>swidth)-1);
    }

    for (int y=0; y<(256>>sheight); y++) {
      for (int x=0; x<(256>>swidth); x++) {
        pdstb[x] = y<<sheight;
      }
      pdstb += dst->GetPitch(PLANAR_V);
    }

  }
  return dst;
}


PVideoFrame Histogram::DrawModeLevels(int n, IScriptEnvironment* env) {
  PVideoFrame dst = env->NewVideoFrame(vi);
  BYTE* p = dst->GetWritePtr();
  PVideoFrame src = child->GetFrame(n, env);

  int imgSize = dst->GetHeight()*dst->GetPitch();
  if (src->GetHeight()<dst->GetHeight()) {
    memset(p, 16, imgSize);
    int imgSizeU = dst->GetHeight(PLANAR_U) * dst->GetPitch(PLANAR_U);
    memset(dst->GetWritePtr(PLANAR_U) , 128, imgSizeU);
    memset(dst->GetWritePtr(PLANAR_V), 128, imgSizeU);
  }

  env->BitBlt(p, dst->GetPitch(), src->GetReadPtr(), src->GetPitch(), src->GetRowSize(), src->GetHeight());
  if (vi.IsPlanar()) {
    env->BitBlt(dst->GetWritePtr(PLANAR_U), dst->GetPitch(PLANAR_U), src->GetReadPtr(PLANAR_U), src->GetPitch(PLANAR_U), src->GetRowSize(PLANAR_U), src->GetHeight(PLANAR_U));
    env->BitBlt(dst->GetWritePtr(PLANAR_V), dst->GetPitch(PLANAR_V), src->GetReadPtr(PLANAR_V), src->GetPitch(PLANAR_V), src->GetRowSize(PLANAR_V), src->GetHeight(PLANAR_V));

    int histY[256] = {0};
    int histU[256] = {0};
    int histV[256] = {0};

    const BYTE* pY = src->GetReadPtr(PLANAR_Y);
    const BYTE* pU = src->GetReadPtr(PLANAR_U);
    const BYTE* pV = src->GetReadPtr(PLANAR_V);

    int wy = src->GetRowSize(PLANAR_Y);
	int wu = src->GetRowSize(PLANAR_U);
    int p = src->GetPitch(PLANAR_U);
    int pitY = src->GetPitch(PLANAR_Y);

	// luma
    for (int y = 0; y < src->GetHeight(PLANAR_Y); y++) {
      for (int x = 0; x < wy; x++) {
        histY[pY[y*pitY+x]]++;
      }
    }

	// chroma
    for (int y2 = 0; y2 < src->GetHeight(PLANAR_U); y2++) {
      for (int x = 0; x < wu; x++) {
        histU[pU[y2*p+x]]++;
        histV[pV[y2*p+x]]++;
      }
    }

    unsigned char* pdstb = dst->GetWritePtr(PLANAR_Y);
    pdstb += src->GetRowSize(PLANAR_Y);

    // Clear Y
    for (int y=0;y<dst->GetHeight();y++) {
      memset(&pdstb[y*dst->GetPitch(PLANAR_Y)], 16, 256);
    }

    int dstPitch = dst->GetPitch(PLANAR_Y);

    // Draw Unsafe zone (UV-graph)
    for (int y=64+16; y<128+16+2; y++) {
      for (int x=0; x<16; x++) {
        pdstb[dstPitch*y+x] = 32;
        pdstb[dstPitch*y+x+240] = 32;
        pdstb[dstPitch*(y+80)+x] = 32;
        pdstb[dstPitch*(y+80)+x+240] = 32;
      }
    }

    // Draw dotted centerline
    for (int y=0; y<=256-32; y++) {
      if ((y&3)>1)
        pdstb[dstPitch*y+128] = 128;
    }

    // Draw Y histograms
    int maxval = 0;
    for (int i=0;i<256;i++) {
      maxval = std::max(histY[i], maxval);
    }

    float scale = 64.0f / maxval;

    for (int x=0;x<256;x++) {
      float scaled_h = (float)histY[x] * scale;
      int h = 64 -  std::min((int)scaled_h, 64)+1;
      int left = (int)(220.0f*(scaled_h-(float)((int)scaled_h)));

      for (int y=64+1 ; y > h ; y--) {
        pdstb[x+y*dstPitch] = 235;
      }
      pdstb[x+h*dstPitch] = 16+left;
    }

    // Draw U
    maxval = 0;
    for (int i=0; i<256 ;i++) {
      maxval = std::max(histU[i], maxval);
    }

    scale = 64.0f / maxval;

    for (int x=0;x<256;x++) {
      float scaled_h = (float)histU[x] * scale;
      int h = 128+16 -  std::min((int)scaled_h, 64)+1;
      int left = (int)(220.0f*(scaled_h-(float)((int)scaled_h)));

      for (int y=128+16+1 ; y > h ; y--) {
        pdstb[x+y*dstPitch] = 235;
      }
      pdstb[x+h*dstPitch] = 16+left;
    }

    // Draw V
    maxval = 0;
    for (int i=0; i<256 ;i++) {
      maxval = std::max(histV[i], maxval);
    }

    scale = 64.0f / maxval;

    for (int x=0;x<256;x++) {
      float scaled_h = (float)histV[x] * scale;
      int h = 192+32 -  std::min((int)scaled_h, 64)+1;
      int left = (int)(220.0f*((int)scaled_h-scaled_h));
      for (int y=192+32+1 ; y > h ; y--) {
        pdstb[x+y*dstPitch] = 235;
      }
      pdstb[x+h*dstPitch] = 16+left;
    }

    // Draw chroma
    unsigned char* pdstbU = dst->GetWritePtr(PLANAR_U);
    unsigned char* pdstbV = dst->GetWritePtr(PLANAR_V);
    pdstbU += src->GetRowSize(PLANAR_U);
    pdstbV += src->GetRowSize(PLANAR_V);

    // Clear chroma
    int dstPitchUV = dst->GetPitch(PLANAR_U);
	int swidth = 1; // vi.GetPlaneWidthSubsampling(PLANAR_U);
	int sheight = 1; // vi.GetPlaneHeightSubsampling(PLANAR_U);

    for (int y=0; y<dst->GetHeight(PLANAR_U); y++) {
      memset(&pdstbU[y*dstPitchUV], 128, 256>>swidth);
      memset(&pdstbV[y*dstPitchUV], 128, 256>>swidth);
    }

    // Draw Unsafe zone (Y-graph)
    for (int y=0; y<=(64>>sheight); y++) {
      for (int x=0; x<(16>>swidth); x++) {
        pdstbV[dstPitchUV*y+x] = 160;
        pdstbU[dstPitchUV*y+x] = 16;

      }
      for (int x=(236>>swidth); x<(256>>swidth); x++) {
        pdstbV[dstPitchUV*y+x] = 160;
        pdstbU[dstPitchUV*y+x] = 16;
      }
    }

    // Draw upper gradient
    for (int y=((64+16)>>sheight); y<=((128+16)>>sheight); y++) {
      for (int x=0; x<(256>>swidth); x++) {
        pdstbU[dstPitchUV*y+x] = x<<swidth;
      }
    }

    //  Draw lower gradient
    for (int y=((128+32)>>sheight); y<=((128+64+32)>>sheight); y++) {
      for (int x=0; x<(256>>swidth); x++) {
        pdstbV[dstPitchUV*y+x] = x<<swidth;
      }
    }
  }

  return dst;
}


PVideoFrame Histogram::DrawModeClassic(int n, IScriptEnvironment* env)
{
  static BYTE exptab[256];
  static bool init = false;
  static int E167;

  if (!init) {
	init = true;

	const double K = log(0.5/219)/255; // approx -1/42

	exptab[0] = 16;
    for (int i=1; i<255; i++) {
	  exptab[i] = BYTE(16.5 + 219 * (1-exp(i*K)));
	  if (exptab[i] <= 235-68) E167 = i;
	}
	exptab[255] = 235;
  }

  const int w = vi.width-256;

  PVideoFrame dst = env->NewVideoFrame(vi);
  BYTE* p = dst->GetWritePtr();
  PVideoFrame src = child->GetFrame(n, env);
  env->BitBlt(p, dst->GetPitch(), src->GetReadPtr(), src->GetPitch(), src->GetRowSize(), src->GetHeight());
  if (vi.IsPlanar()) {
    env->BitBlt(dst->GetWritePtr(PLANAR_U), dst->GetPitch(PLANAR_U), src->GetReadPtr(PLANAR_U), src->GetPitch(PLANAR_U), src->GetRowSize(PLANAR_U), src->GetHeight(PLANAR_U));
    env->BitBlt(dst->GetWritePtr(PLANAR_V), dst->GetPitch(PLANAR_V), src->GetReadPtr(PLANAR_V), src->GetPitch(PLANAR_V), src->GetRowSize(PLANAR_V), src->GetHeight(PLANAR_V));

	// luma
    for (int y=0; y<src->GetHeight(PLANAR_Y); ++y) {
      int hist[256] = {0};
      int x;
      for (x=0; x<w; ++x) {
        hist[p[x]]++;
      }
      BYTE* const q = p + w;
      for (x=0; x<256; ++x) {
        if (x<16 || x==124 || x>234) {
          q[x] = exptab[std::min(E167, hist[x])] + 68;
        } else {
	      q[x] = exptab[std::min(255, hist[x])];
        }
      }
      p += dst->GetPitch();
    }

	// chroma
    if (dst->GetPitch(PLANAR_U)) {
      const int subs = 1; // vi.GetPlaneWidthSubsampling(PLANAR_U);
      const int fact = 1<<subs;

      BYTE* p2 = dst->GetWritePtr(PLANAR_U) + (w >> subs);
      BYTE* p3 = dst->GetWritePtr(PLANAR_V) + (w >> subs);

      for (int y2=0; y2<src->GetHeight(PLANAR_U); ++y2) {
        for (int x=0; x<256; x+=fact) {
          if (x<16 || x>235) {
            p2[x >> subs] = 16;
            p3[x >> subs] = 160;
          } else if (x==124) {
            p2[x >> subs] = 160;
            p3[x >> subs] = 16;
          } else {
            p2[x >> subs] = 128;
            p3[x >> subs] = 128;
          }
        }
        p2 += dst->GetPitch(PLANAR_U);
        p3 += dst->GetPitch(PLANAR_V);
      }
    }
  } else {
    for (int y=0; y<src->GetHeight(); ++y) { // YUY2
      int hist[256] = {0};
      int x;
      for (x=0; x<w; ++x) {
        hist[p[x*2]]++;
	  }
      BYTE* const q = p + w*2;
      for (x=0; x<256; x+=2) {
        if (x<16 || x>235) {
          q[x*2+0] = exptab[std::min(E167, hist[x])] + 68;
		  q[x*2+1] = 16;
          q[x*2+2] = exptab[std::min(E167, hist[x+1])] + 68;
		  q[x*2+3] = 160;
		} else if (x==124) {
          q[x*2+0] = exptab[std::min(E167, hist[x])] + 68;
	      q[x*2+1] = 160;
          q[x*2+2] = exptab[std::min(E167, hist[x+1])] + 68;
		  q[x*2+3] = 16;
		} else {
	      q[x*2+0] = exptab[std::min(255, hist[x])];
		  q[x*2+1] = 128;
          q[x*2+2] = exptab[std::min(255, hist[x+1])];
		  q[x*2+3] = 128;
		}
	  }
      p += dst->GetPitch();
	}
  }
  return dst;
}


AVSValue __cdecl Histogram::Create(AVSValue args, void*, IScriptEnvironment* env)
{
  const char* st_m = args[1].AsString("classic");

  Mode mode = ModeClassic;

  if (!lstrcmpi(st_m, "classic"))
    mode = ModeClassic;

  if (!lstrcmpi(st_m, "levels"))
    mode = ModeLevels;

  if (!lstrcmpi(st_m, "color"))
    mode = ModeColor;

  if (!lstrcmpi(st_m, "color2"))
    mode = ModeColor2;

  if (!lstrcmpi(st_m, "luma"))
    mode = ModeLuma;

  if (!lstrcmpi(st_m, "stereo"))
    mode = ModeStereo;

  if (!lstrcmpi(st_m, "stereooverlay"))
    mode = ModeOverlay;

  if (!lstrcmpi(st_m, "audiolevels"))
    mode = ModeAudioLevels;

  return new Histogram(args[0].AsClip(), mode, env);
}

}; // namespace avxsynth
