// Copyright (c) 2012 David Ronca.  All rights reserved.
//  videophool@hotmail.com
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

/**********************************************************************************************************************************************/
// avxsubtitle is an avxsynth plugin that can be used to burn subtitles into video.
// The parameters are <CaptionFilePath>, font=<font name> size=<font size> stroke_size=<size> ext_color=<RGB value> stroke_color=<RGB value>
// Currently, only SRT captions are supported.
/**********************************************************************************************************************************************/


#include "AvxSubtitle.h"
#include "SubtitleParser.h"
#include "AvxException.h"
#include "AvxTextRender.h"
#include "avxlog.h"

using namespace avxsynth;


#ifdef MODULE_NAME
#undef MODULE_NAME
#endif

#define MODULE_NAME AvxSubtitle

AvxSubtitle::AvxSubtitle(PClip _child, CaptionList _captions,  TextConfig const& _config)
 : GenericVideoFilter(_child), captions(_captions), config(_config), curCaption(CaptionEntry::Empty)
{

}

AvxSubtitle::~AvxSubtitle(void)
{
    Clean();
}

void AvxSubtitle::Clean()
{
#ifdef _WIN32
    if(this->blt != 0)
    {
        delete this->blt;
        this->blt = 0;
    }
#endif // #ifdef _WIN32
}

PVideoFrame __stdcall AvxSubtitle::GetFrame(int n, IScriptEnvironment* env) 
{
    PVideoFrame frame = child->GetFrame(n, env);
    env->MakeWritable(&frame);
    int ms = (int)(((__int64)n * vi.fps_denominator * 1000 / vi.fps_numerator)%1000);
    int sec = (int)((__int64)n * vi.fps_denominator / vi.fps_numerator);
    int min = sec/60;
    int hour = sec/3600;
    
    long long curTime = ((__int64)n * vi.fps_denominator * 1000 / vi.fps_numerator) * 10000;

    try
    {
        CaptionEntry const& curCaption = this->captions.GetCaptionForTime(curTime);
        
        if(curCaption != CaptionEntry::Empty)
        {
            if(this->curCaption != curCaption)
                AVXLOG_INFO("Caption change: %s", curCaption.toDebugString().c_str());
            int rowHight = vi.height / 15;
            int top =  vi.height - (rowHight * curCaption.TextLines().size());
            
            AvxTextRender::FrameBuffer ptrc(frame->GetWritePtr(), vi.width, vi.height, frame->GetPitch());
                 
            AvxTextRender::RenderSubtitleText(curCaption.toDisplayString().c_str(), ptrc, this->config);
        }
        else
        {
            if(this->curCaption != CaptionEntry::Empty)
                AVXLOG_INFO("%s", "Clear current caption");
        }
        
        this->curCaption = curCaption;
   
    }
    catch(AvxException &e)
    {
        env->ThrowError(Utf8String(e.GetMsg()).c_str());
    }

    return frame;
}

AVSValue __cdecl AvxSubtitle::Create(AVSValue args, void*, IScriptEnvironment* env) 
{
    PClip upstream = args[0].AsClip();
    const char* subtitlePath  = args[1].AsString();
    const char* inputEncoding = args[7].AsString("UTF-8");
    
    VideoInfo vi = upstream->GetVideoInfo();
    
    if(!vi.IsRGB() && !vi.IsYUY2() && !vi.IsYV12())
        env->ThrowError("AvxSubtitle: Only RGB24, RGB32, YUY2 and YV12 colorspaces are supported");
    
    if(!vi.IsRGB24())
    { // need to convert
         upstream = env->Invoke("ConvertToRGB24", AVSValue(upstream)).AsClip();
    }

    CaptionList captions;
    try
    {
        captions = SubtitleParser::Parse(subtitlePath);
    }
    catch(AvxException &e)
    {
        env->ThrowError(Utf8String(e.GetMsg()).c_str());
    }
    
    AVXLOG_INFO("Caption file: %s.  %d captions detected", subtitlePath, captions.size());

    PClip ret = new AvxSubtitle
    (
        upstream, 
        captions, 
        TextConfig
        (
            args[2].AsString("Arial"), 
            args[3].AsInt(24),
            args[4].AsFloat(0.75),
            args[5].AsInt(0xff00ff), 
            args[6].AsInt(0xffffff)
        )
    );
    
    if(vi.IsYUY2())
        ret = env->Invoke("ConvertToYUY2", AVSValue(ret)).AsClip();
    else if(vi.IsYV12())
        ret = env->Invoke("ConvertToYV12", AVSValue(ret)).AsClip();
    else if(vi.IsRGB32())
        ret = env->Invoke("ConvertToRGB32", AVSValue(ret)).AsClip();
    
    return ret;
}

extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env)
{
    env->AddFunction
    (
        "AvxSubtitle",
        "cs[font]s[size]i[stroke_size]i[text_color]i[stroke_color]i[input_encoding]s",
        AvxSubtitle::Create, 
        0
    );

    return "AvxSubtitle";
}




