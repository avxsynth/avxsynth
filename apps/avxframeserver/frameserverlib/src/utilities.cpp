// Copyright (c) 2012 Anne Aaron, Milan Stevanovic, Pradip Gajjar.  All rights reserved.
// avxanne@gmail.com, avxsynth@gmail.com, pradip.gajjar@gmail.com
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

#include "utilities.h"
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <avxiface.h>

namespace avxsynth
{
	
#define BITS_PER_BYTE	(8)

bool DebugDumpRGBBuffer(const char* pImageBuffer, long nImageBytes, long nImageWidth, long nImageHeight, long nBitsPerPixel, long nFrameIndex)
{
  if((NULL == pImageBuffer) || 
     (0 == nImageBytes)     || 
     (0 == nImageWidth)     || 
     (0 == nImageHeight)    || 
     (0 == nBitsPerPixel))
  {
    AVXLOG_ERROR("%s got incorrect input arguments", __FUNCTION__);
    return false;
  }
  
  char filename[FILENAME_MAX];
  sprintf(filename, "~/Desktop/AVISynthDumpFrames/OutputFrame_%04ld_%ld_x_%ld.rgb", nFrameIndex, nImageWidth, nImageHeight);
  FILE* fp1 = fopen(filename, "wb");
  if(NULL == fp1)
  {
    AVXLOG_ERROR("Failed opening file %s for writing", filename);
    return false;
  }

  long nBytesPerPixel = nBitsPerPixel/BITS_PER_BYTE;
  long nPixels        = nImageBytes/nBytesPerPixel;
  long nPitchPixels    = nPixels/nImageHeight;
  for(unsigned int k = 0; k < nImageHeight; k++)
  {
    fwrite(pImageBuffer + k*nPitchPixels*nBytesPerPixel, nBytesPerPixel, nImageWidth, fp1);
    fflush(fp1);
  }
  fclose(fp1);
  return true;
}

bool AddRGBBufferToRawDumpFile(const char* pImageBuffer, long nImageBytes, long nImageWidth, long nImageHeight, long nBitsPerPixel, FILE* fpDump)
{
  if((NULL == pImageBuffer) || 
     (0 == nImageBytes)     || 
     (0 == nImageWidth)     || 
     (0 == nImageHeight)    || 
     (0 == nBitsPerPixel)   ||
     (NULL == fpDump))
  {
    AVXLOG_ERROR("%s got incorrect input arguments", __FUNCTION__);
    return false;
  }

  long nBytesPerPixel = nBitsPerPixel/BITS_PER_BYTE;
  long nPixels        = nImageBytes/nBytesPerPixel;
  long nPitchPixels    = nPixels/nImageHeight;
  for(unsigned int k = 0; k < nImageHeight; k++)
  {
    fwrite(pImageBuffer + k*nPitchPixels*nBytesPerPixel, nBytesPerPixel, nImageWidth, fpDump);
    fflush(fpDump);
  }
  return true;  
}

bool DebugDumpI420Buffer(const char* pImageBuffer, long nImageBytes, long nImageWidth, long nImageHeight, long nBitsPerPixel, long nFrameIndex)
{
  if((NULL == pImageBuffer) || 
     (0 == nImageBytes)     || 
     (0 == nImageWidth)     || 
     (0 == nImageHeight)    || 
     (0 == nBitsPerPixel))
  {
    AVXLOG_ERROR("%s got incorrect input arguments", __FUNCTION__);
    return false;
  }
  
  char filename[FILENAME_MAX];
  sprintf(filename, "~/Desktop/AVISynthDumpFrames/OutputFrame_%04ld_%ld_x_%ld.i420", nFrameIndex, nImageWidth, nImageHeight);
  FILE* fp1 = fopen(filename, "wb");
  if(NULL == fp1)
  {
    AVXLOG_ERROR("Failed opening file %s for writing", filename);
    return false;
  }

  fwrite(pImageBuffer, 1, nImageBytes, fp1);
  fclose(fp1);
  return true;  
}

bool AddI420BufferToRawDumpFile(const char* pImageBuffer, long nImageBytes, long nImageWidth, long nImageHeight, long nBitsPerPixel, FILE* fpDump)
{
  if((NULL == pImageBuffer) || 
     (0 == nImageBytes)     || 
     (0 == nImageWidth)     || 
     (0 == nImageHeight)    || 
     (0 == nBitsPerPixel)   ||
     (NULL == fpDump))
  {
    AVXLOG_ERROR("%s got incorrect input arguments", __FUNCTION__);
    return false;
  }

  fwrite(pImageBuffer, 1, nImageBytes, fpDump);
  fflush(fpDump);
  return true;  
}

const char* GetUserHomeDirectory(void)
{
  uid_t uid = getuid();
  struct passwd *user_info;
  user_info = getpwuid(uid);	
  if(NULL == user_info)
    return NULL;
  
  return user_info->pw_dir;
}

} // namespace avxsynth
