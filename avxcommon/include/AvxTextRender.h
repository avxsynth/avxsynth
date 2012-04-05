// Copyright (c) 2012 Anne Aaron, David Ronca, Milan Stevanovic, Pradip Gajjar.  All rights reserved.
// avxanne@gmail.com, videophool@hotmail.com, avxsynth@gmail.com, pradip.gajjar@gmail.com
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

#ifndef __AVX_TEXT_RENDERER_H__
#define __AVX_TEXT_RENDERER_H__

#ifdef _WIN32
#error("Can not use this library with WIN32")
#endif

#include "TextConfig.h"
#include "TextLayout.h"
#include "AvxException.h"

namespace avxsynth 
{
    class AvxTextRender
    {
    public:
        struct FrameBuffer
        {
            FrameBuffer() : originalBuffer(0), width(0), height(0), originalStride(0)
            {
            }
            
            FrameBuffer(unsigned char* originalBuffer, int width, int height, int originalStride)
            {
                this->originalBuffer = originalBuffer;
                this->width = width;
                this->height = height;
                this->originalStride = originalStride;
            }
            
            unsigned char*    originalBuffer;
            int               width;
            int               height;
            int               originalStride;
            
        private:
            // don't allow
            FrameBuffer(FrameBuffer const& cpy){};
            FrameBuffer& operator=(FrameBuffer const& cpy)
            {
                return *this;
            }
        };
        
        /**
         * bit flag rendering options 
         */
        enum RenderOptions
        {
            RenderOptions_None = 0,
            RenderOptions_ResizeToFit = 1,       // text will bre resized to fit the specified rectangle or screen
        };
        
        
        static void RenderSubtitleText(const char* strText, FrameBuffer & trd, TextConfig const& textConfig) throw(AvxException);
        static void RenderText
        (
            const char* strText, FrameBuffer & trd, TextConfig const& textConfig, TextLayout const& layout, unsigned int options = RenderOptions_None
        ) throw(AvxException);
    
    };

}; // namespace avxsynth

#endif // __AVX_TEXT_RENDERER_H__
