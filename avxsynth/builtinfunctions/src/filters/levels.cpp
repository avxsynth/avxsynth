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

#include "levels.h"

#include <text-overlay.h>


namespace avxsynth {


/********************************************************************
***** Declare index of new filters for Avisynth's filter engine *****
********************************************************************/

AVSFunction Levels_filters[] = {
  { "Levels", "cifiii[coring]b", Levels::Create },        // src_low, gamma, src_high, dst_low, dst_high 
  { "RGBAdjust", "c[r]f[g]f[b]f[a]f[rb]f[gb]f[bb]f[ab]f[rg]f[gg]f[bg]f[ag]f[analyze]b", RGBAdjust::Create },
  { "Tweak", "c[hue]f[sat]f[bright]f[cont]f[coring]b[sse]b[startHue]f[endHue]f[maxSat]f[minSat]f[interp]f", Tweak::Create },
  { "Limiter", "c[min_luma]i[max_luma]i[min_chroma]i[max_chroma]i[show]s", Limiter::Create },
  { 0 }
};



/********************************
 *******   Levels Filter   ******
 ********************************/

Levels::Levels( PClip _child, int in_min, double gamma, int in_max, int out_min, int out_max, bool coring, 
                IScriptEnvironment* env )
  : GenericVideoFilter(_child)
{
	try {	// HIDE DAMN SEH COMPILER BUG!!!
  if (gamma <= 0.0)
    env->ThrowError("Levels: gamma must be positive");
  gamma = 1/gamma;
  int divisor = in_max - in_min + (in_max == in_min);
  if (vi.IsYUV()) 
  {
    for (int i=0; i<256; ++i) 
    {
      double p;

      if (coring) 
        p = ((i-16)*(255.0/219.0) - in_min) / divisor;
      else 
        p = double(i - in_min) / divisor;

      p = pow(std::min(std::max(p, 0.0), 1.0), gamma);
      p = p * (out_max - out_min) + out_min;
      int pp;

      if (coring) 
        pp = int(p*(219.0/255.0)+16.5);
      else 
        pp = int(p+0.5);

      map[i] = std::min(std::max(pp, (coring) ? 16 : 0), (coring) ? 235 : 255);

      int q = ((i-128) * (out_max-out_min) + (divisor>>1)) / divisor + 128;
      mapchroma[i] = std::min(std::max(q, (coring) ? 16 : 0), (coring) ? 240 : 255);
    }
  } else if (vi.IsRGB()) {
    for (int i=0; i<256; ++i) 
    {
      double p = double(i - in_min) / divisor;
      p = pow(std::min(std::max(p, 0.0), 1.0), gamma);
      p = p * (out_max - out_min) + out_min;
      map[i] = PixelClip(int(p+0.5));
    }
  }
	}
	catch (...) { throw; }
}

PVideoFrame __stdcall Levels::GetFrame(int n, IScriptEnvironment* env) 
{
  PVideoFrame frame = child->GetFrame(n, env);
  env->MakeWritable(&frame);
  BYTE* p = frame->GetWritePtr();
  int pitch = frame->GetPitch();
  if (vi.IsYUY2()) {
    for (int y=0; y<vi.height; ++y) {
      for (int x=0; x<vi.width; ++x) {
        p[x*2] = map[p[x*2]];
        p[x*2+1] = mapchroma[p[x*2+1]];
      }
      p += pitch;
    }
  } 
  else if (vi.IsPlanar()){
    for (int y=0; y<vi.height; ++y) {
      for (int x=0; x<vi.width; ++x) {
        p[x] = map[p[x]];
      }
      p += pitch;
    }
    pitch = frame->GetPitch(PLANAR_U);
    p = frame->GetWritePtr(PLANAR_U);
    int w=frame->GetRowSize(PLANAR_U);
    int h=frame->GetHeight(PLANAR_U);
    for (int y=0; y<h; ++y) {
      for (int x=0; x<w; ++x) {
        p[x] = mapchroma[p[x]];
      }
      p += pitch;
    }
    p = frame->GetWritePtr(PLANAR_V);
    for (int y=0; y<h; ++y) {
      for (int x=0; x<w; ++x) {
        p[x] = mapchroma[p[x]];
      }
      p += pitch;
    }

  } else if (vi.IsRGB()) {
    const int row_size = frame->GetRowSize();
    for (int y=0; y<vi.height; ++y) {
      for (int x=0; x<row_size; ++x) {
        p[x] = map[p[x]];
      }
      p += pitch;
    }
  }
  return frame;
}

AVSValue __cdecl Levels::Create(AVSValue args, void*, IScriptEnvironment* env) 
{
  return new Levels( args[0].AsClip(), args[1].AsInt(), args[2].AsFloat(), args[3].AsInt(), 
                     args[4].AsInt(), args[5].AsInt(), args[6].AsBool(true), env );
}








/********************************
 *******    RGBA Filter    ******
 ********************************/

RGBAdjust::RGBAdjust(PClip _child, double r,  double g,  double b,  double a,
                                   double rb, double gb, double bb, double ab,
                                   double rg, double gg, double bg, double ag,
                                   bool _analyze, IScriptEnvironment* env)
  : GenericVideoFilter(_child), analyze(_analyze)
{
	try {	// HIDE DAMN SEH COMPILER BUG!!!
  if (!vi.IsRGB())
    env->ThrowError("RGBAdjust requires RGB input");

  if ((rg <= 0.0) || (gg <= 0.0) || (bg <= 0.0) || (ag <= 0.0))
    env->ThrowError("RGBAdjust: gammas must be positive");

  rg=1/rg; gg=1/gg; bg=1/bg; ag=1/ag;

  for (int i=0; i<256; ++i) 
  {
	mapR[i] = int(pow(std::min(std::max((rb + i * r)/255.0, 0.0), 1.0), rg) * 255.0 + 0.5);
	mapG[i] = int(pow(std::min(std::max((gb + i * g)/255.0, 0.0), 1.0), gg) * 255.0 + 0.5);
	mapB[i] = int(pow(std::min(std::max((bb + i * b)/255.0, 0.0), 1.0), bg) * 255.0 + 0.5);
	mapA[i] = int(pow(std::min(std::max((ab + i * a)/255.0, 0.0), 1.0), ag) * 255.0 + 0.5);
  }
	}
	catch (...) { throw; }
}


PVideoFrame __stdcall RGBAdjust::GetFrame(int n, IScriptEnvironment* env) 
{
  PVideoFrame frame = child->GetFrame(n, env);
  env->MakeWritable(&frame);
  BYTE* p = frame->GetWritePtr();
  const int pitch = frame->GetPitch();

  if (vi.IsRGB32())
  {
    for (int y=0; y<vi.height; ++y) 
    {
      for (int x=0; x < vi.width; ++x) 
      {
        p[x*4] = mapB[p[x*4]];
        p[x*4+1] = mapG[p[x*4+1]];
        p[x*4+2] = mapR[p[x*4+2]];
        p[x*4+3] = mapA[p[x*4+3]];
      }
      p += pitch;
    }
  }
  else if (vi.IsRGB24()) {
    const int row_size = frame->GetRowSize();
    for (int y=0; y<vi.height; ++y) 
    {
      for (int x=0; x<row_size; x+=3) 
      {
         p[x] = mapB[p[x]];
         p[x+1] = mapG[p[x+1]];
         p[x+2] = mapR[p[x+2]];
      }
      p += pitch;
    }
  }

	if (analyze) {
		const int w = frame->GetRowSize();
		const int h = frame->GetHeight();
		const int t= vi.IsRGB24() ? 3 : 4;
		unsigned int accum_r[256], accum_g[256], accum_b[256];
		
		p = frame->GetWritePtr();

		for (int i=0;i<256;i++) {
			accum_r[i]=0;
			accum_g[i]=0;
			accum_b[i]=0;
		}

		for (int y=0;y<h;y++) {
			for (int x=0;x<w;x+=t) {
				accum_r[p[x+2]]++;
				accum_g[p[x+1]]++;
				accum_b[p[x]]++;
			}
			p += pitch;
		}
	
		int pixels = vi.width*vi.height;
		float avg_r=0, avg_g=0, avg_b=0;
		float st_r=0, st_g=0, st_b=0;
		int min_r=0, min_g=0, min_b=0;
		int max_r=0, max_g=0, max_b=0;
		bool hit_r=false, hit_g=false, hit_b=false;
		int Amin_r=0, Amin_g=0, Amin_b=0;
		int Amax_r=0, Amax_g=0, Amax_b=0;
		bool Ahit_minr=false,Ahit_ming=false,Ahit_minb=false;
		bool Ahit_maxr=false,Ahit_maxg=false,Ahit_maxb=false;
		int At_256=(pixels+128)/256; // When 1/256th of all pixels have been reached, trigger "Loose min/max"

		
		for (int i=0;i<256;i++) {
			avg_r+=(float)accum_r[i]*(float)i;
			avg_g+=(float)accum_g[i]*(float)i;
			avg_b+=(float)accum_b[i]*(float)i;

			if (accum_r[i]!=0) {max_r=i;hit_r=true;} else {if (!hit_r) min_r=i+1;} 
			if (accum_g[i]!=0) {max_g=i;hit_g=true;} else {if (!hit_g) min_g=i+1;} 
			if (accum_b[i]!=0) {max_b=i;hit_b=true;} else {if (!hit_b) min_b=i+1;} 

			if (!Ahit_minr) {Amin_r+=accum_r[i]; if (Amin_r>At_256){Ahit_minr=true; Amin_r=i;} }
			if (!Ahit_ming) {Amin_g+=accum_g[i]; if (Amin_g>At_256){Ahit_ming=true; Amin_g=i;} }
			if (!Ahit_minb) {Amin_b+=accum_b[i]; if (Amin_b>At_256){Ahit_minb=true; Amin_b=i;} }

			if (!Ahit_maxr) {Amax_r+=accum_r[255-i]; if (Amax_r>At_256){Ahit_maxr=true; Amax_r=255-i;} }
			if (!Ahit_maxg) {Amax_g+=accum_g[255-i]; if (Amax_g>At_256){Ahit_maxg=true; Amax_g=255-i;} }
			if (!Ahit_maxb) {Amax_b+=accum_b[255-i]; if (Amax_b>At_256){Ahit_maxb=true; Amax_b=255-i;} }
		}
		
		float Favg_r=avg_r/pixels;
		float Favg_g=avg_g/pixels;
		float Favg_b=avg_b/pixels;

		for (int i=0;i<256;i++) {
			st_r+=(float)accum_r[i]*(float (i-Favg_r)*(i-Favg_r));
			st_g+=(float)accum_g[i]*(float (i-Favg_g)*(i-Favg_g));
			st_b+=(float)accum_b[i]*(float (i-Favg_b)*(i-Favg_b));
		}
		
		float Fst_r=sqrt(st_r/pixels);
		float Fst_g=sqrt(st_g/pixels);
		float Fst_b=sqrt(st_b/pixels);

		char text[512];
		sprintf(text,
			"             Frame: %-8u (  Red  / Green / Blue  )\n"
			"           Average:      ( %7.2f / %7.2f / %7.2f )\n"
			"Standard Deviation:      ( %7.2f / %7.2f / %7.2f )\n"
			"           Minimum:      ( %3d    / %3d    / %3d    )\n"
			"           Maximum:      ( %3d    / %3d    / %3d    )\n"
			"     Loose Minimum:      ( %3d    / %3d    / %3d    )\n"
			"     Loose Maximum:      ( %3d    / %3d    / %3d    )\n"
			,
			n,
			Favg_r,Favg_g,Favg_b,
			Fst_r,Fst_g,Fst_b,
			min_r,min_g,min_b,
			max_r,max_g,max_b,
			Amin_r,Amin_g,Amin_b,
			Amax_r,Amax_g,Amax_b
			);
		ApplyMessage(&frame, vi, text, vi.width/24, 0xa0a0a0,0,0 , env );
	}
  return frame;
}


AVSValue __cdecl RGBAdjust::Create(AVSValue args, void*, IScriptEnvironment* env) 
{
	try {	// HIDE DAMN SEH COMPILER BUG!!!
  return new RGBAdjust(args[ 0].AsClip(),
                       args[ 1].AsFloat(1), args[ 2].AsFloat(1), args[ 3].AsFloat(1), args[ 4].AsFloat(1),
                       args[ 5].AsFloat(0), args[ 6].AsFloat(0), args[ 7].AsFloat(0), args[ 8].AsFloat(0),
                       args[ 9].AsFloat(1), args[10].AsFloat(1), args[11].AsFloat(1), args[12].AsFloat(1),
                       args[13].AsBool(false), env );
	}
	catch (...) { throw; }
}



/* helper function for Tweak and MaskHS filters */
bool ProcessPixel(int X, int Y, double startHue, double endHue,
                  double maxSat, double minSat, double p, int &iSat)
{
	// a hue analog
	double T = atan2((double)X, (double)Y) * 180.0 / 3.14159265358979323846;
	if ( T < 0.0) T += 360.0;

	// startHue <= hue <= endHue
	if (startHue < endHue) {
		if (T>endHue || T<startHue) return false;
	} else {
		if (T<startHue && T>endHue) return false;
	}

	const double W = X*X + Y*Y;

	// In Range, full adjust but no need to interpolate
	if (minSat*minSat <= W && W <= maxSat*maxSat) return true;

	// p == 0 (no interpolation) needed for MaskHS
	if (p == 0) return false;

	// Interpolation range is +/-p for p>0
	const double max = std::min(maxSat+p, 180.0);
	const double min = std::max(minSat-p,   0.0);

	// Outside of [min-p, max+p] no adjustment
	// minSat-p <= (U^2 + V^2) <= maxSat+p
	if (W < min*min || max*max < W) return false; // don't adjust

	// Interpolate saturation value
	const double holdSat = W < 180.0*180.0 ? sqrt(W) : 180.0;

	if (holdSat < minSat) { // within p of lower range
		iSat += (int)((512 - iSat) * (minSat - holdSat) / (p+1.0)); 
	} else { // within p of upper range
		iSat += (int)((512 - iSat) * (holdSat - maxSat) / (p+1.0));
	}
	
	return true;
}


/**********************
******   Tweak    *****
**********************/

Tweak::Tweak( PClip _child, double _hue, double _sat, double _bright, double _cont, bool _coring, bool _sse,
                            double startHue, double endHue, double _maxSat, double _minSat, double p, 
                            IScriptEnvironment* env ) 
  : GenericVideoFilter(_child), coring(_coring), sse(_sse)
{
  try {  // HIDE DAMN SEH COMPILER BUG!!!
    if (vi.IsRGB())
          env->ThrowError("Tweak: YUV data only (no RGB)");

    // Flag to skip special processing if doing all pixels
    // If defaults, don't check for ranges, just do all
    const bool allPixels = (startHue == 0.0 && endHue == 360.0 && _maxSat == 150.0 && _minSat == 0.0);

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
// The new "mapping" C code is faster than the iSSE code on my 3GHz P4HT - Make it optional
    if (sse && (!allPixels || coring || !vi.IsYUY2()))
        env->ThrowError("Tweak: SSE option only available for YUY2 with coring=false and no options.");
    if (sse && !(env->GetCPUFlags() & CPUF_INTEGER_SSE))
        env->ThrowError("Tweak: SSE option needs an iSSE capable processor");
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE

    if (startHue < 0.0 || startHue >= 360.0)
          env->ThrowError("Tweak: startHue must be greater than or equal to 0.0 and less than 360.0");

    if (endHue <= 0.0 || endHue > 360.0)
          env->ThrowError("Tweak: endHue must be greater than 0.0 and less than or equal to 360.0");

    if (_minSat >= _maxSat)
          env->ThrowError("Tweak: MinSat must be less than MaxSat");

    if (_minSat < 0.0 || _minSat >= 150.0)
          env->ThrowError("Tweak: minSat must be greater than or equal to 0 and less than 150.");

    if (_maxSat <= 0.0 || _maxSat > 150.0)
          env->ThrowError("Tweak: maxSat must be greater than 0 and less than or equal to 150.");

    if (p>=32.0 || p<0.0)
          env->ThrowError("Tweak: P must be greater than or equal to 0 and less than 32.");

    Sat = (int) (_sat * 512);
    Cont = (int) (_cont * 512);
    Bright = (int) _bright;

    const double Hue = (_hue * 3.14159265358979323846) / 180.0;
    const double SIN = sin(Hue);
    const double COS = cos(Hue);

    Sin = (int) (SIN * 4096 + 0.5);
    Cos = (int) (COS * 4096 + 0.5);

    const int maxY = coring ? 235 : 255;
    const int minY = coring ? 16 : 0;

    const int maxUV = coring ? 240 : 255;
    const int minUV = coring ? 16 : 0;

    for (int i = 0; i < 256; i++)
    {
        /* brightness and contrast */
        int y = int((i - 16)*_cont + _bright + 16.5);
        map[i] = std::min(std::max(y,minY),maxY);
    }

    // 100% equals sat=119 (= maximal saturation of valid RGB (R=255,G=B=0)
    // 150% (=180) - 100% (=119) overshoot
    const double minSat = 1.19 * _minSat;
    const double maxSat = 1.19 * _maxSat;

    for (int u = 0; u < 256; u++) {
      const int destu = u-128;
      for (int v = 0; v < 256; v++) {
        const int destv = v-128;
        int iSat = Sat;
        if (allPixels || ProcessPixel(destv, destu, startHue, endHue, maxSat, minSat, p, iSat)) {
          int du = int ( (destu*COS + destv*SIN) * iSat ) >> 9;
          int dv = int ( (destv*COS - destu*SIN) * iSat ) >> 9;
          du = std::min(std::max(du+128,minUV),maxUV);
          dv = std::min(std::max(dv+128,minUV),maxUV);
          mapUV[(u<<8)|v]  = (unsigned short)(du | (dv<<8));
        } else {
          mapUV[(u<<8)|v]  = (unsigned short)(std::min(std::max(u,minUV),maxUV) | ((std::min(std::max(v,minUV),maxUV))<<8));
        }
      }
    }

  }
  catch (...) { throw; }
}



PVideoFrame __stdcall Tweak::GetFrame(int n, IScriptEnvironment* env)
{
	PVideoFrame src = child->GetFrame(n, env);
	env->MakeWritable(&src);

	BYTE* srcp = src->GetWritePtr();

	int src_pitch = src->GetPitch();
	int height = src->GetHeight();
	int row_size = src->GetRowSize();

	if (vi.IsYUY2()) {
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
		if (sse && !coring && (env->GetCPUFlags() & CPUF_INTEGER_SSE)) {
			const __int64 hue64 = (in64 Cos<<48) + (in64 (-Sin)<<32) + (in64 Sin<<16) + in64 Cos;
			const __int64 satcont64 = (in64 Sat<<48) + (in64 Cont<<32) + (in64 Sat<<16) + in64 Cont;
			const __int64 bright64 = (in64 Bright<<32) + in64 Bright;

			asm_tweak_ISSE_YUY2(srcp, row_size>>2, height, src_pitch-row_size, hue64, satcont64, bright64);   
			return src;
		}
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
		

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < row_size; x+=4)
			{
				/* brightness and contrast */
				srcp[x] = map[srcp[x]];
				srcp[x+2] = map[srcp[x+2]];

				/* hue and saturation */
				const int u = srcp[x+1];
				const int v = srcp[x+3];
				const int mapped = mapUV[(u<<8) | v];
				srcp[x+1] = (BYTE)(mapped&0xff);
				srcp[x+3] = (BYTE)(mapped>>8);
			}
			srcp += src_pitch;
		}
	} else if (vi.IsPlanar()) {
		int y;  // VC6 scoping sucks - Yes!
		for (y=0; y<height; ++y) {
			for (int x=0; x<row_size; ++x) {
				/* brightness and contrast */
				srcp[x] = map[srcp[x]];
			}
			srcp += src_pitch;
		}

		src_pitch = src->GetPitch(PLANAR_U);
		BYTE * srcpu = src->GetWritePtr(PLANAR_U);
		BYTE * srcpv = src->GetWritePtr(PLANAR_V);
		row_size = src->GetRowSize(PLANAR_U);
		height = src->GetHeight(PLANAR_U);
		for (y=0; y<height; ++y) {
			for (int x=0; x<row_size; ++x) {
				/* hue and saturation */
				const int u = srcpu[x];
				const int v = srcpv[x];
				const int mapped = mapUV[(u<<8) | v];
				srcpu[x] = (BYTE)(mapped&0xff);
				srcpv[x] = (BYTE)(mapped>>8);
			}
			srcpu += src_pitch;
			srcpv += src_pitch;
		}
	}

	return src;
}

AVSValue __cdecl Tweak::Create(AVSValue args, void* user_data, IScriptEnvironment* env)
{
	try {	// HIDE DAMN SEH COMPILER BUG!!!
    return new Tweak(args[0].AsClip(),
					 args[1].AsFloat(0.0),		// hue
					 args[2].AsFloat(1.0),		// sat
					 args[3].AsFloat(0.0),		// bright
					 args[4].AsFloat(1.0),		// cont
					 args[5].AsBool(true),      // coring
					 args[6].AsBool(false),     // sse
					 args[7].AsFloat(0.0),      // startHue
					 args[8].AsFloat(360.0),    // endHue
					 args[9].AsFloat(150.0),    // maxSat
					 args[10].AsFloat(0.0),     // minSat
					 args[11].AsFloat(16.0),	// interp
					 env);
	}
	catch (...) { throw; }
}


#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
// Integer SSE optimization by "Dividee".
void __declspec(naked) asm_tweak_ISSE_YUY2( BYTE *srcp, int w, int h, int modulo, __int64 hue, 
                                       __int64 satcont, __int64 bright ) 
{
	static const __int64 norm = 0x0080001000800010i64;

	__asm {
		push		ebp
		push		edi
		push		esi
		push		ebx

		pxor		mm0, mm0
		movq		mm1, norm				// 128 16 128 16
		movq		mm2, [esp+16+20]		// Cos -Sin Sin Cos (fix12)
		movq		mm3, [esp+16+28]		// Sat Cont Sat Cont (fix9)
		movq		mm4, mm1
		paddw		mm4, [esp+16+36]		// 128 16+Bright 128 16+Bright

		mov			esi, [esp+16+4]			// srcp
		mov			edx, [esp+16+12]		// height
y_loop:
		mov			ecx, [esp+16+8]			// width
x_loop:
		movd		mm7, [esi]   			// 0000VYUY
		punpcklbw	mm7, mm0
		psubw		mm7, mm1				//  V Y U Y
		pshufw		mm6, mm7, 0xDD			//  V U V U
		pmaddwd		mm6, mm2				// V*Cos-U*Sin V*Sin+U*Cos (fix12)
		psrad		mm6, 12					// ? V' ? U'
		movq		mm5, mm7
		punpcklwd	mm7, mm6				// ? ? U' Y
		punpckhwd	mm5, mm6				// ? ? V' Y
		punpckldq	mm7, mm5				// V' Y U' Y
		psllw		mm7, 7					// (fix7)
		pmulhw		mm7, mm3	            // V'*Sat Y*Cont U'*Sat Y*Cont
		paddw		mm7, mm4				// V" Y" U" Y"
		packuswb	mm7, mm0				// 0000V"Y"U"Y"
		movd		[esi], mm7

		add			esi, 4
		dec			ecx
		jnz			x_loop

		add			esi, [esp+16+16]		// skip to next scanline
		dec			edx
		jnz			y_loop

		pop			ebx
		pop			esi
		pop			edi
		pop			ebp
		emms
		ret
	};
}
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE



Limiter::Limiter(PClip _child, int _min_luma, int _max_luma, int _min_chroma, int _max_chroma, int _show, IScriptEnvironment* env)
    : GenericVideoFilter(_child),
  max_luma(_max_luma),    
  min_luma(_min_luma),
  max_chroma(_max_chroma),  
  min_chroma(_min_chroma),
  show((enum SHOW)_show) 
  {
      
  if (!vi.IsYUV())
      env->ThrowError("Limiter: Source must be YUV");

  if ((min_luma<0)||(min_luma>255))
      env->ThrowError("Limiter: Invalid minimum luma");
  if ((max_luma<0)||(max_luma>255))
      env->ThrowError("Limiter: Invalid maximum luma");
  if ((min_chroma<0)||(min_chroma>255))
      env->ThrowError("Limiter: Invalid minimum chroma");
  if ((max_chroma<0)||(max_chroma>255))
      env->ThrowError("Limiter: Invalid maximum chroma");

  luma_emulator=false;
  chroma_emulator=false;

}

PVideoFrame __stdcall Limiter::GetFrame(int n, IScriptEnvironment* env) {
  PVideoFrame frame = child->GetFrame(n, env);
  env->MakeWritable(&frame);
  unsigned char* srcp = frame->GetWritePtr();
  int pitch = frame->GetPitch();
  int row_size = frame->GetRowSize();
  int height = frame->GetHeight();

  if (vi.IsYUY2()) {

		if (show == show_luma) {  // Mark clamped pixels red/yellow/green over a colour image
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < row_size; x+=4) {
					int uv = 0;
					if      (srcp[x  ] < min_luma) { srcp[x  ] =  81; uv |= 1;}
					else if (srcp[x  ] > max_luma) { srcp[x  ] = 145; uv |= 2;}
					if      (srcp[x+2] < min_luma) { srcp[x+2] =  81; uv |= 1;}
					else if (srcp[x+2] > max_luma) { srcp[x+2] = 145; uv |= 2;}
					switch (uv) {
						case 1: srcp[x+1] = 91; srcp[x+3] = 240; break;     // red:   Y= 81, U=91 and V=240 
						case 2: srcp[x+1] = 54; srcp[x+3] =  34; break;     // green: Y=145, U=54 and V=34 
						case 3: srcp[x  ] =     srcp[x+2] = 210;
						        srcp[x+1] = 16; srcp[x+3] = 146; break;     // yellow:Y=210, U=16 and V=146
						default: break;
					}
				}
				srcp += pitch;
			}
			return frame;
		}
		else if (show == show_luma_grey) {    // Mark clamped pixels coloured over a greyscaled image
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < row_size; x+=4) {
					int uv = 0;
					if      (srcp[x  ] < min_luma) { srcp[x  ] =  81; uv |= 1;}
					else if (srcp[x  ] > max_luma) { srcp[x  ] = 145; uv |= 2;}
					if      (srcp[x+2] < min_luma) { srcp[x+2] =  81; uv |= 1;}
					else if (srcp[x+2] > max_luma) { srcp[x+2] = 145; uv |= 2;}
					switch (uv) {
						case 1: srcp[x+1] = 91; srcp[x+3] = 240; break;     // red:   Y=81, U=91 and V=240 
						case 2: srcp[x+1] = 54; srcp[x+3] =  34; break;     // green: Y=145, U=54 and V=34 
						case 3: srcp[x+1] = 90; srcp[x+3] = 134; break;     // puke:  Y=81, U=90 and V=134
						default: srcp[x+1] = srcp[x+3] = 128; break;        // olive: Y=145, U=90 and V=134
					}
				}
				srcp += pitch;
			}
			return frame;
		}
		else if (show == show_chroma) {    // Mark clamped pixels yellow over a colour image
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < row_size; x+=4) {
					if ( (srcp[x+1] < min_chroma)  // U-
					  || (srcp[x+1] > max_chroma)  // U+
					  || (srcp[x+3] < min_chroma)  // V-
					  || (srcp[x+3] > max_chroma) )// V+
					 { srcp[x]=srcp[x+2]=210; srcp[x+1]=16; srcp[x+3]=146; }    // yellow:Y=210, U=16 and V=146
				}
				srcp += pitch;
			}
			return frame;
		}
		else if (show == show_chroma_grey) {    // Mark clamped pixels coloured over a greyscaled image
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < row_size; x+=4) {
					int uv = 0;
					if      (srcp[x+1] < min_chroma) uv |= 1; // U-
					else if (srcp[x+1] > max_chroma) uv |= 2; // U+
					if      (srcp[x+3] < min_chroma) uv |= 4; // V-
					else if (srcp[x+3] > max_chroma) uv |= 8; // V+
					switch (uv) {
						case  8: srcp[x] = srcp[x+2] =  81; srcp[x+1] =  91; srcp[x+3] = 240; break;    //   +V Red
						case  9: srcp[x] = srcp[x+2] = 146; srcp[x+1] =  53; srcp[x+3] = 193; break;    // -U+V Orange
						case  1: srcp[x] = srcp[x+2] = 210; srcp[x+1] =  16; srcp[x+3] = 146; break;    // -U   Yellow
						case  5: srcp[x] = srcp[x+2] = 153; srcp[x+1] =  49; srcp[x+3] =  49; break;    // -U-V Green
						case  4: srcp[x] = srcp[x+2] = 170; srcp[x+1] = 165; srcp[x+3] =  16; break;    //   -V Cyan
						case  6: srcp[x] = srcp[x+2] = 105; srcp[x+1] = 203; srcp[x+3] =  63; break;    // +U-V Teal
						case  2: srcp[x] = srcp[x+2] =  41; srcp[x+1] = 240; srcp[x+3] = 110; break;    // +U   Blue
						case 10: srcp[x] = srcp[x+2] = 106; srcp[x+1] = 202; srcp[x+3] = 222; break;    // +U+V Magenta
						default: srcp[x+1] = srcp[x+3] = 128; break;
					}
				}
				srcp += pitch;
			}
			return frame;
		}

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
    /** Run emulator if CPU supports it**/
    if (env->GetCPUFlags() & CPUF_INTEGER_SSE) {
      c_plane = srcp;
      if (!luma_emulator) {  
        assemblerY = create_emulator(row_size, height, env);
        luma_emulator=true;
      }
      emu_cmin =  min_luma|(min_chroma<<8);
      emu_cmax =  max_luma|(max_chroma<<8);
      modulo = pitch-row_size;
      assemblerY.Call();
      return frame;
    } 
    else 
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE        
    {  // If not ISSE
      for(int y = 0; y < height; y++) {
        for(int x = 0; x < row_size; x++) {
          if(srcp[x] < min_luma )
            srcp[x++] = min_luma;
          else if(srcp[x] > max_luma)
            srcp[x++] = max_luma;
          else
            x++;
          if(srcp[x] < min_chroma)
            srcp[x] = min_chroma;
          else if(srcp[x] > max_chroma)
            srcp[x] = max_chroma;
        }
        srcp += pitch;
      }
      return frame;
    }  
  } else if(vi.IsYV12()) {
		if (show == show_luma) {    // Mark clamped pixels red/yellow/green over a colour image
			const int pitchUV = frame->GetPitch(PLANAR_U);
			unsigned char* srcn = srcp + pitch;
			unsigned char* srcpV = frame->GetWritePtr(PLANAR_V);
			unsigned char* srcpU = frame->GetWritePtr(PLANAR_U);

			for (int h=0; h < height;h+=2) {
				for (int x = 0; x < row_size; x+=2) {
					int uv = 0;
					if      (srcp[x  ] < min_luma) { srcp[x  ] =  81; uv |= 1;}
					else if (srcp[x  ] > max_luma) { srcp[x  ] = 145; uv |= 2;}
					if      (srcp[x+1] < min_luma) { srcp[x+1] =  81; uv |= 1;}
					else if (srcp[x+1] > max_luma) { srcp[x+1] = 145; uv |= 2;}
					if      (srcn[x  ] < min_luma) { srcn[x  ] =  81; uv |= 1;}
					else if (srcn[x  ] > max_luma) { srcn[x  ] = 145; uv |= 2;}
					if      (srcn[x+1] < min_luma) { srcn[x+1] =  81; uv |= 1;}
					else if (srcn[x+1] > max_luma) { srcn[x+1] = 145; uv |= 2;}
					switch (uv) {
						case 1: srcpU[x/2] = 91; srcpV[x/2] = 240; break;       // red:   Y=81, U=91 and V=240 
						case 2: srcpU[x/2] = 54; srcpV[x/2] =  34; break;       // green: Y=145, U=54 and V=34 
						case 3: srcp[x]=srcp[x+2]=srcn[x]=srcn[x+2]=210;
						        srcpU[x/2] = 16; srcpV[x/2] = 146; break;       // yellow:Y=210, U=16 and V=146
						default: break;
					}
				}
				srcp += pitch*2;
				srcn += pitch*2;
				srcpV += pitchUV;
				srcpU += pitchUV;
			}
			return frame;
		}
		else if (show == show_luma_grey) {       // Mark clamped pixels coloured over a greyscaled image
			const int pitchUV = frame->GetPitch(PLANAR_U);
			unsigned char* srcn = srcp + pitch;
			unsigned char* srcpV = frame->GetWritePtr(PLANAR_V);
			unsigned char* srcpU = frame->GetWritePtr(PLANAR_U);

			for (int h=0; h < height;h+=2) {
				for (int x = 0; x < row_size; x+=2) {
					int uv = 0;
					if      (srcp[x  ] < min_luma) { srcp[x  ] =  81; uv |= 1;}
					else if (srcp[x  ] > max_luma) { srcp[x  ] = 145; uv |= 2;}
					if      (srcp[x+1] < min_luma) { srcp[x+1] =  81; uv |= 1;}
					else if (srcp[x+1] > max_luma) { srcp[x+1] = 145; uv |= 2;}
					if      (srcn[x  ] < min_luma) { srcn[x  ] =  81; uv |= 1;}
					else if (srcn[x  ] > max_luma) { srcn[x  ] = 145; uv |= 2;}
					if      (srcn[x+1] < min_luma) { srcn[x+1] =  81; uv |= 1;}
					else if (srcn[x+1] > max_luma) { srcn[x+1] = 145; uv |= 2;}
					switch (uv) {
						case 1: srcpU[x/2] = 91; srcpV[x/2] = 240; break;       // red:   Y=81, U=91 and V=240 
						case 2: srcpU[x/2] = 54; srcpV[x/2] =  34; break;       // green: Y=145, U=54 and V=34 
						case 3: srcpU[x/2] = 90; srcpV[x/2] = 134; break;       // puke:  Y=81, U=90 and V=134
						default: srcpU[x/2] = srcpV[x/2] = 128; break;          // olive: Y=145, U=90 and V=134
					}
				}
				srcp += pitch*2;
				srcn += pitch*2;
				srcpV += pitchUV;
				srcpU += pitchUV;
			}
			return frame;
		}
		else if (show == show_chroma) {   // Mark clamped pixels yellow over a colour image
			const int pitchUV = frame->GetPitch(PLANAR_U);
			unsigned char* srcn = srcp + pitch;
			unsigned char* srcpV = frame->GetWritePtr(PLANAR_V);
			unsigned char* srcpU = frame->GetWritePtr(PLANAR_U);

			for (int h=0; h < height;h+=2) {
				for (int x = 0; x < row_size; x+=2) {
					if ( (srcpU[x/2] < min_chroma)  // U-
					  || (srcpU[x/2] > max_chroma)  // U+
					  || (srcpV[x/2] < min_chroma)  // V-
					  || (srcpV[x/2] > max_chroma) )// V+
					{ srcp[x]=srcp[x+1]=srcn[x]=srcn[x+1]=210; srcpU[x/2]= 16; srcpV[x/2]=146; }   // yellow:Y=210, U=16 and V=146
				}
				srcp += pitch*2;
				srcn += pitch*2;
				srcpV += pitchUV;
				srcpU += pitchUV;
			}
			return frame;
		}
		else if (show == show_chroma_grey) {   // Mark clamped pixels coloured over a greyscaled image
			const int pitchUV = frame->GetPitch(PLANAR_U);
			unsigned char* srcn = srcp + pitch;
			unsigned char* srcpV = frame->GetWritePtr(PLANAR_V);
			unsigned char* srcpU = frame->GetWritePtr(PLANAR_U);

			for (int h=0; h < height;h+=2) {
				for (int x = 0; x < row_size; x+=2) {
					int uv = 0;
					if      (srcpU[x/2] < min_chroma) uv |= 1; // U-
					else if (srcpU[x/2] > max_chroma) uv |= 2; // U+
					if      (srcpV[x/2] < min_chroma) uv |= 4; // V-
					else if (srcpV[x/2] > max_chroma) uv |= 8; // V+
					switch (uv) {
						case  8: srcp[x]=srcp[x+1]=srcn[x]=srcn[x+1]= 81; srcpU[x/2]= 91; srcpV[x/2]=240; break;   //   +V Red
						case  9: srcp[x]=srcp[x+1]=srcn[x]=srcn[x+1]=146; srcpU[x/2]= 53; srcpV[x/2]=193; break;   // -U+V Orange
						case  1: srcp[x]=srcp[x+1]=srcn[x]=srcn[x+1]=210; srcpU[x/2]= 16; srcpV[x/2]=146; break;   // -U   Yellow
						case  5: srcp[x]=srcp[x+1]=srcn[x]=srcn[x+1]=153; srcpU[x/2]= 49; srcpV[x/2]= 49; break;   // -U-V Green
						case  4: srcp[x]=srcp[x+1]=srcn[x]=srcn[x+1]=170; srcpU[x/2]=165; srcpV[x/2]= 16; break;   //   -V Cyan
						case  6: srcp[x]=srcp[x+1]=srcn[x]=srcn[x+1]=105; srcpU[x/2]=203; srcpV[x/2]= 63; break;   // +U-V Teal
						case  2: srcp[x]=srcp[x+1]=srcn[x]=srcn[x+1]= 41; srcpU[x/2]=240; srcpV[x/2]=110; break;   // +U   Blue
						case 10: srcp[x]=srcp[x+1]=srcn[x]=srcn[x+1]=106; srcpU[x/2]=202; srcpV[x/2]=222; break;   // +U+V Magenta
						default: srcpU[x/2] = srcpV[x/2] = 128; break;
					}
				}
				srcp += pitch*2;
				srcn += pitch*2;
				srcpV += pitchUV;
				srcpU += pitchUV;
			}
			return frame;
		}
  }
  if (vi.IsPlanar()) {
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
    if (env->GetCPUFlags() & CPUF_INTEGER_SSE) {
      /** Run emulator if CPU supports it**/
      row_size= frame->GetRowSize(PLANAR_Y_ALIGNED);
      if (!luma_emulator) {
        assemblerY = create_emulator(row_size, height, env);
        luma_emulator=true;
      }
      emu_cmin = min_luma|(min_luma<<8);
      emu_cmax = max_luma|(max_luma<<8);
      modulo = pitch-row_size;

      c_plane = (BYTE*)srcp;
      assemblerY.Call();

      // Prepare for chroma
      row_size = frame->GetRowSize(PLANAR_U_ALIGNED);
      pitch = frame->GetPitch(PLANAR_U);
      if (!pitch)
        return frame;

      if (!chroma_emulator) {
        height = frame->GetHeight(PLANAR_U);
        assemblerUV = create_emulator(row_size, height, env);
        chroma_emulator=true;
      }
      emu_cmin = min_chroma|(min_chroma<<8);
      emu_cmax = max_chroma|(max_chroma<<8);
      modulo = pitch-row_size;

      c_plane = frame->GetWritePtr(PLANAR_U);
      assemblerUV.Call();

      c_plane = frame->GetWritePtr(PLANAR_V);
      assemblerUV.Call();

      return frame;
    }
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE

    for(int y = 0; y < height; y++) {
      for(int x = 0; x < row_size; x++) {
        if(srcp[x] < min_luma )
          srcp[x] = min_luma;
        else if(srcp[x] > max_luma)
          srcp[x] = max_luma;        
      }
      srcp += pitch;
    }
    // Prepare for chroma
    srcp = frame->GetWritePtr(PLANAR_U);
    unsigned char* srcpV = frame->GetWritePtr(PLANAR_V);
    row_size = frame->GetRowSize(PLANAR_U);
    height = frame->GetHeight(PLANAR_U);
    pitch = frame->GetPitch(PLANAR_U);
    if (!pitch)
      return frame;

    for(int y = 0; y < height; y++) {
      for(int x = 0; x < row_size; x++) {
        if(srcp[x] < min_chroma)
          srcp[x] = min_chroma;
        else if(srcp[x] > max_chroma)
          srcp[x] = max_chroma;
        if(srcpV[x] < min_chroma)
          srcpV[x] = min_chroma;
        else if(srcpV[x] > max_chroma)
          srcpV[x] = max_chroma;
      }
      srcp += pitch;
      srcpV += pitch;
    }
  }
  return frame;
}

Limiter::~Limiter() {
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
  if (luma_emulator) assemblerY.Free();
  if (chroma_emulator) assemblerUV.Free();
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE  
}


#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
/***
 * Dynamicly assembled code - runs at approx 14000 fps at 480x480 on a 1.8Ghz Athlon. 5-6x faster than C code
 ***/

DynamicAssembledCode Limiter::create_emulator(int row_size, int height, IScriptEnvironment* env) {

  int mod32_w = row_size/32;
  int remain_4 = (row_size-(mod32_w*32))/4;

  int prefetchevery = 1;  // 32 byte cache line

  if ((env->GetCPUFlags() & CPUF_3DNOW_EXT)||((env->GetCPUFlags() & CPUF_SSE2))) {
    // We have either an Athlon or a P4
    prefetchevery = 2;  // 64 byte cacheline
  }

  bool use_movntq = true;  // We cannot enable write combining as we are only writing 32 bytes between reads. Softwire also crashes here!!!
  bool hard_prefetch = false;   // Do we prefetch ALL data before any processing takes place?

  if (env->GetCPUFlags() & CPUF_3DNOW_EXT) {
    hard_prefetch = true;   // We have AMD - so we enable hard prefetching.
  }


  Assembler x86;   // This is the class that assembles the code.

  if (env->GetCPUFlags() & CPUF_INTEGER_SSE) {
    x86.push(eax);
    x86.push(ebx);
    x86.push(ecx);
    x86.push(edx);
    x86.push(esi);
    x86.push(edi);


    x86.mov(eax, height);
    x86.mov(ebx, dword_ptr [&c_plane]);  // Pointer to the current plane
    x86.mov(ecx, dword_ptr [&modulo]);   // Modulo
    x86.movd(mm7,dword_ptr [&emu_cmax]);  
    x86.movd(mm6, dword_ptr [&emu_cmin]);
    x86.pshufw(mm7,mm7,0);  // Move thresholds into all 8 bytes
    x86.pshufw(mm6,mm6,0);

    x86.align(16);
    x86.label("yloop");
    if (hard_prefetch) {
      for (int i=0;i<=mod32_w;i+=2) {
        x86.mov(edx, dword_ptr [ebx + (i*32)]);
      }
    }
    for (int i=0;i<mod32_w;i++) {
      // This loop processes 32 bytes at the time.
      // All remaining pixels are handled by the next loop.
      if ((!(i%prefetchevery)) && (!hard_prefetch)) {
         //Prefetch only once per cache line
       x86.prefetchnta(dword_ptr [ebx+256]);
      }
      x86.movq(mm0, qword_ptr[ebx]);
      x86.movq(mm1, qword_ptr[ebx+8]);
      x86.movq(mm2, qword_ptr[ebx+16]);
      x86.movq(mm3, qword_ptr[ebx+24]);
      x86.pminub(mm0,mm7);
      x86.pminub(mm1,mm7);
      x86.pminub(mm2,mm7);
      x86.pminub(mm3,mm7);
      x86.pmaxub(mm0,mm6);
      x86.pmaxub(mm1,mm6);
      x86.pmaxub(mm2,mm6);
      x86.pmaxub(mm3,mm6);
      if (!use_movntq) {
        x86.movq(qword_ptr[ebx],mm0);
        x86.movq(qword_ptr[ebx+8],mm1);
        x86.movq(qword_ptr[ebx+16],mm2);
        x86.movq(qword_ptr[ebx+24],mm3);
      } else {
        x86.movntq(qword_ptr [ebx],mm0);
        x86.movntq(qword_ptr [ebx+8],mm1);
        x86.movntq(qword_ptr [ebx+16],mm2);
        x86.movntq(qword_ptr [ebx+24],mm3);
      }
      x86.add(ebx,32);
    }
    for (int i=0;i<remain_4;i++) {
      // Here we process any pixels not being within mod32.
      x86.movd(mm0,dword_ptr [ebx]);
      x86.pminub(mm0,mm7);
      x86.pmaxub(mm0,mm6);      
      x86.movd(dword_ptr [ebx],mm0);
      x86.add(ebx,4);
    }
    x86.add(ebx,ecx);
    x86.dec(eax);
    x86.jnz("yloop");
    if (use_movntq) {
      x86.sfence();  // Flush write combiner.
    }
    x86.emms();

    x86.pop(edi);
    x86.pop(esi);
    x86.pop(edx);
    x86.pop(ecx);
    x86.pop(ebx);
    x86.pop(eax);

    x86.ret();
  }
  return DynamicAssembledCode(x86, env, "Limiter: ISSE code could not be compiled.");
}
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE


AVSValue __cdecl Limiter::Create(AVSValue args, void* user_data, IScriptEnvironment* env)
{
	const char* option = args[5].AsString(0);
	enum SHOW show = show_none;

	if (option) {
	  if      (lstrcmpi(option, "luma") == 0) 
			show = show_luma;
	  else if (lstrcmpi(option, "luma_grey") == 0) 
			show = show_luma_grey;
	  else if (lstrcmpi(option, "chroma") == 0) 
			show = show_chroma;
	  else if (lstrcmpi(option, "chroma_grey") == 0) 
			show = show_chroma_grey;
	  else
			env->ThrowError("Limiter: show must be \"luma\", \"luma_grey\", \"chroma\" or \"chroma_grey\"");
	}

	return new Limiter(args[0].AsClip(), args[1].AsInt(16), args[2].AsInt(235), args[3].AsInt(16), args[4].AsInt(240), show, env);
}

}; // namespace avxsynth
