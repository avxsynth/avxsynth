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

#ifndef _AVX_EXCEPTION_H
#define _AVX_EXCEPTION_H

#include "AvxString.h"
#include <stdio.h>

using namespace avxsynth;

namespace avxsynth
{
    class AvxException
    {
    public:
        AvxException(){}
        AvxException(AvxException const& cpy)
        {
            *this = cpy;
        }

        AvxException(const char* fmt, ...)
        {
            va_list args;
            va_start(args, fmt);

            Utf8String msg;
            msg.FormatV(fmt, args);

            this->msg = msg;
        }

        AvxException(Utf8String const& _msg) : msg(_msg){};

        AvxException& operator=(AvxException const& cpy)
        {
            if(this != &cpy)
            {
                this->msg = cpy.msg;
            }

            return *this;
        }

        static void Throw(char const* fmt, ...)
        {
            va_list va;
            va_start(va, fmt);
            throw AvxException(fmt, va);
        }

        static void ThrowCrtError(char const* func, int crtErr)
        {
            vector<char> buf(1024);
            //_wcserror_s(pBuf, buf.size(), crtErr);
            Utf8String msg;
            msg.Format("%s failed.  error: %s", func, L"CRT Error");
            throw AvxException(msg);
        }

        const Utf8String& GetMsg() const {return this->msg;}

    private:
        Utf8String msg;
    };
} // namespace avxsynth

#endif // #ifndef _AVX_EXCEPTION_H