// Avisynth v4.0  Copyright 2002 Ben Rudiak-Gould et al.
// http://www.avisynth.org
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
// import and export plugins, or graphical user interfaces.

#include "stdafx.h"

#include "convert_yv12.h"

namespace avxsynth {
  
  
#define INTERPOLATE_BYTES_1_1(a, b) ((BYTE) (((unsigned int) (a) + (unsigned int) (b)) >> 1 ));             // 1/2 : 1/2
#define INTERPOLATE_BYTES_3_1(a, b) ((BYTE) ((((unsigned int) (a))*3 + (unsigned int) (b)) >> 2));          // 3/4 : 1/4
#define INTERPOLATE_BYTES_7_1(a, b) ((BYTE) ((((unsigned int) (a))*7 + (unsigned int) (b)) >> 3));          // 7/8 : 1/8
#define INTERPOLATE_BYTES_5_3(a, b) ((BYTE) ((((unsigned int) (a))*5 + ((unsigned int) (b))*3) >> 3));      // 5/8 : 3/8

/********************************
 * C functions for
 * YUY2 to YV12
 * and YV12 to YUY2 
 * conversion
 * 
 * (c) 2012, Anne Aaron.
 * 
 ********************************/


/********************************
 * Progressive YUY2 to YV12
 ********************************/

void yuy2_to_yv12(const BYTE* src, int src_rowsize, int src_pitch, 
                  BYTE* dstY, BYTE* dstU, BYTE* dstV, int dst_pitchY, int dst_pitchUV,
                  int height) 
{
  
    int src_pitch2 = src_pitch*2;
    int dst_pitch2 = dst_pitchY*2;

    // Pointers to first byte in row
    BYTE *src_row = (BYTE *)src;                    // Initialize to first row                        
    BYTE *dstY_row = dstY;
    BYTE *dstU_row = dstU;
    BYTE *dstV_row = dstV;   
        
    for (int y=0; y<height; y+=2)                   // Process two input rows at a time
    {

        BYTE *srctop_ij = (BYTE *)src_row;          // Top input row
        BYTE *srcbot_ij =  srctop_ij + src_pitch;   // Bottom input row
        BYTE *dstYtop_ij = dstY_row;
        BYTE *dstYbot_ij = dstYtop_ij + dst_pitchY;        
        BYTE *dstU_ij = dstU_row;
        BYTE *dstV_ij = dstV_row;   
        
                
        for (int x=0; x<src_rowsize; x+=4)          
        {
            // Process 8 bytes at a time (2 macropixels)
            //    Y11 U1 Y12 V1
            //    Y21 U2 Y22 V2 
            
            *dstYtop_ij = *srctop_ij;               // Y11
            dstYtop_ij++;
            *dstYtop_ij = *(srctop_ij+2);           // Y12
            dstYtop_ij++;
            *dstYbot_ij = *srcbot_ij;               // Y21
            dstYbot_ij++;
            *dstYbot_ij = *(srcbot_ij+2);           // Y22
            dstYbot_ij++;
            
            *dstU_ij = INTERPOLATE_BYTES_1_1(*(srctop_ij+1), *(srcbot_ij+1));           // U = (U1 + U2)/2. Average U from top and bottom lines
            dstU_ij++;
            *dstV_ij = INTERPOLATE_BYTES_1_1(*(srctop_ij+3), *(srcbot_ij+3));           // V = (V1 + V2)/2. Average V from top and bottom lines
            dstV_ij++;
            
            srctop_ij+=4;                           // Move by 1 macropixel YUYV
            srcbot_ij+=4;                           // Move by 1 macropixel YUYV
        }
        
        // Increment pointers to move two input rows down 
        src_row += src_pitch2;
        dstY_row += dst_pitch2;        
        dstU_row += dst_pitchUV;
        dstV_row += dst_pitchUV;        
    }
 
}  
  
/********************************
 * Interlaced YUY2 to YV12
 ********************************/

void yuy2_i_to_yv12(const BYTE* src, int src_rowsize, int src_pitch, 
                    BYTE* dstY, BYTE* dstU, BYTE* dstV, int dst_pitchY, int dst_pitchUV,
                    int height) 
{
  
    int src_pitch2 = src_pitch*2;
    int dst_pitch2 = dst_pitchY*2;
    int dst_pitchUV2 = dst_pitchUV*2;
    int src_pitch4 = src_pitch2*2;
    int dst_pitch4 = dst_pitch2*2;

    /////////////////
    // First field //
    /////////////////
    
    // Pointers to first byte in row
    BYTE *src_row = (BYTE *)src;                    // Initialize to first row         
    BYTE *dstY_row = dstY;
    BYTE *dstU_row = dstU;
    BYTE *dstV_row = dstV;   
        
    for (int y=0; y<height; y+=4)                   // Process two input rows of one field at a time
    {

        BYTE *srctop_ij = (BYTE *)src_row;          // Top input row
        BYTE *srcbot_ij =  srctop_ij + src_pitch2;  // Bottom input row from the same field
        BYTE *dstYtop_ij = dstY_row;
        BYTE *dstYbot_ij = dstYtop_ij + dst_pitch2;        
        BYTE *dstU_ij = dstU_row;
        BYTE *dstV_ij = dstV_row;   
        
                
        for (int x=0; x<src_rowsize; x+=4)          
        {
            // Process 8 bytes of the first field 
            //    Y11 U1 Y12 V1
            //    --- -- --- --
            //    Y21 U2 Y22 V2 
            
            *dstYtop_ij = *srctop_ij;               // Y11
            dstYtop_ij++;
            *dstYtop_ij = *(srctop_ij+2);           // Y12
            dstYtop_ij++;
            *dstYbot_ij = *srcbot_ij;               // Y21
            dstYbot_ij++;
            *dstYbot_ij = *(srcbot_ij+2);           // Y22
            dstYbot_ij++;
            
            // U = (U1*3 + U2)/4
            // 75% upper, 25% lower
            *dstU_ij = INTERPOLATE_BYTES_3_1(*(srctop_ij+1), *(srcbot_ij+1));
            dstU_ij++;
            // V = (V1*3 + V2)/4. 
            // 75% upper, 25% lower            
            *dstV_ij = INTERPOLATE_BYTES_3_1(*(srctop_ij+3), *(srcbot_ij+3));       
            dstV_ij++;
            
            srctop_ij+=4;                           // Move by 1 macropixel YUYV
            srcbot_ij+=4;                           // Move by 1 macropixel YUYV
        }
        
        // Increment pointers to move two input rows down the field (four rows down the frame)
        src_row += src_pitch4;
        dstY_row += dst_pitch4;        
        dstU_row += dst_pitchUV2;
        dstV_row += dst_pitchUV2;        
    }
 
    //////////////////
    // Second field //
    //////////////////
    
    // Pointers to first byte in row
    src_row = (BYTE *)(src+src_pitch);              // Initialize to second row (first row of second field)         
    dstY_row = dstY+dst_pitchY;
    dstU_row = dstU+dst_pitchUV;
    dstV_row = dstV+dst_pitchUV;   
        
    for (int y=1; y<height; y+=4)                   // Process two input rows of one field at a time
    {

        BYTE *srctop_ij = (BYTE *)src_row;          // Top input row
        BYTE *srcbot_ij =  srctop_ij + src_pitch2;  // Bottom input row from the same field
        BYTE *dstYtop_ij = dstY_row;
        BYTE *dstYbot_ij = dstYtop_ij + dst_pitch2;        
        BYTE *dstU_ij = dstU_row;
        BYTE *dstV_ij = dstV_row;   
        
                
        for (int x=0; x<src_rowsize; x+=4)          
        {
            // Process 8 bytes of the second field 
            //    Y11 U1 Y12 V1
            //    --- -- --- --
            //    Y21 U2 Y22 V2 
            
            *dstYtop_ij = *srctop_ij;               // Y11
            dstYtop_ij++;
            *dstYtop_ij = *(srctop_ij+2);           // Y12
            dstYtop_ij++;
            *dstYbot_ij = *srcbot_ij;               // Y21
            dstYbot_ij++;
            *dstYbot_ij = *(srcbot_ij+2);           // Y22
            dstYbot_ij++;
            
            // U = (U1 + U2*3)/4
            // 25% upper, 75% lower
            *dstU_ij = INTERPOLATE_BYTES_3_1(*(srcbot_ij+1), *(srctop_ij+1));        
            dstU_ij++;
            // V = (V1 + V2*3)/4. 
            // 25% upper, 75% lower            
            *dstV_ij = INTERPOLATE_BYTES_3_1(*(srcbot_ij+3), *(srctop_ij+3));      
            dstV_ij++;
            
            srctop_ij+=4;                           // Move by 1 macropixel YUYV
            srcbot_ij+=4;                           // Move by 1 macropixel YUYV
        }
        
        // Increment pointers to move two input rows down the field (four rows down the frame)
        src_row += src_pitch4;
        dstY_row += dst_pitch4;        
        dstU_row += dst_pitchUV2;
        dstV_row += dst_pitchUV2;        
    }
   
} 

/********************************
 * Progressive YV12 to YUY2
 ********************************/
void yv12_to_yuy2(const BYTE* srcY, const BYTE* srcU, const BYTE* srcV, int src_rowsize, int src_pitch, int src_pitch_uv, 
                  BYTE* dst, int dst_pitch,
                  int height) 
{

    int src_pitch2 = src_pitch*2;
    int dst_pitch2 = dst_pitch*2;
    
    // Pointers to first byte in row
    BYTE *srcY_row = (BYTE *)srcY;                  // Initialize to first row                        
    BYTE *srcU_row = (BYTE *)srcU;                                          
    BYTE *srcV_row = (BYTE *)srcV;                                              
    BYTE *dst_row = dst;
    
    // No interpolation for first two rows and last two rows
    for (int y=0; y<4; y++)
    {
        switch(y)
        {
            case 1: 
                // Second row
                srcY_row += src_pitch;
                dst_row += dst_pitch;
                break;
            case 2:
                // Second to the last row
                srcY_row = (BYTE *)srcY + (src_pitch*(height-2));
                srcU_row = (BYTE *)srcU + (src_pitch_uv*((height>>1)-1));
                srcV_row = (BYTE *)srcV + (src_pitch_uv*((height>>1)-1));                
                dst_row = (BYTE *)dst + (dst_pitch*(height-2));
                break;
            case 3:
                // Last row
                srcY_row += src_pitch;
                dst_row += dst_pitch;                
                break;
            default:
                // First row
                // Do nothing
                break;
        }
        
        BYTE *srcY_ij = srcY_row;
        BYTE *srcU_ij = srcU_row;
        BYTE *srcV_ij = srcV_row;
        BYTE *dst_ij = dst_row;
        
        // Copy values
        for (int x=0; x<src_rowsize; x+=2)
        {
            (*dst_ij) = (*srcY_ij);         // Y
            dst_ij++;
            srcY_ij++;
            (*dst_ij) = (*srcU_ij);         // U
            dst_ij++;
            srcU_ij++;
            (*dst_ij) = (*srcY_ij);         // Y
            dst_ij++;
            srcY_ij++;
            (*dst_ij) = (*srcV_ij);         // V
            dst_ij++;
            srcV_ij++;            
        }         
    }
    
    // Pointers to first byte in row
    srcY_row = (BYTE *)srcY + src_pitch2;                   // Initialize to third row                                             
    srcU_row = (BYTE *)srcU + src_pitch_uv;                 // Initialize to second row of U                                          
    srcV_row = (BYTE *)srcV+ src_pitch_uv;                  // Initialize to second row of V                               
    dst_row = dst + dst_pitch2;                             // Initialize to third row 
        
    // All rows except first two and last two rows
    for (int y=2; y<(height-2); y+=2)                       // Process two rows at a time
    {
        BYTE *srcYtop_ij = srcY_row;                                         
        BYTE *srcYbot_ij = srcYtop_ij + src_pitch;                                               
        BYTE *srcUtop_ij = srcU_row - src_pitch_uv;                                           
        BYTE *srcUmid_ij = srcUtop_ij + src_pitch_uv;
        BYTE *srcUbot_ij = srcUmid_ij + src_pitch_uv;                                           
        BYTE *srcVtop_ij = srcV_row - src_pitch_uv;                                           
        BYTE *srcVmid_ij = srcVtop_ij + src_pitch_uv;
        BYTE *srcVbot_ij = srcVmid_ij + src_pitch_uv;
        BYTE *dsttop_ij = dst_row;    
        BYTE *dstbot_ij = dsttop_ij + dst_pitch;    
        for (int x=0; x<src_rowsize; x+=2)
        {
            // Process 8 bytes at a time (2 macropixels)
            //    Y11 U1 Y12 V1
            //    Y21 U2 Y22 V2 
            
            (*dsttop_ij) = (*srcYtop_ij);                                       // Y11
            srcYtop_ij++;
            dsttop_ij++;
            
            (*dsttop_ij) = INTERPOLATE_BYTES_3_1(*srcUmid_ij, *srcUtop_ij);     // U1
            srcUtop_ij++;
            dsttop_ij++;
            
            (*dsttop_ij) = (*srcYtop_ij);                                       // Y12
            srcYtop_ij++;
            dsttop_ij++;

            (*dsttop_ij) = INTERPOLATE_BYTES_3_1(*srcVmid_ij, *srcVtop_ij);     // V1
            srcVtop_ij++;
            dsttop_ij++;
            
            (*dstbot_ij) = (*srcYbot_ij);                                       // Y21
            srcYbot_ij++;
            dstbot_ij++;
            
            (*dstbot_ij) = INTERPOLATE_BYTES_3_1(*srcUmid_ij, *srcUbot_ij);     // U2
            srcUbot_ij++;
            dstbot_ij++;

            (*dstbot_ij) = (*srcYbot_ij);                                       // Y22
            srcYbot_ij++;
            dstbot_ij++;

            (*dstbot_ij) = INTERPOLATE_BYTES_3_1(*srcVmid_ij, *srcVbot_ij);     // V2
            srcVbot_ij++;
            dstbot_ij++;

            srcUmid_ij++;
            srcVmid_ij++;            
        }
        
        srcY_row += src_pitch2;
        srcU_row += src_pitch_uv;
        srcV_row += src_pitch_uv;
        dst_row += dst_pitch2;        
    }
}

/********************************
 * Interlaced YV12 to YUY2
 ********************************/
void yv12_i_to_yuy2(const BYTE* srcY, const BYTE* srcU, const BYTE* srcV, int src_rowsize, int src_pitch, int src_pitch_uv, 
                  BYTE* dst, int dst_pitch,
                  int height) 
{

    int src_pitch2 = src_pitch*2;
    int dst_pitch2 = dst_pitch*2;
    int src_pitch4 = src_pitch*4;
    int dst_pitch4 = dst_pitch*4;    
    int src_pitch_uv2 = src_pitch_uv*2;
    
    // Pointers to first byte in row
    BYTE *srcY_row = (BYTE *)srcY;                  // Initialize to first row                        
    BYTE *srcU_row = (BYTE *)srcU;                                          
    BYTE *srcV_row = (BYTE *)srcV;                                              
    BYTE *dst_row = dst;
    
    // No interpolation for first four rows and last four rows
    for (int y=0; y<8; y++)
    {
        switch(y)
        {
            case 1: 
                // Second row of first field. Same chroma as case 0.
                srcY_row += src_pitch2;
                dst_row += dst_pitch2;
                break;
            case 2:
                // First row of second field
                srcY_row -= src_pitch;
                dst_row -= dst_pitch;
                srcU_row += src_pitch_uv;
                srcV_row += src_pitch_uv;                
                break;
            case 3:
                // Second row of second field. Same chroma as case 2.
                srcY_row += src_pitch2;
                dst_row += dst_pitch2;
                break;
            case 4:
                // Second to the last row of first field (height - 4)
                srcY_row = (BYTE *)srcY + (src_pitch*(height-4));
                srcU_row = (BYTE *)srcU + (src_pitch_uv*((height>>1)-2));
                srcV_row = (BYTE *)srcV + (src_pitch_uv*((height>>1)-2));                
                dst_row = (BYTE *)dst + (dst_pitch*(height-4));
                break;
            case 5:
                // Last row of first field. Same chroma as case 4.
                srcY_row += src_pitch2;
                dst_row += dst_pitch2;
                break;
            case 6:
                // Second to the last row of second field (height - 3)
                srcY_row -= src_pitch;
                dst_row -= dst_pitch;
                srcU_row += src_pitch_uv;
                srcV_row += src_pitch_uv;                
                break;
            case 7:
                // Last row of second field. Same chroma as case 6.
                srcY_row += src_pitch2;
                dst_row += dst_pitch2;
                break;                
            default:
                // First row of first field
                // Do nothing
                break;
        }
        
        BYTE *srcY_ij = srcY_row;
        BYTE *srcU_ij = srcU_row;
        BYTE *srcV_ij = srcV_row;
        BYTE *dst_ij = dst_row;
        
        // Copy values
        for (int x=0; x<src_rowsize; x+=2)
        {
            (*dst_ij) = (*srcY_ij);         // Y
            dst_ij++;
            srcY_ij++;
            (*dst_ij) = (*srcU_ij);         // U
            dst_ij++;
            srcU_ij++;
            (*dst_ij) = (*srcY_ij);         // Y
            dst_ij++;
            srcY_ij++;
            (*dst_ij) = (*srcV_ij);         // V
            dst_ij++;
            srcV_ij++;            
        }         
    }
    
    
    /////////////////
    // First field //
    /////////////////
        
    // Pointers to first byte in row
    srcY_row = (BYTE *)srcY + src_pitch4;                   // Initialize to fifth row                                             
    srcU_row = (BYTE *)srcU + src_pitch_uv2;                // Initialize to third row of U                                          
    srcV_row = (BYTE *)srcV + src_pitch_uv2;                // Initialize to third row of V                               
    dst_row = dst + dst_pitch4;                             // Initialize to fifth row 
                               
    // Remaining rows of first field
    for (int y=4; y<(height-4); y+=4)                       // Process two rows of the same field 
    {
        BYTE *srcYtop_ij = srcY_row;                        // Top input row                  
        BYTE *srcYbot_ij = srcYtop_ij + src_pitch2;         // Bottom input row from the same field                                          
        BYTE *srcUtop_ij = srcU_row - src_pitch_uv2;                                           
        BYTE *srcUmid_ij = srcUtop_ij + src_pitch_uv2;
        BYTE *srcUbot_ij = srcUmid_ij + src_pitch_uv2;                                           
        BYTE *srcVtop_ij = srcV_row - src_pitch_uv2;                                           
        BYTE *srcVmid_ij = srcVtop_ij + src_pitch_uv2;
        BYTE *srcVbot_ij = srcVmid_ij + src_pitch_uv2;
        BYTE *dsttop_ij = dst_row;    
        BYTE *dstbot_ij = dsttop_ij + dst_pitch2;    
        for (int x=0; x<src_rowsize; x+=2)
        {
            // Process 8 bytes at a time (2 macropixels)
            //    Y11 U1 Y12 V1
            //    Y21 U2 Y22 V2 
            
            (*dsttop_ij) = (*srcYtop_ij);                                       // Y11
            srcYtop_ij++;
            dsttop_ij++;
            
            (*dsttop_ij) = INTERPOLATE_BYTES_7_1(*srcUmid_ij, *srcUtop_ij);     // U1
            srcUtop_ij++;
            dsttop_ij++;
            
            (*dsttop_ij) = (*srcYtop_ij);                                       // Y12
            srcYtop_ij++;
            dsttop_ij++;

            (*dsttop_ij) = INTERPOLATE_BYTES_7_1(*srcVmid_ij, *srcVtop_ij);     // V1
            srcVtop_ij++;
            dsttop_ij++;
            
            (*dstbot_ij) = (*srcYbot_ij);                                       // Y21
            srcYbot_ij++;
            dstbot_ij++;
            
            (*dstbot_ij) = INTERPOLATE_BYTES_5_3(*srcUmid_ij, *srcUbot_ij);     // U2
            srcUbot_ij++;
            dstbot_ij++;

            (*dstbot_ij) = (*srcYbot_ij);                                       // Y22
            srcYbot_ij++;
            dstbot_ij++;

            (*dstbot_ij) = INTERPOLATE_BYTES_5_3(*srcVmid_ij, *srcVbot_ij);     // V2
            srcVbot_ij++;
            dstbot_ij++;

            srcUmid_ij++;
            srcVmid_ij++;            
        }
        
        srcY_row += src_pitch4;
        srcU_row += src_pitch_uv2;
        srcV_row += src_pitch_uv2;
        dst_row += dst_pitch4;        
    }
    
    //////////////////
    // Second field //
    //////////////////
        
    // Pointers to first byte in row
    srcY_row = (BYTE *)srcY + src_pitch4 + src_pitch;       // Initialize to sixth row                                             
    srcU_row = (BYTE *)srcU + src_pitch_uv2 + src_pitch_uv; // Initialize to fourth row of U                                          
    srcV_row = (BYTE *)srcV + src_pitch_uv2 + src_pitch_uv; // Initialize to fourth row of V                               
    dst_row = dst + dst_pitch4 + dst_pitch;                 // Initialize to sixth row 
                               
    // Remaining rows of first field
    for (int y=5; y<(height-4); y+=4)                       // Process two rows of the same field
    {
        BYTE *srcYtop_ij = srcY_row;                        // Top input row                   
        BYTE *srcYbot_ij = srcYtop_ij + src_pitch2;         // Bottom input row from the same field                                     
        BYTE *srcUtop_ij = srcU_row - src_pitch_uv2;                                           
        BYTE *srcUmid_ij = srcUtop_ij + src_pitch_uv2;
        BYTE *srcUbot_ij = srcUmid_ij + src_pitch_uv2;                                           
        BYTE *srcVtop_ij = srcV_row - src_pitch_uv2;                                           
        BYTE *srcVmid_ij = srcVtop_ij + src_pitch_uv2;
        BYTE *srcVbot_ij = srcVmid_ij + src_pitch_uv2;
        BYTE *dsttop_ij = dst_row;    
        BYTE *dstbot_ij = dsttop_ij + dst_pitch2;    
        for (int x=0; x<src_rowsize; x+=2)
        {
            // Process 8 bytes at a time (2 macropixels)
            //    Y11 U1 Y12 V1
            //    Y21 U2 Y22 V2 
            
            (*dsttop_ij) = (*srcYtop_ij);                                       // Y11
            srcYtop_ij++;
            dsttop_ij++;
            
            (*dsttop_ij) = INTERPOLATE_BYTES_5_3(*srcUmid_ij, *srcUtop_ij);     // U1
            srcUtop_ij++;
            dsttop_ij++;
            
            (*dsttop_ij) = (*srcYtop_ij);                                       // Y12
            srcYtop_ij++;
            dsttop_ij++;

            (*dsttop_ij) = INTERPOLATE_BYTES_5_3(*srcVmid_ij, *srcVtop_ij);     // V1
            srcVtop_ij++;
            dsttop_ij++;
            
            (*dstbot_ij) = (*srcYbot_ij);                                       // Y21
            srcYbot_ij++;
            dstbot_ij++;
            
            (*dstbot_ij) = INTERPOLATE_BYTES_7_1(*srcUmid_ij, *srcUbot_ij);     // U2
            srcUbot_ij++;
            dstbot_ij++;

            (*dstbot_ij) = (*srcYbot_ij);                                       // Y22
            srcYbot_ij++;
            dstbot_ij++;

            (*dstbot_ij) = INTERPOLATE_BYTES_7_1(*srcVmid_ij, *srcVbot_ij);     // V2
            srcVbot_ij++;
            dstbot_ij++;

            srcUmid_ij++;
            srcVmid_ij++;            
        }
        
        srcY_row += src_pitch4;
        srcU_row += src_pitch_uv2;
        srcV_row += src_pitch_uv2;
        dst_row += dst_pitch4;        
    }    
    
}



#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
	
/**************************************************************************
 * WARNING! : These routines are typical of when the compiler forgets to save
 * the ebx register. They have just enough C++ code to cause a slight need for
 * using the ebx register but not enough to stop the optimizer from restructuring
 * the code to avoid using the ebx register. The optimizer having done this 
 * very smugly removes the push ebx from the prologue. It seems totally oblivious
 * that there is still __asm code using the ebx register.
 **************************************************************************/

/*************************************
 * Interlaced YV12 -> YUY2 conversion
 *
 * (c) 2003, Klaus Post.
 *
 * Requires mod 8 pitch.
 *************************************/

__declspec(align(8)) static const __int64 add_ones=0x0101010101010101;


void isse_yv12_i_to_yuy2(const BYTE* srcY, const BYTE* srcU, const BYTE* srcV, int src_rowsize, int src_pitch, int src_pitch_uv, 
                    BYTE* dst, int dst_pitch,
                    int height) {
  const BYTE** srcp= new const BYTE*[3];
  int src_pitch_uv2 = src_pitch_uv*2;
  int src_pitch_uv4 = src_pitch_uv*4;
  int skipnext = 0;

  int dst_pitch2=dst_pitch*2;
  int src_pitch2 = src_pitch*2;

  int dst_pitch4 = dst_pitch*4;
  int src_pitch4 = src_pitch*4;

  
  /**** Do first and last lines - NO interpolation:   *****/
  // MMX loop relies on C-code to adjust the lines for it.
  const BYTE* _srcY=srcY;
  const BYTE* _srcU=srcU;
  const BYTE* _srcV=srcV;
  BYTE* _dst=dst;
//
  for (int i=0;i<8;i++) {
    switch (i) {
    case 1:
      _srcY+=src_pitch2;  // Same chroma as in 0
      _dst+=dst_pitch2;
      break;
    case 2:
      _srcY-=src_pitch;  // Next field
      _dst-=dst_pitch;
      _srcU+=src_pitch_uv;
      _srcV+=src_pitch_uv;
      break;
    case 3:
      _srcY+=src_pitch2;  // Same  chroma as in 2
      _dst+=dst_pitch2;
      break;
    case 4: // Now we process the bottom four lines of the picture. 
      _srcY=srcY+(src_pitch*(height-4));
      _srcU=srcU+(src_pitch_uv*((height>>1)-2));
      _srcV=srcV+(src_pitch_uv*((height>>1)-2));
      _dst = dst+(dst_pitch*(height-4));
      break;
    case 5: // Same chroma as in 4
      _srcY += src_pitch2;
      _dst += dst_pitch2;
      break;
    case 6:  // Next field
      _srcY -= src_pitch;
      _dst -= dst_pitch;
      _srcU+=src_pitch_uv;
      _srcV+=src_pitch_uv;
      break;
    case 7:  // Same chroma as in 6
      _srcY += src_pitch2;
      _dst += dst_pitch2;
      default:  // Nothing, case 0
        break;
    }

    __asm {
	push ebx    // stupid compiler forgets to save ebx!!
    mov edi, [_dst]
    mov eax, [_srcY]
    mov ebx, [_srcU]
    mov ecx, [_srcV]
    mov edx,0
    pxor mm7,mm7
    jmp xloop_test_p
xloop_p:
    movq mm0,[eax]    //Y
      movd mm1,[ebx]  //U
    movq mm3,mm0  
     movd mm2,[ecx]   //V
    punpcklbw mm0,mm7  // Y low
     punpckhbw mm3,mm7   // Y high
    punpcklbw mm1,mm7   // 00uu 00uu
     punpcklbw mm2,mm7   // 00vv 00vv
    movq mm4,mm1
     movq mm5,mm2
    punpcklbw mm1,mm7   // 0000 00uu low
     punpcklbw mm2,mm7   // 0000 00vv low
    punpckhbw mm4,mm7   // 0000 00uu high
     punpckhbw mm5,mm7   // 0000 00vv high
    pslld mm1,8
     pslld mm4,8
    pslld mm2,24
     pslld mm5,24
    por mm0, mm1
     por mm3, mm4
    por mm0, mm2
     por mm3, mm5
    movq [edi],mm0
     movq [edi+8],mm3
    add eax,8
    add ebx,4
    add ecx,4
    add edx,8
    add edi, 16
xloop_test_p:
      cmp edx,[src_rowsize]
      jl xloop_p
	  pop ebx
    }
  }

/****************************************
 * Conversion main loop.
 * The code properly interpolates UV from
 * interlaced material.
 * We process two lines in the same field
 * in the same loop, to avoid reloading
 * chroma each time.
 *****************************************/

  height-=8;

  dst+=dst_pitch4;
  srcY+=src_pitch4;
  srcU+=src_pitch_uv2;
  srcV+=src_pitch_uv2;

  srcp[0] = srcY;
  srcp[1] = srcU-src_pitch_uv2;
  srcp[2] = srcV-src_pitch_uv2;

  int y=0;
  int x=0;
  
  __asm {
  push ebx    // stupid compiler forgets to save ebx!!
    mov esi, [srcp]
    mov edi, [dst]

    mov eax,[esi]
    mov ebx,[esi+4]
    mov ecx,[esi+8]
    mov edx,0
    jmp yloop_test
    align 16
yloop:
    mov edx,0               // x counter
    jmp xloop_test
    align 16
xloop:
    mov edx, src_pitch_uv2
      movq mm6, [add_ones]
    movq mm0,[eax]          // mm0 = Y current line
     pxor mm7,mm7
    movd mm2,[ebx+edx]            // mm2 = U top field
     movd mm3, [ecx+edx]          // mm3 = V top field
    movd mm4,[ebx]            // U prev top field
     movq mm1,mm0             // mm1 = Y current line
    movd mm5,[ecx]            // V prev top field
     pavgb mm4,mm2            // interpolate chroma U 
    pavgb mm5,mm3             // interpolate chroma V
     psubusb mm4, mm6         // Better rounding (thanks trbarry!)
    psubusb mm5, mm6
     pavgb mm4,mm2            // interpolate chroma U 
    pavgb mm5,mm3             // interpolate chroma V    
    punpcklbw mm0,mm7        // Y low
    punpckhbw mm1,mm7         // Y high*
     punpcklbw mm4,mm7        // U 00uu 00uu 00uu 00uu
    punpcklbw mm5,mm7         // V 00vv 00vv 00vv 00vv
     pxor mm6,mm6
    punpcklbw mm6,mm4         // U 0000 uu00 0000 uu00 (low)
     punpckhbw mm7,mm4         // V 0000 uu00 0000 uu00 (high
    por mm0,mm6
     por mm1,mm7
    movq mm6,mm5
     punpcklbw mm5,mm5          // V 0000 vvvv 0000 vvvv (low)
    punpckhbw mm6,mm6           // V 0000 vvvv 0000 vvvv (high)
     pslld mm5,24
    pslld mm6,24
     por mm0,mm5
    por mm1,mm6
    mov edx, src_pitch_uv4
     movq [edi],mm0
    movq [edi+8],mm1

    //Next line in same field
     movq mm6, [add_ones]     
    movd mm4,[ebx+edx]        // U next top field
     movd mm5,[ecx+edx]       // V prev top field
    mov edx, [src_pitch2]
     movq mm0,[eax+edx]        // Next Y-line
    pavgb mm4,mm2            // interpolate chroma U
     pavgb mm5,mm3             // interpolate chroma V
    psubusb mm4, mm6         // Better rounding (thanks trbarry!)
     psubusb mm5, mm6
    pavgb mm4,mm2            // interpolate chroma U
     pavgb mm5,mm3             // interpolate chroma V
    pxor mm7,mm7
    movq mm1,mm0             // mm1 = Y current line
     punpcklbw mm0,mm7        // Y low
    punpckhbw mm1,mm7         // Y high*
     punpcklbw mm4,mm7        // U 00uu 00uu 00uu 00uu
    punpcklbw mm5,mm7         // V 00vv 00vv 00vv 00vv
     pxor mm6,mm6
    punpcklbw mm6,mm4         // U 0000 uu00 0000 uu00 (low)
     punpckhbw mm7,mm4         // V 0000 uu00 0000 uu00 (high
    por mm0,mm6
     por mm1,mm7
    movq mm6,mm5
     punpcklbw mm5,mm5          // V 0000 vvvv 0000 vvvv (low)
    punpckhbw mm6,mm6           // V 0000 vvvv 0000 vvvv (high)
     pslld mm5,24
    mov edx,[dst_pitch2]
    pslld mm6,24
     por mm0,mm5
    por mm1,mm6
     movq [edi+edx],mm0
    movq [edi+edx+8],mm1
     add edi,16
    mov edx, [x]
     add eax, 8
    add ebx, 4
     add edx, 8
    add ecx, 4
xloop_test:
    cmp edx,[src_rowsize]
    mov x,edx
    jl xloop
    mov edi, dst
    mov eax,[esi]
    mov ebx,[esi+4]
    mov ecx,[esi+8]

    mov edx,skipnext
    cmp edx,1
    je dont_skip
    add edi,[dst_pitch]
    add eax,[src_pitch]
    add ebx,[src_pitch_uv]
    add ecx,[src_pitch_uv]
    mov [skipnext],1
    jmp yloop  // Never out of loop, if not skip
    align 16
dont_skip:
    add edi,[dst_pitch4]
    add eax,[src_pitch4]
    add ebx,[src_pitch_uv2]
    add ecx,[src_pitch_uv2]
    mov [skipnext],0
    mov edx, [y]
    mov [esi],eax
    mov [esi+4],ebx
    mov [esi+8],ecx
    mov [dst],edi
    add edx, 4

yloop_test:
    cmp edx,[height]
    mov [y],edx
    jl yloop
    sfence
    emms
    pop ebx
  }
   delete[] srcp;
}


/*************************************
 * Progessive YV12 -> YUY2 conversion
 *
 * (c) 2003, Klaus Post.
 *
 * Requires mod 8 pitch.
 *************************************/


void isse_yv12_to_yuy2(const BYTE* srcY, const BYTE* srcU, const BYTE* srcV, int src_rowsize, int src_pitch, int src_pitch_uv, 
                    BYTE* dst, int dst_pitch,
                    int height) {
  const BYTE** srcp= new const BYTE*[3];
  int src_pitch_uv2 = src_pitch_uv*2;
//  int src_pitch_uv4 = src_pitch_uv*4;
  int skipnext = 0;

  int dst_pitch2=dst_pitch*2;
  int src_pitch2 = src_pitch*2;

  
  /**** Do first and last lines - NO interpolation:   *****/
  // MMX loop relies on C-code to adjust the lines for it.
  const BYTE* _srcY=srcY;
  const BYTE* _srcU=srcU;
  const BYTE* _srcV=srcV;
  BYTE* _dst=dst;

  for (int i=0;i<4;i++) {
    switch (i) {
    case 1:
      _srcY+=src_pitch;  // Same chroma as in 0
      _dst+=dst_pitch;
      break;
    case 2:
      _srcY=srcY+(src_pitch*(height-2));
      _srcU=srcU+(src_pitch_uv*((height>>1)-1));
      _srcV=srcV+(src_pitch_uv*((height>>1)-1));
      _dst = dst+(dst_pitch*(height-2));
      break;
    case 3: // Same chroma as in 4
      _srcY += src_pitch;
      _dst += dst_pitch;
      break;
    default:  // Nothing, case 0
        break;
    }

    __asm {
	push ebx    // stupid compiler forgets to save ebx!!
    mov edi, [_dst]
    mov eax, [_srcY]
    mov ebx, [_srcU]
    mov ecx, [_srcV]
    mov edx,0
    pxor mm7,mm7
    jmp xloop_test_p
xloop_p:
    movq mm0,[eax]    //Y
      movd mm1,[ebx]  //U
    movq mm3,mm0  
     movd mm2,[ecx]   //V
    punpcklbw mm0,mm7  // Y low
     punpckhbw mm3,mm7   // Y high
    punpcklbw mm1,mm7   // 00uu 00uu
     punpcklbw mm2,mm7   // 00vv 00vv
    movq mm4,mm1
     movq mm5,mm2
    punpcklbw mm1,mm7   // 0000 00uu low
     punpcklbw mm2,mm7   // 0000 00vv low
    punpckhbw mm4,mm7   // 0000 00uu high
     punpckhbw mm5,mm7   // 0000 00vv high
    pslld mm1,8
     pslld mm4,8
    pslld mm2,24
     pslld mm5,24
    por mm0, mm1
     por mm3, mm4
    por mm0, mm2
     por mm3, mm5
    movq [edi],mm0
     movq [edi+8],mm3
    add eax,8
    add ebx,4
    add ecx,4
    add edx,8
    add edi, 16
xloop_test_p:
      cmp edx,[src_rowsize]
      jl xloop_p
	  pop ebx
    }
  }

/****************************************
 * Conversion main loop.
 * The code properly interpolates UV from
 * interlaced material.
 * We process two lines in the same field
 * in the same loop, to avoid reloading
 * chroma each time.
 *****************************************/

  height-=4;

  dst+=dst_pitch2;
  srcY+=src_pitch2;
  srcU+=src_pitch_uv;
  srcV+=src_pitch_uv;

  srcp[0] = srcY;
  srcp[1] = srcU-src_pitch_uv;
  srcp[2] = srcV-src_pitch_uv;

  int y=0;
  int x=0;

  __asm {
  push ebx    // stupid compiler forgets to save ebx!!
    mov esi, [srcp]
    mov edi, [dst]

    mov eax,[esi]
    mov ebx,[esi+4]
    mov ecx,[esi+8]
    mov edx,0
    jmp yloop_test
    align 16
yloop:
    mov edx,0               // x counter
    jmp xloop_test
    align 16
xloop:
    movq mm6,[add_ones]
    mov edx, src_pitch_uv
    movq mm0,[eax]          // mm0 = Y current line
     pxor mm7,mm7
    movd mm2,[ebx+edx]            // mm2 = U top field
     movd mm3, [ecx+edx]          // mm3 = V top field
    movd mm4,[ebx]        // U prev top field
     movq mm1,mm0             // mm1 = Y current line
    movd mm5,[ecx]        // V prev top field
     pavgb mm4,mm2            // interpolate chroma U  (25/75)
    pavgb mm5,mm3             // interpolate chroma V  (25/75)
     psubusb mm4, mm6         // Better rounding (thanks trbarry!)
    psubusb mm5, mm6
     pavgb mm4,mm2            // interpolate chroma U 
    pavgb mm5,mm3             // interpolate chroma V
     punpcklbw mm0,mm7        // Y low
    punpckhbw mm1,mm7         // Y high*
     punpcklbw mm4,mm7        // U 00uu 00uu 00uu 00uu
    punpcklbw mm5,mm7         // V 00vv 00vv 00vv 00vv
     pxor mm6,mm6
    punpcklbw mm6,mm4         // U 0000 uu00 0000 uu00 (low)
     punpckhbw mm7,mm4         // V 0000 uu00 0000 uu00 (high
    por mm0,mm6
     por mm1,mm7
    movq mm6,mm5
     punpcklbw mm5,mm5          // V 0000 vvvv 0000 vvvv (low)
    punpckhbw mm6,mm6           // V 0000 vvvv 0000 vvvv (high)
     pslld mm5,24
    pslld mm6,24
     por mm0,mm5
    por mm1,mm6
    mov edx, src_pitch_uv2
     movq [edi],mm0
    movq [edi+8],mm1

    //Next line
     
     movq mm6,[add_ones]
    movd mm4,[ebx+edx]        // U next top field
     movd mm5,[ecx+edx]       // V prev top field
    mov edx, [src_pitch]
     pxor mm7,mm7
    movq mm0,[eax+edx]        // Next U-line
     pavgb mm4,mm2            // interpolate chroma U 
    movq mm1,mm0             // mm1 = Y current line
    pavgb mm5,mm3             // interpolate chroma V
     psubusb mm4, mm6         // Better rounding (thanks trbarry!)
    psubusb mm5, mm6
     pavgb mm4,mm2            // interpolate chroma U 
    pavgb mm5,mm3             // interpolate chroma V
     punpcklbw mm0,mm7        // Y low
    punpckhbw mm1,mm7         // Y high*
     punpcklbw mm4,mm7        // U 00uu 00uu 00uu 00uu
    punpcklbw mm5,mm7         // V 00vv 00vv 00vv 00vv
     pxor mm6,mm6
    punpcklbw mm6,mm4         // U 0000 uu00 0000 uu00 (low)
     punpckhbw mm7,mm4         // V 0000 uu00 0000 uu00 (high
    por mm0,mm6
     por mm1,mm7
    movq mm6,mm5
     punpcklbw mm5,mm5          // V 0000 vvvv 0000 vvvv (low)
    punpckhbw mm6,mm6           // V 0000 vvvv 0000 vvvv (high)
     pslld mm5,24
    mov edx,[dst_pitch]
    pslld mm6,24
     por mm0,mm5
    por mm1,mm6
     movq [edi+edx],mm0
    movq [edi+edx+8],mm1
     add edi,16
    mov edx, [x]
     add eax, 8
    add ebx, 4
     add edx, 8
    add ecx, 4
xloop_test:
    cmp edx,[src_rowsize]
    mov x,edx
    jl xloop
    mov edi, dst
    mov eax,[esi]
    mov ebx,[esi+4]
    mov ecx,[esi+8]

    add edi,[dst_pitch2]
    add eax,[src_pitch2]
    add ebx,[src_pitch_uv]
    add ecx,[src_pitch_uv]
    mov edx, [y]
    mov [esi],eax
    mov [esi+4],ebx
    mov [esi+8],ecx
    mov [dst],edi
    add edx, 2

yloop_test:
    cmp edx,[height]
    mov [y],edx
    jl yloop
    sfence
    emms
    pop ebx
  }
   delete[] srcp;
}


/*************************************
 * Interlaced YV12 -> YUY2 conversion
 *
 * (c) 2003, Klaus Post.
 *
 * Requires mod 8 rowsize.
 * MMX version.
 *************************************/


void mmx_yv12_i_to_yuy2(const BYTE* srcY, const BYTE* srcU, const BYTE* srcV, int src_rowsize, int src_pitch, int src_pitch_uv, 
                    BYTE* dst, int dst_pitch,
                    int height) {
  __declspec(align(8)) static __int64 add_64=0x0002000200020002;
  const BYTE** srcp= new const BYTE*[3];
  int src_pitch_uv2 = src_pitch_uv*2;
  int src_pitch_uv4 = src_pitch_uv*4;
  int skipnext = 0;

  int dst_pitch2=dst_pitch*2;
  int src_pitch2 = src_pitch*2;

  int dst_pitch4 = dst_pitch*4;
  int src_pitch4 = src_pitch*4;

  
  /**** Do first and last lines - NO interpolation:   *****/
  // MMX loop relies on C-code to adjust the lines for it.
  const BYTE* _srcY=srcY;
  const BYTE* _srcU=srcU;
  const BYTE* _srcV=srcV;
  BYTE* _dst=dst;

  for (int i=0;i<8;i++) {
    switch (i) {
    case 1:
      _srcY+=src_pitch2;  // Same chroma as in 0
      _dst+=dst_pitch2;
      break;
    case 2:
      _srcY-=src_pitch;  // Next field
      _dst-=dst_pitch;
      _srcU+=src_pitch_uv;
      _srcV+=src_pitch_uv;
      break;
    case 3:
      _srcY+=src_pitch2;  // Same  chroma as in 2
      _dst+=dst_pitch2;
      break;
    case 4:
      _srcY=srcY+(src_pitch*(height-4));
      _srcU=srcU+(src_pitch_uv*((height>>1)-2));
      _srcV=srcV+(src_pitch_uv*((height>>1)-2));
      _dst = dst+(dst_pitch*(height-4));
      break;
    case 5: // Same chroma as in 4
      _srcY += src_pitch2;
      _dst += dst_pitch2;
      break;
    case 6:  // Next field
      _srcY -= src_pitch;
      _dst -= dst_pitch;
      _srcU+=src_pitch_uv;
      _srcV+=src_pitch_uv;
      break;
    case 7:  // Same chroma as in 6
      _srcY += src_pitch2;
      _dst += dst_pitch2;
      default:  // Nothing, case 0
        break;
    }

    __asm {
	push ebx    // stupid compiler forgets to save ebx!!
    mov edi, [_dst]
    mov eax, [_srcY]
    mov ebx, [_srcU]
    mov ecx, [_srcV]
    mov edx,0
    pxor mm7,mm7
    jmp xloop_test_p
xloop_p:
    movq mm0,[eax]    //Y
      movd mm1,[ebx]  //U
    movq mm3,mm0  
     movd mm2,[ecx]   //V
    punpcklbw mm0,mm7  // Y low
     punpckhbw mm3,mm7   // Y high
    punpcklbw mm1,mm7   // 00uu 00uu
     punpcklbw mm2,mm7   // 00vv 00vv
    movq mm4,mm1
     movq mm5,mm2
    punpcklbw mm1,mm7   // 0000 00uu low
     punpcklbw mm2,mm7   // 0000 00vv low
    punpckhbw mm4,mm7   // 0000 00uu high
     punpckhbw mm5,mm7   // 0000 00vv high
    pslld mm1,8
     pslld mm4,8
    pslld mm2,24
     pslld mm5,24
    por mm0, mm1
     por mm3, mm4
    por mm0, mm2
     por mm3, mm5
    movq [edi],mm0
     movq [edi+8],mm3
    add eax,8
    add ebx,4
    add ecx,4
    add edx,8
    add edi, 16
xloop_test_p:
      cmp edx,[src_rowsize]
      jl xloop_p
	  pop ebx
    }
  }

/****************************************
 * Conversion main loop.
 * The code properly interpolates UV from
 * interlaced material.
 * We process two lines in the same field
 * in the same loop, to avoid reloading
 * chroma each time.
 *****************************************/

  height-=8;

  dst+=dst_pitch4;
  srcY+=src_pitch4;
  srcU+=src_pitch_uv2;
  srcV+=src_pitch_uv2;

  srcp[0] = srcY;
  srcp[1] = srcU-src_pitch_uv2;
  srcp[2] = srcV-src_pitch_uv2;

  int y=0;
  int x=0;

  __asm {
  push ebx    // stupid compiler forgets to save ebx!!
    mov esi, [srcp]
    mov edi, [dst]

    mov eax,[esi]
    mov ebx,[esi+4]
    mov ecx,[esi+8]
    mov edx,0
    jmp yloop_test
    align 16
yloop:
    mov edx,0               // x counter
    jmp xloop_test
    align 16
xloop:
    mov edx, src_pitch_uv2
    movq mm0,[eax]          // mm0 = Y current line
     pxor mm7,mm7
    movd mm2,[ebx+edx]            // mm2 = U top field
     movd mm3, [ecx+edx]          // mm3 = V top field
    movd mm4,[ebx]        // U prev top field
     movq mm1,mm0             // mm1 = Y current line
    movd mm5,[ecx]        // V prev top field

    punpcklbw mm2,mm7        // U 00uu 00uu 00uu 00uu
     punpcklbw mm3,mm7         // V 00vv 00vv 00vv 00vv
    punpcklbw mm4,mm7        // U 00uu 00uu 00uu 00uu
     punpcklbw mm5,mm7         // V 00vv 00vv 00vv 00vv
    paddusw mm4,mm2
     paddusw mm5,mm3
    paddusw mm4,mm2
     paddusw mm5,mm3
    paddusw mm4,mm2
     paddusw mm5,mm3
    paddusw mm4, [add_64]
     paddusw mm5, [add_64]
    psrlw mm4,2
     pxor mm7,mm7
    psrlw mm5,2

     punpcklbw mm0,mm7        // Y low
    punpckhbw mm1,mm7         // Y high*
     pxor mm6,mm6
    punpcklbw mm6,mm4         // U 0000 uu00 0000 uu00 (low)
     punpckhbw mm7,mm4         // V 0000 uu00 0000 uu00 (high
    por mm0,mm6
     por mm1,mm7
    movq mm6,mm5
     punpcklbw mm5,mm5          // V 0000 vvvv 0000 vvvv (low)
    punpckhbw mm6,mm6           // V 0000 vvvv 0000 vvvv (high)
     pslld mm5,24
    pslld mm6,24
     por mm0,mm5
    por mm1,mm6
    mov edx, src_pitch_uv4
     movq [edi],mm0
    movq [edi+8],mm1

    //Next line in same field
     
    movd mm4,[ebx+edx]        // U next top field
     movd mm5,[ecx+edx]       // V prev top field
    mov edx, [src_pitch2]
     pxor mm7,mm7
    movq mm0,[eax+edx]        // Next U-line
    movq mm1,mm0             // mm1 = Y current line

    punpcklbw mm4,mm7        // U 00uu 00uu 00uu 00uu
     punpcklbw mm5,mm7         // V 00vv 00vv 00vv 00vv
    paddusw mm4,mm2
     paddusw mm5,mm3
    paddusw mm4,mm2
     paddusw mm5,mm3
    paddusw mm4,mm2
     paddusw mm5,mm3
    paddusw mm4, [add_64]
     paddusw mm5, [add_64]
    psrlw mm4,2
     pxor mm7,mm7
    psrlw mm5,2

     punpcklbw mm0,mm7        // Y low
    punpckhbw mm1,mm7         // Y high*
     pxor mm6,mm6
    punpcklbw mm6,mm4         // U 0000 uu00 0000 uu00 (low)
     punpckhbw mm7,mm4         // V 0000 uu00 0000 uu00 (high
    por mm0,mm6
     por mm1,mm7
    movq mm6,mm5
     punpcklbw mm5,mm5          // V 0000 vvvv 0000 vvvv (low)
    punpckhbw mm6,mm6           // V 0000 vvvv 0000 vvvv (high)
     pslld mm5,24
    mov edx,[dst_pitch2]
    pslld mm6,24
     por mm0,mm5
    por mm1,mm6
     movq [edi+edx],mm0
    movq [edi+edx+8],mm1
     add edi,16
    mov edx, [x]
     add eax, 8
    add ebx, 4
     add edx, 8
    add ecx, 4
xloop_test:
    cmp edx,[src_rowsize]
    mov x,edx
    jl xloop
    mov edi, dst
    mov eax,[esi]
    mov ebx,[esi+4]
    mov ecx,[esi+8]

    mov edx,skipnext
    cmp edx,1
    je dont_skip
    add edi,[dst_pitch]
    add eax,[src_pitch]
    add ebx,[src_pitch_uv]
    add ecx,[src_pitch_uv]
    mov [skipnext],1
    jmp yloop  // Never out of loop, if not skip
    align 16
dont_skip:
    add edi,[dst_pitch4]
    add eax,[src_pitch4]
    add ebx,[src_pitch_uv2]
    add ecx,[src_pitch_uv2]
    mov [skipnext],0
    mov edx, [y]
    mov [esi],eax
    mov [esi+4],ebx
    mov [esi+8],ecx
    mov [dst],edi
    add edx, 4

yloop_test:
    cmp edx,[height]
    mov [y],edx
    jl yloop
    emms
    pop ebx
  }
   delete[] srcp;
}

/*************************************
 * Progressive YV12 -> YUY2 conversion
 *
 * (c) 2003, Klaus Post.
 *
 * Requires mod 8 rowsize.
 * MMX version.
 *************************************/


void mmx_yv12_to_yuy2(const BYTE* srcY, const BYTE* srcU, const BYTE* srcV, int src_rowsize, int src_pitch, int src_pitch_uv, 
                    BYTE* dst, int dst_pitch,
                    int height) {
  __declspec(align(8)) static __int64 add_64=0x0002000200020002;
  const BYTE** srcp= new const BYTE*[3];
  int src_pitch_uv2 = src_pitch_uv*2;
  int skipnext = 0;

  int dst_pitch2=dst_pitch*2;
  int src_pitch2 = src_pitch*2;


  
  /**** Do first and last lines - NO interpolation:   *****/
  // MMX loop relies on C-code to adjust the lines for it.
  const BYTE* _srcY=srcY;
  const BYTE* _srcU=srcU;
  const BYTE* _srcV=srcV;
  BYTE* _dst=dst;

  for (int i=0;i<4;i++) {
    switch (i) {
    case 1:
      _srcY+=src_pitch;  // Same chroma as in 0
      _dst+=dst_pitch;
      break;
    case 2:
      _srcY=srcY+(src_pitch*(height-2));
      _srcU=srcU+(src_pitch_uv*((height>>1)-1));
      _srcV=srcV+(src_pitch_uv*((height>>1)-1));
      _dst = dst+(dst_pitch*(height-2));
      break;
    case 3: // Same chroma as in 4
      _srcY += src_pitch;
      _dst += dst_pitch;
      break;
    default:  // Nothing, case 0
        break;
    }


    __asm {
	push ebx    // stupid compiler forgets to save ebx!!
    mov edi, [_dst]
    mov eax, [_srcY]
    mov ebx, [_srcU]
    mov ecx, [_srcV]
    mov edx,0
    pxor mm7,mm7
    jmp xloop_test_p
xloop_p:
    movq mm0,[eax]    //Y
      movd mm1,[ebx]  //U
    movq mm3,mm0  
     movd mm2,[ecx]   //V
    punpcklbw mm0,mm7  // Y low
     punpckhbw mm3,mm7   // Y high
    punpcklbw mm1,mm7   // 00uu 00uu
     punpcklbw mm2,mm7   // 00vv 00vv
    movq mm4,mm1
     movq mm5,mm2
    punpcklbw mm1,mm7   // 0000 00uu low
     punpcklbw mm2,mm7   // 0000 00vv low
    punpckhbw mm4,mm7   // 0000 00uu high
     punpckhbw mm5,mm7   // 0000 00vv high
    pslld mm1,8
     pslld mm4,8
    pslld mm2,24
     pslld mm5,24
    por mm0, mm1
     por mm3, mm4
    por mm0, mm2
     por mm3, mm5
    movq [edi],mm0
     movq [edi+8],mm3
    add eax,8
    add ebx,4
    add ecx,4
    add edx,8
    add edi, 16
xloop_test_p:
      cmp edx,[src_rowsize]
      jl xloop_p
	  pop ebx
    }
  }

/****************************************
 * Conversion main loop.
 * The code properly interpolates UV from
 * interlaced material.
 * We process two lines in the same field
 * in the same loop, to avoid reloading
 * chroma each time.
 *****************************************/

  height-=4;

  dst+=dst_pitch2;
  srcY+=src_pitch2;
  srcU+=src_pitch_uv;
  srcV+=src_pitch_uv;

  srcp[0] = srcY;
  srcp[1] = srcU-src_pitch_uv;
  srcp[2] = srcV-src_pitch_uv;

  int y=0;
  int x=0;

  __asm {
  push ebx    // stupid compiler forgets to save ebx!!
    mov esi, [srcp]
    mov edi, [dst]

    mov eax,[esi]
    mov ebx,[esi+4]
    mov ecx,[esi+8]
    mov edx,0
    jmp yloop_test
    align 16
yloop:
    mov edx,0               // x counter
    jmp xloop_test
    align 16
xloop:
    mov edx, src_pitch_uv
    movq mm0,[eax]          // mm0 = Y current line
     pxor mm7,mm7
    movd mm2,[ebx+edx]            // mm2 = U top field
     movd mm3, [ecx+edx]          // mm3 = V top field
    movd mm4,[ebx]        // U prev top field
     movq mm1,mm0             // mm1 = Y current line
    movd mm5,[ecx]        // V prev top field

    punpcklbw mm2,mm7        // U 00uu 00uu 00uu 00uu
     punpcklbw mm3,mm7         // V 00vv 00vv 00vv 00vv
    punpcklbw mm4,mm7        // U 00uu 00uu 00uu 00uu
     punpcklbw mm5,mm7         // V 00vv 00vv 00vv 00vv
    paddusw mm4,mm2
     paddusw mm5,mm3
    paddusw mm4,mm2
     paddusw mm5,mm3
    paddusw mm4,mm2
     paddusw mm5,mm3
    paddusw mm4, [add_64]
     paddusw mm5, [add_64]
    psrlw mm4,2
     psrlw mm5,2


     punpcklbw mm0,mm7        // Y low
    punpckhbw mm1,mm7         // Y high*
     pxor mm6,mm6
    punpcklbw mm6,mm4         // U 0000 uu00 0000 uu00 (low)
     punpckhbw mm7,mm4         // V 0000 uu00 0000 uu00 (high
    por mm0,mm6
     por mm1,mm7
    movq mm6,mm5
     punpcklbw mm5,mm5          // V 0000 vvvv 0000 vvvv (low)
    punpckhbw mm6,mm6           // V 0000 vvvv 0000 vvvv (high)
     pslld mm5,24
    pslld mm6,24
     por mm0,mm5
    por mm1,mm6
    mov edx, src_pitch_uv2
     movq [edi],mm0
    movq [edi+8],mm1

    //Next line 
     
    movd mm4,[ebx+edx]        // U next top field
     movd mm5,[ecx+edx]       // V prev top field
    mov edx, [src_pitch]
     pxor mm7,mm7
    movq mm0,[eax+edx]        // Next U-line
    movq mm1,mm0             // mm1 = Y current line

    punpcklbw mm4,mm7        // U 00uu 00uu 00uu 00uu
     punpcklbw mm5,mm7         // V 00vv 00vv 00vv 00vv
    paddusw mm4,mm2
     paddusw mm5,mm3
    paddusw mm4,mm2
     paddusw mm5,mm3
    paddusw mm4,mm2
     paddusw mm5,mm3
    paddusw mm4, [add_64]
     paddusw mm5, [add_64]
    psrlw mm4,2
     psrlw mm5,2

     punpcklbw mm0,mm7        // Y low
    punpckhbw mm1,mm7         // Y high*
     pxor mm6,mm6
    punpcklbw mm6,mm4         // U 0000 uu00 0000 uu00 (low)
     punpckhbw mm7,mm4         // V 0000 uu00 0000 uu00 (high
    por mm0,mm6
     por mm1,mm7
    movq mm6,mm5
     punpcklbw mm5,mm5          // V 0000 vvvv 0000 vvvv (low)
    punpckhbw mm6,mm6           // V 0000 vvvv 0000 vvvv (high)
     pslld mm5,24
    mov edx,[dst_pitch]
    pslld mm6,24
     por mm0,mm5
    por mm1,mm6
     movq [edi+edx],mm0
    movq [edi+edx+8],mm1
     add edi,16
    mov edx, [x]
     add eax, 8
    add ebx, 4
     add edx, 8
    add ecx, 4
xloop_test:
    cmp edx,[src_rowsize]
    mov x,edx
    jl xloop
    mov edi, dst
    mov eax,[esi]
    mov ebx,[esi+4]
    mov ecx,[esi+8]

    add edi,[dst_pitch2]
    add eax,[src_pitch2]
    add ebx,[src_pitch_uv]
    add ecx,[src_pitch_uv]
    mov edx, [y]
    mov [esi],eax
    mov [esi+4],ebx
    mov [esi+8],ecx
    mov [dst],edi
    add edx, 2

yloop_test:
    cmp edx,[height]
    mov [y],edx
    jl yloop
    emms
    pop ebx
  }
   delete[] srcp;
}


/********************************
 * Progressive YUY2 to YV12
 * 
 * (c) Copyright 2003, Klaus Post
 *
 * Converts 8x2 (8 pixels, two lines) in parallel.
 * Requires mod8 pitch for output, and mod16 pitch for input.
 ********************************/

void isse_yuy2_to_yv12(const BYTE* src, int src_rowsize, int src_pitch, 
                    BYTE* dstY, BYTE* dstU, BYTE* dstV, int dst_pitchY, int dst_pitchUV,
                    int height) {

__declspec(align(8)) static __int64 mask1	= 0x00ff00ff00ff00ff;
__declspec(align(8)) static __int64 mask2	= 0xff00ff00ff00ff00;

  const BYTE** dstp= new const BYTE*[4];
  dstp[0]=dstY;
  dstp[1]=dstY+dst_pitchY;
  dstp[2]=dstU;
  dstp[3]=dstV;
  int src_pitch2 = src_pitch*2;
  int dst_pitch2 = dst_pitchY*2;

  int y=0;
  int x=0;
  src_rowsize = (src_rowsize+3)/4;
  __asm {
  push ebx    // stupid compiler forgets to save ebx!!
    movq mm7,[mask2]
    movq mm4,[mask1]
    mov edx,0
    mov esi, src
    mov edi, dstp
    jmp yloop_test
    align 16
yloop:
      mov edx,0               // x counter   
      mov eax, [src_pitch]
      jmp xloop_test
      align 16
xloop:      
      movq mm0,[esi]        // YUY2 upper line  (4 pixels luma, 2 chroma)
       movq mm1,[esi+eax]   // YUY2 lower line  
      movq mm6,mm0
       movq mm2, [esi+8]    // Load second pair
      movq mm3, [esi+eax+8]
       movq mm5,mm2
      pavgb mm6,mm1         // Average (chroma)
       pavgb mm5,mm3        // Average Chroma (second pair)
      pand mm0,mm4          // Mask luma
  	    psrlq mm5, 8
      pand mm1,mm4          // Mask luma
 	     psrlq mm6, 8
      pand mm2,mm4          // Mask luma
       pand mm3,mm4         
      pand mm5,mm4           // Mask chroma
       pand mm6,mm4          // Mask chroma
   		packuswb mm0, mm2     // Pack luma (upper)
   		 packuswb mm6, mm5    // Pack chroma
   		packuswb mm1, mm3     // Pack luma (lower)     
       movq mm5, mm6        // Chroma copy
      pand mm5, mm7         // Mask V
       pand mm6, mm4        // Mask U
      psrlq mm5,8            // shift down V
   		 packuswb mm5, mm7     // Pack U 
   		packuswb mm6, mm7     // Pack V 
       mov ebx, [edi]
      mov ecx, [edi+4]
      movq [ebx+edx*2],mm0
       movq [ecx+edx*2],mm1

      mov ecx, [edi+8]
      mov ebx, [edi+12]
       movd [ecx+edx], mm6  // Store U
      movd [ebx+edx], mm5   // Store V
      add esi, 16
      add edx, 4
xloop_test:
    cmp edx,[src_rowsize]
    jl xloop
    mov esi, src
    mov edx,[edi]
    mov ebx,[edi+4]
    mov ecx,[edi+8]
    mov eax,[edi+12]
    
    add edx, [dst_pitch2]
    add ebx, [dst_pitch2]
    add ecx, [dst_pitchUV]
    add eax, [dst_pitchUV]
    add esi, [src_pitch2]

    mov [edi],edx
    mov [edi+4],ebx
    mov [edi+8],ecx
    mov [edi+12],eax
    mov edx, [y]
    mov [src],esi
    
    add edx, 2

yloop_test:
    cmp edx,[height]
    mov [y],edx
    jl yloop
    sfence
    emms
    pop ebx
  }
   delete[] dstp;
}



/********************************
 * Interlaced YUY2 to YV12
 * 
 * (c) Copyright 2003, Klaus Post
 *
 * Converts 8x2 (8 pixels, two lines) in parallel.
 * Requires mod8 pitch for output, and mod16 pitch for input.
 ********************************/


void isse_yuy2_i_to_yv12(const BYTE* src, int src_rowsize, int src_pitch, 
                    BYTE* dstY, BYTE* dstU, BYTE* dstV, int dst_pitchY, int dst_pitchUV,
                    int height) {

__declspec(align(8)) static __int64 mask1	= 0x00ff00ff00ff00ff;
__declspec(align(8)) static __int64 mask2	= 0xff00ff00ff00ff00;

  const BYTE** dstp= new const BYTE*[4];
  dstp[0]=dstY;
  dstp[1]=dstY+(dst_pitchY*2);
  dstp[2]=dstU;
  dstp[3]=dstV;
  int src_pitch2 = src_pitch*2;
  int dst_pitch2 = dst_pitchY*2;
  int src_pitch4 = src_pitch*4;
  int dst_pitch3 = dst_pitchY*3;

  int y=0;
  int x=0;
  src_rowsize = (src_rowsize+3)/4;
  __asm {
  push ebx    // stupid compiler forgets to save ebx!!
    movq mm7,[mask2]
    movq mm4,[mask1]
    mov edx,0
    mov esi, src
    mov edi, dstp
    jmp yloop_test
    align 16
yloop:
      mov edx,0               // x counter   
      mov eax, [src_pitch2]
      jmp xloop_test
      align 16
xloop:      
      movq mm0,[esi]        // YUY2 upper line  (4 pixels luma, 2 chroma)
       movq mm1,[esi+eax]   // YUY2 lower line  
      movq mm6,mm0
       movq mm2, [esi+8]    // Load second pair
      movq mm3, [esi+eax+8]
       movq mm5,mm2

      pavgb mm6,mm1         // Average (chroma)
       pavgb mm5,mm3        // Average Chroma (second pair)
      psubusb mm5, [add_ones]         // Better rounding (thanks trbarry!)
       psubusb mm6, [add_ones]
      pavgb mm6,mm0         // Average (chroma) (upper = 75% lower = 25%)
       pavgb mm5,mm2        // Average Chroma (second pair) (upper = 75% lower = 25%)

      pand mm0,mm4          // Mask luma
  	    psrlq mm5, 8
      pand mm1,mm4          // Mask luma
 	     psrlq mm6, 8
      pand mm2,mm4          // Mask luma
       pand mm3,mm4         
      pand mm5,mm4           // Mask chroma
       pand mm6,mm4          // Mask chroma
   		packuswb mm0, mm2     // Pack luma (upper)
   		 packuswb mm6, mm5    // Pack chroma
   		packuswb mm1, mm3     // Pack luma (lower)     
       movq mm5, mm6        // Chroma copy
      pand mm5, mm7         // Mask V
       pand mm6, mm4        // Mask U
      psrlq mm5,8            // shift down V
   		 packuswb mm5, mm7     // Pack U 
   		packuswb mm6, mm7     // Pack V 
       mov ebx, [edi]
      mov ecx, [edi+4]
      movq [ebx+edx*2],mm0
       movq [ecx+edx*2],mm1

      mov ecx, [edi+8]
      mov ebx, [edi+12]
       movd [ecx+edx], mm6  // Store U
      movd [ebx+edx], mm5   // Store V
      add esi, 16
      add edx, 4
xloop_test:
    cmp edx,[src_rowsize]
    jl xloop

    mov esi, src
    mov edx,[edi]
    mov ebx,[edi+4]
    mov ecx,[edi+8]
    mov eax,[edi+12]
    
    add edx, [dst_pitchY]
    add ebx, [dst_pitchY]
    add ecx, [dst_pitchUV]
    add eax, [dst_pitchUV]
    add esi, [src_pitch]

    mov [edi],edx
    mov [edi+4],ebx
    mov [edi+8],ecx
    mov [edi+12],eax

    mov edx, 0
    mov eax, [src_pitch2]

    jmp xloop2_test
xloop2:   // Second field
      movq mm0,[esi]        // YUY2 upper line  (4 pixels luma, 2 chroma)
       movq mm1,[esi+eax]   // YUY2 lower line  
      movq mm6,mm0
       movq mm2, [esi+8]    // Load second pair
      movq mm3, [esi+eax+8]
       movq mm5,mm2

      pavgb mm6,mm1         // Average (chroma)
       pavgb mm5,mm3        // Average Chroma (second pair)
      psubusb mm5, [add_ones]         // Better rounding (thanks trbarry!)
       psubusb mm6, [add_ones]
      pavgb mm6,mm1         // Average (chroma) (upper = 25% lower = 75%)
       pavgb mm5,mm3        // Average Chroma (second pair) (upper = 25% lower = 75%)

      pand mm0,mm4          // Mask luma
  	    psrlq mm5, 8
      pand mm1,mm4          // Mask luma
 	     psrlq mm6, 8
      pand mm2,mm4          // Mask luma
       pand mm3,mm4         
      pand mm5,mm4           // Mask chroma
       pand mm6,mm4          // Mask chroma
   		packuswb mm0, mm2     // Pack luma (upper)
   		 packuswb mm6, mm5    // Pack chroma
   		packuswb mm1, mm3     // Pack luma (lower)     
       movq mm5, mm6        // Chroma copy
      pand mm5, mm7         // Mask V
       pand mm6, mm4        // Mask U
      psrlq mm5,8            // shift down V
   		 packuswb mm5, mm7     // Pack U 
   		packuswb mm6, mm7     // Pack V 
       mov ebx, [edi]
      mov ecx, [edi+4]
      movq [ebx+edx*2],mm0
       movq [ecx+edx*2],mm1

      mov ecx, [edi+8]
      mov ebx, [edi+12]
       movd [ecx+edx], mm6  // Store U
      movd [ebx+edx], mm5   // Store V
      add esi, 16
      add edx, 4
xloop2_test:
    cmp edx,[src_rowsize]
    jl xloop2

    mov esi, src
    mov edx,[edi]
    mov ebx,[edi+4]
    mov ecx,[edi+8]
    mov eax,[edi+12]
    
    add edx, [dst_pitch3]
    add ebx, [dst_pitch3]
    add ecx, [dst_pitchUV]
    add eax, [dst_pitchUV]
    add esi, [src_pitch4]

    mov [edi],edx
    mov [edi+4],ebx
    mov [edi+8],ecx
    mov [edi+12],eax
    mov edx, [y]
    mov [src],esi
    
    add edx, 4

yloop_test:
    cmp edx,[height]
    mov [y],edx
    jl yloop
    sfence
    emms
    pop ebx
  }
   delete[] dstp;
}



/********************************
 * Progressive YUY2 to YV12
 * 
 * (c) Copyright 2003, Klaus Post
 *
 * Converts 8x2 (8 pixels, two lines) in parallel.
 * Requires mod8 pitch for output, and mod16 pitch for input.
 * MMX Version (much slower than ISSE!) (used as fallback for ISSE version)
 ********************************/

void mmx_yuy2_to_yv12(const BYTE* src, int src_rowsize, int src_pitch, 
                    BYTE* dstY, BYTE* dstU, BYTE* dstV, int dst_pitchY, int dst_pitchUV,
                    int height) {

__declspec(align(8)) static __int64 mask1	= 0x00ff00ff00ff00ff;
__declspec(align(8)) static __int64 mask2	= 0xff00ff00ff00ff00;
__declspec(align(8)) static __int64 add_1	= 0x0001000100010001;

  const BYTE** dstp= new const BYTE*[4];
  dstp[0]=dstY;
  dstp[1]=dstY+dst_pitchY;
  dstp[2]=dstU;
  dstp[3]=dstV;
  int src_pitch2 = src_pitch*2;
  int dst_pitch2 = dst_pitchY*2;

  int y=0;
  int x=0;
  src_rowsize = (src_rowsize+3)/4;
  __asm {
  push ebx    // stupid compiler forgets to save ebx!!
    mov edx,0
    mov esi, src
    mov edi, dstp
    jmp yloop_test
    align 16
yloop:
      mov edx,0               // x counter   
      mov eax, [src_pitch]
      jmp xloop_test
      align 16
xloop:      
      movq mm0,[esi]        // YUY2 upper line  (4 pixels luma, 2 chroma)
       movq mm1,[esi+eax]   // YUY2 lower line  
      movq mm6,mm0
       movq mm2, [esi+8]    // Load second pair
      movq mm3, [esi+eax+8]
       movq mm5,mm2
      movq mm7, mm1
       movq mm4, mm3
      psrlw mm5,8
       psrlw mm6,8
      psrlw mm4,8
       psrlw mm7,8
      paddw mm5,mm4
       paddw mm6,mm7
      movq mm4,[mask1]
       movq mm7,[mask2]
      paddw mm5,[add_1]
       paddw mm6,[add_1]
      
//      pavgb mm6,mm1         // Average (chroma)
//       pavgb mm5,mm3        // Average Chroma (second pair)

      pand mm0,mm4          // Mask luma
       psrlw mm5,1
      pand mm1,mm4          // Mask luma
       psrlw mm6,1
      pand mm2,mm4          // Mask luma
       pand mm3,mm4         
   		packuswb mm0, mm2     // Pack luma (upper)
   		 packuswb mm6, mm5    // Pack chroma
   		packuswb mm1, mm3     // Pack luma (lower)     
       movq mm5, mm6        // Chroma copy
      pand mm5, mm7         // Mask V
       pand mm6, mm4        // Mask U
      psrlq mm5,8            // shift down V
   		 packuswb mm5, mm7     // Pack U 
   		packuswb mm6, mm7     // Pack V 
       mov ebx, [edi]
      mov ecx, [edi+4]
      movq [ebx+edx*2],mm0
       movq [ecx+edx*2],mm1

      mov ecx, [edi+8]
      mov ebx, [edi+12]
       movd [ecx+edx], mm6  // Store U
      movd [ebx+edx], mm5   // Store V
      add esi, 16
      add edx, 4
xloop_test:
    cmp edx,[src_rowsize]
    jl xloop
    mov esi, src
    mov edx,[edi]
    mov ebx,[edi+4]
    mov ecx,[edi+8]
    mov eax,[edi+12]
    
    add edx, [dst_pitch2]
    add ebx, [dst_pitch2]
    add ecx, [dst_pitchUV]
    add eax, [dst_pitchUV]
    add esi, [src_pitch2]

    mov [edi],edx
    mov [edi+4],ebx
    mov [edi+8],ecx
    mov [edi+12],eax
    mov edx, [y]
    mov [src],esi
    
    add edx, 2

yloop_test:
    cmp edx,[height]
    mov [y],edx
    jl yloop
    emms
    pop ebx
  }
   delete[] dstp;
}



/********************************
 * Interlaced YUY2 to YV12
 * 
 * (c) Copyright 2003, Klaus Post
 *
 * Converts 8x2 (8 pixels, two lines) in parallel.
 * Requires mod8 pitch for output, and mod16 pitch for input.
 * MMX version (used as fallback for ISSE version)
 ********************************/


void mmx_yuy2_i_to_yv12(const BYTE* src, int src_rowsize, int src_pitch, 
                    BYTE* dstY, BYTE* dstU, BYTE* dstV, int dst_pitchY, int dst_pitchUV,
                    int height) {

__declspec(align(8)) static __int64 mask1	= 0x00ff00ff00ff00ff;
__declspec(align(8)) static __int64 mask2	= 0xff00ff00ff00ff00;
__declspec(align(8)) static __int64 add_2	= 0x0002000200020002;

  const BYTE** dstp= new const BYTE*[4];
  dstp[0]=dstY;
  dstp[1]=dstY+(dst_pitchY*2);
  dstp[2]=dstU;
  dstp[3]=dstV;
  int src_pitch2 = src_pitch*2;
  int dst_pitch2 = dst_pitchY*2;
  int src_pitch4 = src_pitch*4;
  int dst_pitch3 = dst_pitchY*3;

  int y=0;
  int x=0;
  src_rowsize = (src_rowsize+3)/4;
  __asm {
  push ebx    // stupid compiler forgets to save ebx!!
    mov edx,0
    mov esi, src
    mov edi, dstp
    jmp yloop_test
    align 16
yloop:
      mov edx,0               // x counter   
      mov eax, [src_pitch2]
      jmp xloop_test
      align 16
xloop:      
      movq mm0,[esi]        // YUY2 upper line  (4 pixels luma, 2 chroma) (u1)
       movq mm1,[esi+eax]   // YUY2 lower line  (l1)
      movq mm7,mm0          // (u1)
       movq mm2, [esi+8]    // Load second pair (u2)
      movq mm3, [esi+eax+8] // (l2)
       movq mm4,mm2         // (u2)
      movq mm6, mm1         // (l1)
       movq mm5, mm3        // (l2)

      psrlw mm5,8  //(l2)
       psrlw mm6,8 //(l1)
      psrlw mm4,8  //(u2)
       psrlw mm7,8 //(u1)

      paddw mm5,mm4  //l2+u2
       paddw mm6,mm7 //l1+u1
      paddw mm5,mm4  //l2+u2+u2
       paddw mm6,mm7 //l1+u1+u1
      paddw mm5,mm4  //l2+u2+u2+u2
       paddw mm6,mm7 //l1+u1+u1+u1
      movq mm4,[mask1]
       movq mm7,[mask2]
      paddw mm5,[add_2]
       paddw mm6,[add_2]

//      pavgb mm6,mm1         // Average (chroma)
//       pavgb mm5,mm3        // Average Chroma (second pair)

      pand mm0,mm4          // Mask luma
  	    psrlw mm5, 2
      pand mm1,mm4          // Mask luma
 	     psrlw mm6, 2
      pand mm2,mm4          // Mask luma
       pand mm3,mm4         
      pand mm5,mm4           // Mask chroma
       pand mm6,mm4          // Mask chroma
   		packuswb mm0, mm2     // Pack luma (upper)
   		 packuswb mm6, mm5    // Pack chroma
   		packuswb mm1, mm3     // Pack luma (lower)     
       movq mm5, mm6        // Chroma copy
      pand mm5, mm7         // Mask V
       pand mm6, mm4        // Mask U
      psrlq mm5,8            // shift down V
   		 packuswb mm5, mm7     // Pack U 
   		packuswb mm6, mm7     // Pack V 
       mov ebx, [edi]
      mov ecx, [edi+4]
      movq [ebx+edx*2],mm0
       movq [ecx+edx*2],mm1

      mov ecx, [edi+8]
      mov ebx, [edi+12]
       movd [ecx+edx], mm6  // Store U
      movd [ebx+edx], mm5   // Store V
      add esi, 16
      add edx, 4
xloop_test:
    cmp edx,[src_rowsize]
    jl xloop

    mov esi, src
    mov edx,[edi]
    mov ebx,[edi+4]
    mov ecx,[edi+8]
    mov eax,[edi+12]
    
    add edx, [dst_pitchY]
    add ebx, [dst_pitchY]
    add ecx, [dst_pitchUV]
    add eax, [dst_pitchUV]
    add esi, [src_pitch]

    mov [edi],edx
    mov [edi+4],ebx
    mov [edi+8],ecx
    mov [edi+12],eax

    mov edx, 0
    mov eax, [src_pitch2]

    jmp xloop2_test
xloop2:   // Second field
      movq mm0,[esi]        // YUY2 upper line  (4 pixels luma, 2 chroma)
       movq mm1,[esi+eax]   // YUY2 lower line  
      movq mm6,mm0
       movq mm2, [esi+8]    // Load second pair
      movq mm3, [esi+eax+8]
       movq mm5,mm2
      movq mm7, mm1
       movq mm4, mm3
      psrlw mm5,8
       psrlw mm6,8
      psrlw mm4,8
       psrlw mm7,8
      paddw mm5,mm4
       paddw mm6,mm7
      paddw mm5,mm4
       paddw mm6,mm7
      paddw mm5,mm4
       paddw mm6,mm7
      movq mm4,[mask1]
       movq mm7,[mask2]
      paddw mm5,[add_2]
       paddw mm6,[add_2]

//      pavgb mm6,mm1         // Average (chroma)
//       pavgb mm5,mm3        // Average Chroma (second pair)

      pand mm0,mm4          // Mask luma
  	    psrlw mm5, 2
      pand mm1,mm4          // Mask luma
 	     psrlw mm6, 2
      pand mm2,mm4          // Mask luma
       pand mm3,mm4         
      pand mm5,mm4           // Mask chroma
       pand mm6,mm4          // Mask chroma
   		packuswb mm0, mm2     // Pack luma (upper)
   		 packuswb mm6, mm5    // Pack chroma
   		packuswb mm1, mm3     // Pack luma (lower)     
       movq mm5, mm6        // Chroma copy
      pand mm5, mm7         // Mask V
       pand mm6, mm4        // Mask U
      psrlq mm5,8            // shift down V
   		 packuswb mm5, mm7     // Pack U 
   		packuswb mm6, mm7     // Pack V 
       mov ebx, [edi]
      mov ecx, [edi+4]
      movq [ebx+edx*2],mm0
       movq [ecx+edx*2],mm1

      mov ecx, [edi+8]
      mov ebx, [edi+12]
       movd [ecx+edx], mm6  // Store U
      movd [ebx+edx], mm5   // Store V
      add esi, 16
      add edx, 4
xloop2_test:
    cmp edx,[src_rowsize]
    jl xloop2

    mov esi, src
    mov edx,[edi]
    mov ebx,[edi+4]
    mov ecx,[edi+8]
    mov eax,[edi+12]
    
    add edx, [dst_pitch3]
    add ebx, [dst_pitch3]
    add ecx, [dst_pitchUV]
    add eax, [dst_pitchUV]
    add esi, [src_pitch4]

    mov [edi],edx
    mov [edi+4],ebx
    mov [edi+8],ecx
    mov [edi+12],eax
    mov edx, [y]
    mov [src],esi
    
    add edx, 4

yloop_test:
    cmp edx,[height]
    mov [y],edx
    jl yloop
    emms
    pop ebx
  }
   delete[] dstp;
}
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE

}; // namespace avxsynth
