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

#include <stdlib.h>
#include "avxlog.h"
#include "utils/AvxException.h"
#include <cairo/cairo.h>
#include <pango-1.0/pango/pangocairo.h>
#include "utils/AvxTextRender.h"
#include <pango-1.0/pango/pango-layout.h>

namespace avxsynth
{

#ifdef MODULE_NAME
#undef MODULE_NAME
#endif

#define MODULE_NAME common::AvxTextRender

    namespace avxsubtitle
    {
        PangoAlignment FromHorizontalAlignment(TextLayout::HorizontalAlignment halign)
        {
            switch(halign)
            {
                case TextLayout::Left:
                    return PANGO_ALIGN_LEFT;

                case TextLayout::HCenter:
                    return PANGO_ALIGN_CENTER;

                case TextLayout::Right:
                    return PANGO_ALIGN_RIGHT;
            }

            return PANGO_ALIGN_LEFT;
        }

        struct PangoStrideBuf
        {
            PangoStrideBuf() : pData(0), nCairoStride(0){};
            ~PangoStrideBuf()
            {
                    if(pData != 0)
                        delete [] pData;
            }

            unsigned char *pData;
            int nCairoStride;
        };

        void adjustStride(AvxTextRender::FrameBuffer const& trd, PangoStrideBuf &strideBuf)
        {
            //
            // the stride RGB buffer used in AVISynth does not match the libcairo recommended
            // stride value. For that sake, we will first repack the original image buffer to
            // the buffer that matches the libcairo stride requirements
            //
            strideBuf.nCairoStride = cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, trd.width);
            strideBuf.pData = new unsigned char[trd.height*strideBuf.nCairoStride];
            if(NULL == strideBuf.pData)
            {
                throw AvxException
                (
                    "Failed allocating %d bytes (%d x %d bytes) for Cairo surface",
                    trd.height*strideBuf.nCairoStride, trd.height, strideBuf.nCairoStride
                );
            }

            if(strideBuf.nCairoStride == trd.originalStride)
            {
                //
                // Repacking RGB32 to RGB32 with inversion
                //
                for(int i = 0; i < trd.height; i++)
                {
                    // single pass through this loop processes one
                    // horizontal line
                    unsigned char* pSrc  = trd.originalBuffer + i*trd.originalStride;
                    unsigned char* pDest = strideBuf.pData + (trd.height - 1 - i)*strideBuf.nCairoStride;
                    for(int j = 0; j < trd.width; j++)
                    {
                        pDest[4 * j + 0] = pSrc[4*j + 0];
                        pDest[4 * j + 3] = pSrc[4*j + 1];
                        pDest[4 * j + 2] = pSrc[4*j + 2];
                        pDest[4 * j + 1] = pSrc[4*j + 3];
                    }
                }
            }
            else
            {
                //
                // Repacking RGB24 to RGB32 with inversion
                //
                for(int i = 0; i < trd.height; i++)
                {
                    // single pass through this loop processes one
                    // horizontal line
                    unsigned char* pSrc  = trd.originalBuffer + i*trd.originalStride;
                    unsigned char* pDest = strideBuf.pData + (trd.height - 1 - i)*strideBuf.nCairoStride;
                    for(int j = 0; j < trd.width; j++)
                    {
                        pDest[4 * j + 2] = pSrc[3*j + 0];
                        pDest[4 * j + 1] = pSrc[3*j + 1];
                        pDest[4 * j + 0] = pSrc[3*j + 2];
                    }
                }
            }
        }

        void repackToOriginalStride(AvxTextRender::FrameBuffer const& trd, PangoStrideBuf &strideBuf)
        {
            if(strideBuf.nCairoStride == trd.originalStride)
            {
                for(int i = 0; i < trd.height; i++)
                {
                    // single pass through this loop processes one
                    // horizontal line
                    unsigned char* pSrc  = strideBuf.pData + (trd.height - 1 - i)*strideBuf.nCairoStride;
                    unsigned char* pDest = trd.originalBuffer + i*trd.originalStride;
                    for(int j = 0; j < trd.width; j++)
                    {
                        pDest[4*j + 0] = /*B*/ pSrc[4 * j + 2];
                        pDest[4*j + 1] = /*G*/ pSrc[4 * j + 1];
                        pDest[4*j + 2] = /*R*/ pSrc[4 * j + 0];
                        pDest[4*j + 3] = /*alpha*/ pSrc[4 * j + 3];
                    }
                }
            }
            else
            {
                //
                // Repack pixels to original RGB24 stride
                //
                for(int i = 0; i < trd.height; i++)
                {
                    // single pass through this loop processes one
                    // horizontal line
                    unsigned char* pSrc  = strideBuf.pData + (trd.height - 1 - i)*strideBuf.nCairoStride;
                    unsigned char* pDest = trd.originalBuffer + i*trd.originalStride;
                    for(int j = 0; j < trd.width; j++)
                    {
                        pDest[3*j + 0] = pSrc[4 * j + 2];
                        pDest[3*j + 1] = pSrc[4 * j + 1];
                        pDest[3*j + 2] = pSrc[4 * j + 0];
                    }
                }
            }
         }

        void RenderShowFrameNumberColumn
        (
            const char* strText,
            AvxTextRender::FrameBuffer& trd,
            const TextConfig& textConfig,
            cairo_t *cr,
            PangoLayout *layout,
            int nLeftCoordinate
        )
        {
            double fontSize = textConfig.size;
            unsigned int nAvailableRows = trd.height/fontSize;
            unsigned int nFrameNumber = atoi(strText);

            unsigned int nInitialOffsetFromTop = (nAvailableRows - 1) % nAvailableRows;
            unsigned int nInitialOffsetFromBottom = (nAvailableRows - 1 - nInitialOffsetFromTop); // 0 to 9
            for(unsigned int i = 0; i < nAvailableRows; i++)
            {
                unsigned int nOffsetFromBottom = (nInitialOffsetFromBottom + i) % nAvailableRows;
                unsigned int nTopCoordinate = trd.height - (nOffsetFromBottom + 1)*fontSize;

                cairo_move_to(cr, nLeftCoordinate, nTopCoordinate);
                char temp[6] = {0};
                sprintf(temp, "%05d", nFrameNumber);

                pango_layout_set_text (layout, temp, -1);
                pango_cairo_show_layout(cr, layout);
            }
        }


        void RenderShowFrameNumberScrolling
        (
            const char* strText,
            AvxTextRender::FrameBuffer& trd,
            const TextConfig& textConfig,
            cairo_t *cr,
            PangoLayout *layout,
            int nLeftCoordinate,
            unsigned int nFrames
        )
        {
            //
            // In addition to rendering the specified frame number, render also N previous
            // frame numbers. The value of N depends on
            //  a) how much was specified through the optionsParam value
            //  b) how many can fit the screen
            //
            // Scrolling will observe the left coordinate, but will incrementally adjust
            // the top coordinate to achieve the scrolling effect
            //
            double fontSize = textConfig.size;
            unsigned int nAvailableRows = trd.height/fontSize;
            unsigned int nRowsToDisplay = nFrames < nAvailableRows ? nFrames + 1 : nAvailableRows;
            unsigned int nStartFrameNumber = atoi(strText);

            unsigned int nInitialOffsetFromTop = (nFrames + nAvailableRows - 2) % nAvailableRows;
            unsigned int nInitialOffsetFromBottom = (nAvailableRows - 1 - nInitialOffsetFromTop); // 0 to 9
            unsigned int nTopCoordinate;
            for(unsigned int i = 0; i < nRowsToDisplay; i++)
            {
                unsigned int nOffsetFromBottom = (nInitialOffsetFromBottom + i) % nAvailableRows;
                nTopCoordinate = trd.height - (nOffsetFromBottom + 1)*fontSize;

                cairo_move_to(cr, nLeftCoordinate, nTopCoordinate);
                char temp[6] = {0,0,0,0,0,0};
                sprintf(temp, "%05d", nStartFrameNumber);

                pango_layout_set_text (layout, temp, -1);
                pango_cairo_show_layout(cr, layout);

                nStartFrameNumber--;
            }
        }

        void RenderOutlineText
        (
            AvxTextRender::FrameBuffer & trd,
            cairo_t *cr,
            PangoFontDescription *font_description,
            int x, int y,
            TextConfig const& textConfig,
            const char* strText,
            PangoAlignment hAlign,
            unsigned int options,
            unsigned int optionsParam = -1
        )
        {

            PangoLayout *layout;                            // layout for a paragraph of text
            layout = pango_cairo_create_layout(cr);                 // init pango layout ready for use

            pango_layout_set_text(layout, strText, -1);

            if(options & AvxTextRender::RenderOptions_ResizeToFit)
            {
                pango_layout_set_width(layout, trd.width * PANGO_SCALE * .9);
            }

            cairo_new_path(cr);
            cairo_move_to(cr, 0, 0);
            cairo_set_line_width(cr, textConfig.strokeSize);
            pango_layout_set_alignment(layout, hAlign);
            //pango_layout_set_justify(layout, TRUE);
            pango_layout_set_font_description (layout, font_description);
            pango_layout_set_text (layout, strText, -1);

            cairo_set_source_rgb (cr, textConfig.strokeColor.fR, textConfig.strokeColor.fG, textConfig.strokeColor.fB);
            cairo_move_to (cr, x, y);
            pango_cairo_update_layout(cr, layout);
            pango_cairo_layout_path(cr, layout);                    // draw the pango layout onto the cairo surface
            cairo_stroke_preserve(cr);                      // draw a stroke along the path, but do not fill it

            g_object_unref(layout);                         // free the layout
        }
    }

    void AvxTextRender::RenderSubtitleText(const char* strText, AvxTextRender::FrameBuffer & trd, TextConfig const& textConfig) throw (AvxException)
    {
        avxsubtitle::PangoStrideBuf strideBuf;

        avxsubtitle::adjustStride(trd, strideBuf);

        cairo_surface_t* pSurface =
        cairo_image_surface_create_for_data(strideBuf.pData, CAIRO_FORMAT_RGB24, trd.width, trd.height, strideBuf.nCairoStride);
        cairo_status_t status = cairo_surface_status(pSurface);

        if(NULL == pSurface || CAIRO_STATUS_SUCCESS != status)
        {
            throw AvxException("Failed creating cairo surface, status = %d\n", status);
        }

        cairo_t *cr = cairo_create (pSurface);

        PangoLayout *layout;
        PangoFontDescription *font_description;

        double fontSize = textConfig.size;

        font_description = pango_font_description_new ();
        pango_font_description_set_family (font_description, textConfig.fontname.c_str());
        pango_font_description_set_weight (font_description, PANGO_WEIGHT_BOLD);
        pango_font_description_set_absolute_size (font_description, PANGO_SCALE*(fontSize));
        pango_font_description_set_stretch(font_description, PANGO_STRETCH_ULTRA_EXPANDED);

        layout = pango_cairo_create_layout (cr);
        pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER);
        //pango_layout_set_justify(layout, TRUE);
        pango_layout_set_font_description (layout, font_description);
        pango_layout_set_text (layout, strText, -1);

        PangoRectangle rect;
        TextConfig::Color textColor = textConfig.textcolor;

        pango_layout_get_extents(layout, 0, &rect);
        int x = trd.width / 2 - ((rect.width / PANGO_SCALE) / 2);
        int vAdj = 10 + (rect.height  / PANGO_SCALE);
        int y = trd.height - vAdj;
        cairo_set_source_rgb (cr, textColor.fR, textColor.fG, textColor.fB);
        cairo_move_to (cr, x, y);
        pango_cairo_show_layout (cr, layout);

        g_object_unref (layout);

        if(textConfig.strokeSize > 0)
            avxsubtitle::RenderOutlineText(trd, cr, font_description, x, y, textConfig, strText, PANGO_ALIGN_CENTER, 0);

        pango_font_description_free (font_description);
        cairo_destroy (cr);

        avxsubtitle::repackToOriginalStride(trd, strideBuf);

        cairo_surface_destroy (pSurface);
    }


    void AvxTextRender::RenderText
    (
        const char* strText,
        AvxTextRender::FrameBuffer& trd,
        const TextConfig& textConfig,
        const TextLayout& textLayout,
        unsigned int options,
        unsigned int optionsParam
    ) throw(AvxException)
    {
        //AVXLOG_INFO("%s", __FUNCTION__);

        avxsubtitle::PangoStrideBuf strideBuf;

        adjustStride(trd, strideBuf);

        cairo_surface_t* pSurface =
        cairo_image_surface_create_for_data(strideBuf.pData, CAIRO_FORMAT_RGB24, trd.width, trd.height, strideBuf.nCairoStride);
        cairo_status_t status = cairo_surface_status(pSurface);

        if(NULL == pSurface || CAIRO_STATUS_SUCCESS != status)
        {
            throw AvxException("Failed creating cairo surface, status = %d\n", status);
        }

        cairo_t *cr = cairo_create (pSurface);

        PangoLayout *layout;
        PangoFontDescription *font_description;

        double fontSize = textConfig.size;

        PangoAlignment hAlign = avxsubtitle::FromHorizontalAlignment(textLayout.horizontalAlignment);

        font_description = pango_font_description_new ();
        pango_font_description_set_family (font_description, textConfig.fontname.c_str());
        pango_font_description_set_weight (font_description, PANGO_WEIGHT_BOLD);
        pango_font_description_set_absolute_size (font_description, PANGO_SCALE*(fontSize));
        pango_font_description_set_stretch(font_description, PANGO_STRETCH_ULTRA_EXPANDED);

        layout = pango_cairo_create_layout (cr);

        pango_layout_set_alignment(layout, hAlign);
        //pango_layout_set_justify(layout, TRUE);
        pango_layout_set_font_description (layout, font_description);
        pango_layout_set_text (layout, strText, -1);

        pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);

        TextConfig::Color textColor = textConfig.textcolor;

        if(options & RenderOptions_ResizeToFit)
        {
            pango_layout_set_width(layout, trd.width * PANGO_SCALE * .9);
        }

        int x = textLayout.rect.left;
        int y = textLayout.rect.top;
        cairo_set_source_rgb (cr, textColor.fR, textColor.fG, textColor.fB);

        if(options & RenderOptions_Scroll_SFN)
        {
            avxsubtitle::RenderShowFrameNumberScrolling(strText, trd, textConfig, cr, layout, x, optionsParam);
        }
        else if(options & RenderOptions_Column_SFN)
        {
           avxsubtitle::RenderShowFrameNumberColumn(strText, trd, textConfig, cr, layout, x);
        }
        else
        {
            cairo_move_to (cr, x, y);
            pango_cairo_show_layout (cr, layout);
        }

        if(textConfig.strokeSize)
            avxsubtitle::RenderOutlineText(trd, cr, font_description, x, y, textConfig, strText, hAlign,  options, optionsParam);

        g_object_unref (layout);

        pango_font_description_free (font_description);
        cairo_destroy (cr);
        //cairo_surface_write_to_png (pSurface, <your_home_directory>/Desktop/firstTry.png");

        repackToOriginalStride(trd, strideBuf);

        cairo_surface_destroy (pSurface);
    }

    void AvxTextRender::GetApproximateCharacterWidth
    (
        TextConfig const& textConfig,
        int & nCharacterWidth
    )
    {
        cairo_surface_t *surface    = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 100, 100);
        cairo_t *cr                 = cairo_create(surface);
        PangoLayout *layout         = pango_cairo_create_layout(cr);
        PangoContext *context       = pango_layout_get_context(layout);

        PangoLanguage *language     = pango_language_get_default();
        PangoFontDescription *desc  = pango_font_description_new ();
        pango_font_description_set_family (desc, textConfig.fontname.c_str());
        pango_font_description_set_weight (desc, PANGO_WEIGHT_BOLD);
        pango_font_description_set_absolute_size (desc, PANGO_SCALE*(textConfig.size));
        pango_font_description_set_stretch(desc, PANGO_STRETCH_ULTRA_EXPANDED);
        pango_layout_set_font_description(layout, desc);

        PangoFontMetrics *metrics   = pango_context_get_metrics(context, desc, language);

        nCharacterWidth = pango_font_metrics_get_approximate_char_width(metrics);
        nCharacterWidth = (nCharacterWidth + PANGO_SCALE)/PANGO_SCALE;

        pango_font_description_free(desc);
        g_object_unref(layout);

        cairo_destroy(cr);
        cairo_surface_destroy(surface);
   }


}; // namespace avxsynth
