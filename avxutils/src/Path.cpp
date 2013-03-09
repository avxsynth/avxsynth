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

#include "utils/Path.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

namespace avxsynth
{
    void Path::SplitPath(const char* fullPath, Utf8String& path, Utf8String& name, Utf8String& ext)
    {
        path.clear();
        name.clear();
        ext.clear();
/*        
        vector<char> resolvedPathBuf(PATH_MAX);
        size_t sz = resolvedPathBuf.size();
        char* rp = realpath(fullPath, 0);
        int _err = errno;
*/
        Utf8String resolvedPath(fullPath);
        
        // extension is to the right of the last '.' inclusive
        size_t idx = resolvedPath.find_last_of('.');
        if(idx != Utf8String::npos)
        {
            ext = resolvedPath.substr(idx);
            resolvedPath = resolvedPath.substr(0, idx);        
        }

        // file name is to the left of the last '/' minus extention
        idx = resolvedPath.find_last_of('/');
        if(idx != Utf8String::npos && idx != resolvedPath.length() - 1)
        {
            name = resolvedPath.substr(idx + 1);
            path = resolvedPath.substr(0, idx);        
        }
    }

    Utf8String Path::FullPath(const char* file)
    {
        Utf8String ext, name, path;

        SplitPath(file, path, name, ext);

        return path;
    }

    Utf8String Path::FileName(const char* file)
    {
        Utf8String ext, name, path;

        SplitPath(file, path, name, ext);

        return name;
    }

    Utf8String Path::FileExt(const char* file)
    {
        Utf8String ext, name, path;

        SplitPath(file, path, name, ext);

        return ext;
    }
}

