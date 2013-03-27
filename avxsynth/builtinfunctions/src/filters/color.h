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


#ifndef __Color_h
#define __Color_h

#include "internal.h"
#include "avxlog.h"

namespace avxsynth {

#ifdef MODULE_NAME
#undef MODULE_NAME
#endif

#define MODULE_NAME builtinfuncs::color

#ifdef _DEBUG
static int DebugMsg(LPSTR sz,...)
{
    char buf[256];
    wvsprintf (buf, sz, (LPSTR)(&sz+1));
    AVXLOG_INFO("%s", buf);
    return FALSE;
}

typedef struct _COUNT {
	int d;
	int	over;
	int	under;
	int max;
	int min;
	int ave;
} COUNT;
#endif

typedef union {
	DWORD	data;
	struct {
		BYTE	y0;
		BYTE	u;
		BYTE	y1;
		BYTE	v;
	} yuv;
} PIXELDATA;

class Color : public GenericVideoFilter
{
	double y_contrast, y_bright, y_gamma, y_gain;
	double u_contrast, u_bright, u_gamma, u_gain;
	double v_contrast, v_bright, v_gamma, v_gain;
	int matrix, levels, opt;
	bool colorbars, analyze, autowhite, autogain;
	unsigned char LUT_Y[256],LUT_U[256],LUT_V[256];
	int           y_thresh1, y_thresh2, u_thresh1, u_thresh2, v_thresh1, v_thresh2;
public:
	Color(PClip _child, double _gain_y, double _off_y, double _gamma_y, double _cont_y,
							double _gain_u, double _off_u, double _gamma_u, double _cont_u,
							double _gain_v, double _off_v, double _gamma_v, double _cont_v,
							const char *_levels, const char *_opt, const char *_matrix, bool _colorbars,
							bool _analyze, bool _autowhite, bool _autogain,
							IScriptEnvironment* env);
    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
    static AVSValue __cdecl Create(AVSValue args, void* user_data, IScriptEnvironment* env);
private:
	bool CheckParms(const char *_levels, const char *_matrix, const char *_opt);
	void MakeGammaLUT(void);
#ifdef _DEBUG
	void YUV2RGB(int y, int u, int v, int *r, int *g, int *b, int matrix);
	void CheckRGB(COUNT *r, COUNT *g, COUNT *b);
	void CheckYUV(PIXELDATA *pixel0, PIXELDATA *pixel, COUNT *y, COUNT *u, COUNT *v, int terget);
	void DumpLUT(void);
#endif

};

}; // namespace avxsynth
#endif // __Color_h
