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

#include "Utils.h"
#include "memory.h"

namespace avxsynth
{
CaptionReader::CaptionReader(const char* path) : file(0)
{
    FILE* file = fopen(path, "r");
    
    if(!file)
        AvxException::ThrowCrtError("fopen", errno);
    
    this->file = file;
}

CaptionReader::~CaptionReader()
{
    if(this->file != 0)
        fclose(file);
    
    this->file = 0; 
}

Utf8String CaptionReader::ReadLine()
{
    char buf[1024];

    char* pInBuf = fgets(&buf[0], 1024, this->file);
    if(pInBuf == 0)       
        throw EOFException();
    
    size_t inSize = strlen(pInBuf);
    while(buf[inSize - 1] == '\r' || buf[inSize - 1] == '\n')
    {
        buf[inSize-1] = '\0';
        --inSize;
    }
    
    return Utf8String(buf);

}

} // namespace avxsynth