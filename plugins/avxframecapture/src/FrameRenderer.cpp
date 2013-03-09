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


#include "FrameRenderer.h"
#include "utils/AvxException.h"
#include "utils/Path.h"
#include <stdio.h>
#include <errno.h>

#define JPEG_CJPEG_DJPEG	/* define proper options in jconfig.h */
#define JPEG_INTERNAL_OPTIONS	/* cjpeg.c,djpeg.c need to see xxx_SUPPORTED */
#include <jpeglib.h>
#include <jerror.h>		/* get library error codes too */

#include "avxlog.h"

#include <vector>
using namespace std;

#ifdef MODULE_NAME
#undef MODULE_NAME
#endif

#define MODULE_NAME AvxFrameCapture

#define JVERSION    "8b  16-May-2010"

#define JCOPYRIGHT  "Copyright (C) 2010, Thomas G. Lane, Guido Vollbeding"

/* Create the add-on message string table. */

#define JMESSAGE(code,string)	string ,

static const char * const cdjpeg_message_table[] = 
{
    #include <jerror.h>
    NULL
};

FrameRenderer::FrameRenderer() 
{
}


FrameRenderer::~FrameRenderer(void)
{
}

vector<LPBYTE> GetRowPtrs(LPBYTE data, ULONG size, ULONG rowSize, ULONG padding)
{
    vector<LPBYTE> ret;

    for(ULONG cur = 0; cur < size; cur += rowSize + padding)
    {
        ret.push_back(data + cur);
    }

    return ret;
}

void InvertPixels(LPBYTE buf, long size)
{
    for(int i = 0; i < size - 3; i += 3)
    {
        BYTE  tmp = buf[i];
        buf[i] = buf[i + 2];
        buf[i + 2] = tmp;
    }
}

/* Error exit handler: does not return to caller */
void error_exit(j_common_ptr cinfo)
{
    char buffer[JMSG_LENGTH_MAX];

    /* Create the message */
    (*cinfo->err->format_message) (cinfo, buffer);

    throw AvxException("IJPG library error: %hs", buffer);
}



/* Routine that actually outputs a trace or error message */
void output_message(j_common_ptr cinfo)
{
    
    char buffer[JMSG_LENGTH_MAX];

    /* Create the message */
    (*cinfo->err->format_message) (cinfo, buffer);
    
    AVXLOG_NOTICE("%s", buffer);

}

void FrameRenderer::RenderFrame(PVideoFrame frame, Size const& frameSize, Utf8String const& outputPath)
{
    
    { // save this frame

        
        const int bitsPerPixel = 24;
        const int bytesPerPixel = bitsPerPixel / 8;

        struct jpeg_compress_struct cinfo;
        struct jpeg_error_mgr jerr;
        cinfo.err = jpeg_std_error(&jerr);
        jerr.error_exit = error_exit;
        jerr.output_message = output_message;
        jpeg_create_compress(&cinfo);
        /* Add some application-specific error messages (from cderror.h) */
        jerr.addon_message_table = cdjpeg_message_table;
        jerr.first_addon_message = 0;
        jerr.last_addon_message = 0;
        cinfo.in_color_space = JCS_RGB;
        jpeg_set_defaults(&cinfo);

        // progressive should be a profile configuration setting
        jpeg_simple_progression(&cinfo);

        FILE* outFile = fopen(outputPath, "wb");
        
        if(outFile == 0)
            AvxException::ThrowCrtError("FrameRenderer::RenderFrame", errno);

        cinfo.image_height = frameSize.Height();
        cinfo.image_width = frameSize.Width();

        cinfo.density_unit = 0;
        cinfo.X_density = 0;
        cinfo.Y_density = 0;

        cinfo.input_components = bytesPerPixel;

        /* Specify data destination for compression */
        jpeg_stdio_dest(&cinfo, outFile);

        /* Start compressor */
        jpeg_start_compress(&cinfo, TRUE);
        long rowSize = bytesPerPixel * frameSize.Width();

        vector<LPBYTE> rows = GetRowPtrs((BYTE*) frame->GetReadPtr(), frameSize.Height() * frameSize.Width() * bytesPerPixel, rowSize, 0);

        for(UINT i = 0; i < rows.size(); ++i)
        {
            // for now, assume aligned, will have to fix
            int idx = rows.size() - (i+1);
            InvertPixels(rows[idx], rowSize);
            jpeg_write_scanlines(&cinfo, (JSAMPARRAY) &rows[idx], 1);
        }

        jpeg_finish_compress(&cinfo);
        jpeg_destroy_compress(&cinfo);

        fclose(outFile);

    }

}
