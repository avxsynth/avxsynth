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

#ifndef _TEXT_CONFIG_H
#define _TEXT_CONFIG_H

#include "AvxString.h"

using namespace avxsynth;

namespace avxsynth
{
    class TextConfig
    {
    public:
        class Color;
            
        struct Color
        {
            Color(int color = 0) : fAlpha(0), fR((color & 0xFF)/255.0), fG(((color >>  8) & 0xFF)/255.0), fB(((color >> 16) & 0xFF)/255.0)
            {
                
            }
            
            Color(double _fR, double _fG, double _fB) : fAlpha(0), fR(_fR), fG(_fG), fB(_fB){};
            
            Color(Color const &cpy) {*this = cpy;}
            
            Color& operator=(Color const& cpy)
            {
                if(this != &cpy)
                {
                    this->fAlpha = cpy.fAlpha;
                    this->fR = cpy.fR;
                    this->fG = cpy.fG;
                    this->fB = cpy.fB;
                }
                return *this;
            }
            
            Color Invert() const
            {
                return Color(1-fR, 1-fG, 1-fB);
            }
            
            operator int() const {return ((int) (fR * 255)) | ((int) (fG * 255) << 8) | ((int)(fB * 255) << 16);}
            
            double fAlpha;
            double fR;
            double fG;
            double fB;
        };
        

        TextConfig
        (
            Utf8String const& _fontname, int _size, float _strokeSize, Color const& _textcolor, Color const& outlineColor
        ) : fontname(_fontname), size(_size), strokeSize(_strokeSize), textcolor(_textcolor), strokeColor(outlineColor)
        {
        }
        
        TextConfig(TextConfig const& cpy) 
        : fontname(cpy.fontname), size(cpy.size), strokeSize(cpy.strokeSize), textcolor(cpy.textcolor), strokeColor(cpy.strokeColor)
        {
        }

        const Utf8String fontname;
        const int size;
        const float strokeSize;
        const Color textcolor;
        const Color strokeColor;

        static const TextConfig  Empty;
    private:
        TextConfig() : fontname(""), size(0), strokeSize(0), textcolor(0), strokeColor(0)
        {
        }

    };
} // namespace avxsynth

#endif // #ifndef _TEXT_CONFIG_H