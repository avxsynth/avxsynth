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

#ifndef CLIPPROXY_H
#define CLIPPROXY_H

#include "avxplugin.h"


using namespace avxsynth;

namespace avxsynth
{  
    
    class ClipProxy : public IClip
    {

    public:
        
        static ClipProxy* Create(PClip outer) {return new ClipProxy(outer);}
        virtual ~ClipProxy()
        {
            ClearCache();
            this->outer = 0;
        }
        
        virtual PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env)
        {
            if(this->cachedFrame != 0 && this->cachedFrame->n != n)
                ClearCache();
            
            if(this->cachedFrame != 0)
                return this->cachedFrame->frame;
            
            return this->outer->GetFrame(n, env);
        }
        
        virtual bool __stdcall GetParity(int n){return this->outer->GetParity(n);}
        virtual void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env)
        {
            this->outer->GetAudio(buf, start, count, env);
        }
        virtual void __stdcall SetCacheHints(int cachehints,size_t frame_range){this->outer->SetCacheHints(cachehints, frame_range);}
        virtual const VideoInfo& __stdcall GetVideoInfo() {return this->outer->GetVideoInfo();}
        
        
        void SetCache(int n, PVideoFrame frame)
        {
            ClearCache();
            this->cachedFrame = new CachedFrame(n, frame);
        }
        
        void ClearCache()
        {
            if(this->cachedFrame != 0)
            {
                delete this->cachedFrame;
                this->cachedFrame = 0;                
            }
        }
        
    private:
        ClipProxy(PClip _outer) : cachedFrame(0), outer(_outer){};
        
        // don't allow copy ctor or assignments
        ClipProxy(ClipProxy const& cpy){};
        ClipProxy& operator=(ClipProxy const& cpy)
        {
            return *this;
        }

        class CachedFrame
        {
        public:
            CachedFrame(int _n, PVideoFrame _frame) : n(_n), frame(_frame){};
            CachedFrame(CachedFrame const& cpy) {*this = cpy;}
            virtual ~CachedFrame(){};
            
            CachedFrame& operator=(CachedFrame const& cpy)
            {
                if(this != &cpy)
                {
                    this->n = cpy.n;
                    this->frame = cpy.frame;
                }
                return *this;
            }
            int n;
            PVideoFrame frame;
        };
        
        CachedFrame *cachedFrame;
        
        PClip outer;
       
    };
    
    class PClipProxy : public PClip
    {
    public:
        
        PClipProxy() : PClip() {}
        PClipProxy(ClipProxy *x) : PClip((IClip*) x), clipProxy(x){ };
        PClipProxy(PClip const& x) : PClip(x), clipProxy(ClipProxy::Create(x)){ };
        PClipProxy(PClipProxy const& x) : PClip((IClip*) x.clipProxy), clipProxy(x.clipProxy) {};
        
        virtual ~PClipProxy(){};
        
        PClipProxy& operator=(PClipProxy const& cpy)
        {
            if(this != &cpy)
            {
                Clear();
                PClipProxy::operator=(cpy);
                this->clipProxy = cpy.clipProxy;
            }
            
            return *this;
        }
        
        PClipProxy& operator=(PClip const& cpy)
        {
            if(this != &cpy)
            {
                Clear();
                PClipProxy clipProxy((PClip) cpy);
                return *this = clipProxy;
            }
            
            return *this;
        }
         
        ClipProxy* GetClipProxy(){return this->clipProxy;}
        
        void Clear()
        {
            PClip::operator=(0);
            this->clipProxy = 0;
        }
        
    private:
        PClipProxy(IClip* x){};
        ClipProxy* clipProxy;
    };    
    
} // namespace avxsynth

#endif // CLIPPROXY_H
