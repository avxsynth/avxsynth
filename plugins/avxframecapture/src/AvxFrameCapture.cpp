// Copyright (c) 2012 David Ronca.  All rights reserved.
// videophool@hotmail.com
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
// import and export plugins, or graphical user interfaces.auto

/****************************************************************************************************/
//  AvxFrameCapture:  This filter is used to capture video frames to JPeg files.
//  Useage:
//      outputPath_name:    The path, and partial filename for the captured images.  /home/encodingtools/Pictures/Capture_ would generate 
//                          the file /home/encodingtools/Pictures/Capture_Frame_<frameNum>_Time_<Frame time>.jpg
//  interval_ms: The interval (in milliseconds, between each frame to capture (i.e. 10010 woudl capture frames every 10 seconds.
//  scale:  the scale in integer percent (i.e. 10-100)

#include "AvxFrameCapture.h"
#include "AvxException.h"
#include "avxlog.h"

#ifdef MODULE_NAME
#undef MODULE_NAME
#endif

#define MODULE_NAME AvxFrameCapture


AvxFrameCapture::AvxFrameCapture(PClip _child, PClipProxy _upstreamProxy, PClip _convert, FrameCaptureProfile const& _profile) 
    : GenericVideoFilter(_child), upstreamProxy(_upstreamProxy), convert(_convert), profile(_profile), lastCaptureTime(0), totalFramesCaptured(0)
{
    
}

AvxFrameCapture::~AvxFrameCapture()
{
    this->upstreamProxy.Clear();
}

Utf8String AvxFrameCapture::GetOutputPath(long frameNum, __int64 frameTime)
{
    Utf8String ret;

    long totalMilliseconds = (long)(frameTime / 10000);
    long totalSeconds = totalMilliseconds / 1000;
    long hours = totalSeconds / 3600;
    long minutes = (totalSeconds % 3600) / 60;
    long seconds = (totalSeconds % 3600) % 60;
    long milliseconds = totalMilliseconds % 1000;

    ret.Format
    (
        "%s_Frame_%08d_Time_%02d_%02d_%02d_%03d.jpg", 
        this->profile.GetOutputPath().c_str(), 
        frameNum,
        hours, minutes, seconds, milliseconds
    );

    return ret;
}


PVideoFrame __stdcall AvxFrameCapture::GetFrame(int n, IScriptEnvironment* env) 
{
    PVideoFrame frame = child->GetFrame(n, env);
    
    try
    {
        int ms = (int)(((__int64)n * vi.fps_denominator * 1000 / vi.fps_numerator)%1000);
        int sec = (int)((__int64)n * vi.fps_denominator / vi.fps_numerator);
        int min = sec/60;
        int hour = sec/3600;
        
        __int64 frameTime = ((__int64)n * vi.fps_denominator * 1000 / vi.fps_numerator) * 10000;        
        __int64 nextCaptureTime = this->lastCaptureTime + this->profile.GetTimeInterval();
        
        if(frameTime >= nextCaptureTime)
        {
            
            AVXLOG_INFO("Capture frame %d at time: %ld", n, frameTime / 10000);
            
            this->upstreamProxy.GetClipProxy()->SetCache(n, frame);
            PVideoFrame cvt = this->convert->GetFrame(n, env);
            
            Utf8String outputPath = GetOutputPath(n, frameTime);
            this->renderer.RenderFrame(cvt, Size(this->convert->GetVideoInfo().width, this->convert->GetVideoInfo().height), outputPath);   
            this->lastCaptureTime = frameTime;
            ++this->totalFramesCaptured;
        }
    }
    catch(AvxException &e)
    {
        env->ThrowError(Utf8String(e.GetMsg()).c_str());
    }

    return frame;
}

AVSValue __cdecl AvxFrameCapture::Create(AVSValue args, void*, IScriptEnvironment* env) 
{
    PClip upstream = args[0].AsClip();
    PClipProxy upstreamProxy(ClipProxy::Create(upstream));
    
    PClip convert = upstreamProxy;
    const char* outputPath  = args[1].AsString();
    
    VideoInfo vi = upstream->GetVideoInfo();
    
    if(!vi.IsRGB() && !vi.IsYUY2() && !vi.IsYV12())
        env->ThrowError("AvxSubtitle: Only RGB24, RGB32, YUY2 and YV12 colorspaces are supported");
    
    if(!vi.IsRGB24())
    { // need to convert
         convert = env->Invoke("ConvertToRGB24", AVSValue(upstream)).AsClip();
    }

    AVSValue const& val = args[3];
    float scale = min(1.0, max(0.1, min(args[3].AsInt(100), 100) / 100.0));
    
    if(scale < 1)
    { // need to add the scale
        int scaledX = vi.width * scale;
        int scaledY = vi.height * scale;
        
        AVSValue scaleArgs[] = {convert, scaledX, scaledY};
        
        convert = env->Invoke("LanczosResize", AVSValue(scaleArgs, 3)).AsClip();
    }
    
    int interval = args[2].AsInt(10010);

    PClip ret = new AvxFrameCapture(upstream, upstreamProxy, convert, FrameCaptureProfile(outputPath, interval * 10000));
    AVXLOG_INFO("Frame capture setup for every %d ms. scale = %d.  Output: %s", interval, (int) (scale * 100), outputPath);
    
    return ret;
}

extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env)
{
    env->AddFunction
    (
        "AvxFrameCapture",
        "cs[interval_ms]i[scale]i",
        AvxFrameCapture::Create, 
        0
    );

    return "AvxFrameCapture";
}
