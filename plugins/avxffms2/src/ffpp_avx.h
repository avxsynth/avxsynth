//  Copyright (c) 2007-2011 Fredrik Mellbin
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#ifndef FFPP_H
#define FFPP_H

#ifndef FFMS_USE_POSTPROC
#define FFMS_USE_POSTPROC   
#endif

#ifdef FFMS_USE_POSTPROC

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libpostproc/postprocess.h>
}

#include <ffmscompat.h>

#include "avxplugin.h"

class FFPP : public avxsynth::GenericVideoFilter {
private:
	pp_context *PPContext;
	pp_mode *PPMode;
	SwsContext *SWSTo422P;
	SwsContext *SWSFrom422P;
	AVPicture InputPicture;
	AVPicture OutputPicture;
public:
	FFPP(avxsynth::PClip AChild, const char *PP, avxsynth::IScriptEnvironment *Env);
	~FFPP();
	avxsynth::PVideoFrame __stdcall GetFrame(int n, avxsynth::IScriptEnvironment* Env);
};

#endif // FFMS_USE_POSTPROC

#endif // FFPP_H
