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

#ifndef _FRAME_CPATURE_PROFILE_H
#define _FRAME_CPATURE_PROFILE_H

#include "utils/AvxString.h"

using namespace avxsynth;

class FrameCaptureProfile
{
public:   
    FrameCaptureProfile(FrameCaptureProfile const& cpy);
    FrameCaptureProfile(const char*, int64_t timeInterval);

    ~FrameCaptureProfile(void);
    
    Utf8String const& GetOutputPath() const {return this->outputPath;}
    int64_t GetTimeInterval() const {return this->timeInterval;}

private:

    const Utf8String outputPath;    
    const int64_t timeInterval;


};

#endif // #define _FRAME_CPATURE_PROFILE_H

