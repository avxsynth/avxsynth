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

#ifndef AVX_STRING_H_INCLUDED
#define AVX_STRING_H_INCLUDED

#include <stdlib.h>
#include <stdarg.h>
#include <string>
#include <vector>
#include <stdio.h>

using namespace std;

namespace avxsynth
{
    class Utf8String: public basic_string< char > 
    {
    public:

        Utf8String() : basic_string(){};
        Utf8String(const char* str) : basic_string(str){};
        Utf8String(basic_string<char> const& cpy){*this = cpy;}

        operator const char*() {return c_str();}
        
        Utf8String& operator=(basic_string<char>  const& cpy)
        {
            if(this != &cpy)
            {
                basic_string::assign(cpy.c_str());
            }

            return *this;
        }

        Utf8String& operator=(const char* cpy)
        {
            basic_string::assign(cpy);
            return *this;
        }
        
        operator const char*() const {return c_str();}

        void FormatV(const char* fmt, va_list args)
        {
            int bufSize = 1024;//vscwprintf(fmt, args) + 1;

            vector<char> buf(bufSize);
            char* p = &buf[0];
            vsnprintf(p, buf.size(), fmt, args);
            basic_string::assign(&buf[0]);
        }

        static Utf8String Build(const char* fmt, ...)
        {
            va_list args;
            va_start(args, fmt);
            Utf8String ret;
            ret.FormatV(fmt, args);

            return ret;
        }

        void Format(const char* fmt, ...)
        {
            va_list args;
            va_start(args, fmt);

            FormatV(fmt, args);

        }


        Utf8String& TrimFront()
        {
            if(empty())
                return *this;

            unsigned int start = 0;

            for(; start < length(); ++start)
            {
                if(at(start) != ' ' && at(start) != '\t' && at(start) != '\r' && at(start) != '\n')
                    break;
            }

            *this = substr(start);

            return *this;
        }

        Utf8String& TrimBack()
        {
            if(empty())
                return *this;

            unsigned int end = length() - 1;

            for (; end > 0; --end)
            {
                if(at(end) != ' ' && at(end) != '\t' && at(end) != '\r' && at(end) != '\n')
                    break;
            }

            *this = substr(0, end + 1);

            return *this;
        }

        Utf8String& Trim()
        {
            TrimFront();
            return TrimBack();
        }

        Utf8String Parse(Utf8String const& seperators)
        {
            size_t offset = basic_string::find_first_of(seperators);

            if(offset == npos)
                return *this;       

            Utf8String ret = substr(0, offset);

            *this = substr(offset + 1);

            return ret; 
        }

        

    private:
        

    };
} // namespace avxsynth
#endif // #ifndef AVX_STRING_H_INCLUDED


