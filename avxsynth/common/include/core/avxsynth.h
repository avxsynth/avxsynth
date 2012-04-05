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

#ifndef __AVXSYNTH_H__
#define __AVXSYNTH_H__

/* Define all types necessary for interfacing with avisynth.dll
   Moved from internal.h */
#define LINUXIZED_VERSION
#ifdef LINUXIZED_VERSION

#else // LINUXIZED_VERSION
// Win32 API macros, notably the types BYTE, DWORD, ULONG, etc. 
//#include <windef.h>  

// COM interface macros
//#include <objbase.h>
#endif // LINUXIZED_VERSION

#include "avxplugin.h"

namespace avxsynth {
	
enum
{
	BUILT_IN_FUNCTIONS_FAMILY_INDEX_Audio_filters = 0,
	BUILT_IN_FUNCTIONS_FAMILY_INDEX_Combine_filters, 
	BUILT_IN_FUNCTIONS_FAMILY_INDEX_Convert_filters,
    BUILT_IN_FUNCTIONS_FAMILY_INDEX_Convolution_filters, 
	BUILT_IN_FUNCTIONS_FAMILY_INDEX_Edit_filters, 
	BUILT_IN_FUNCTIONS_FAMILY_INDEX_Field_filters,
    BUILT_IN_FUNCTIONS_FAMILY_INDEX_Focus_filters, 
	BUILT_IN_FUNCTIONS_FAMILY_INDEX_Fps_filters, 
	BUILT_IN_FUNCTIONS_FAMILY_INDEX_Histogram_filters,
    BUILT_IN_FUNCTIONS_FAMILY_INDEX_Layer_filters, 
	BUILT_IN_FUNCTIONS_FAMILY_INDEX_Levels_filters, 
	BUILT_IN_FUNCTIONS_FAMILY_INDEX_Misc_filters,
    BUILT_IN_FUNCTIONS_FAMILY_INDEX_Resample_filters, 
	BUILT_IN_FUNCTIONS_FAMILY_INDEX_Resize_filters,
    BUILT_IN_FUNCTIONS_FAMILY_INDEX_Script_functions, 
	BUILT_IN_FUNCTIONS_FAMILY_INDEX_Source_filters, 
	BUILT_IN_FUNCTIONS_FAMILY_INDEX_Text_filters,
    BUILT_IN_FUNCTIONS_FAMILY_INDEX_Transform_filters, 
	BUILT_IN_FUNCTIONS_FAMILY_INDEX_Merge_filters, 
	BUILT_IN_FUNCTIONS_FAMILY_INDEX_Color_filters,
    BUILT_IN_FUNCTIONS_FAMILY_INDEX_Debug_filters, 
	BUILT_IN_FUNCTIONS_FAMILY_INDEX_Image_filters, 
	BUILT_IN_FUNCTIONS_FAMILY_INDEX_Turn_filters,
    BUILT_IN_FUNCTIONS_FAMILY_INDEX_Conditional_filters, 
	BUILT_IN_FUNCTIONS_FAMILY_INDEX_Conditional_funtions_filters,
    BUILT_IN_FUNCTIONS_FAMILY_INDEX_Plugin_functions, 
	BUILT_IN_FUNCTIONS_FAMILY_INDEX_CPlugin_filters, 
	BUILT_IN_FUNCTIONS_FAMILY_INDEX_Cache_filters,
    BUILT_IN_FUNCTIONS_FAMILY_INDEX_SSRC_filters, 
	BUILT_IN_FUNCTIONS_FAMILY_INDEX_SuperEq_filters, 
	BUILT_IN_FUNCTIONS_FAMILY_INDEX_Overlay_filters,
    BUILT_IN_FUNCTIONS_FAMILY_INDEX_Greyscale_filters, 
	BUILT_IN_FUNCTIONS_FAMILY_INDEX_Swap_filters,
    BUILT_IN_FUNCTIONS_FAMILY_INDEX_Soundtouch_filters,
	NUMBER_OF_BUILT_IN_FUNCTIONS_FAMILIES
};

#define BUILT_IN_AUDIO_FILTERS_NUMBER_OF_ITEMS                 (23)
#define BUILT_IN_COMBINE_FILTERS_NUMBER_OF_ITEMS               (6)
#define BUILT_IN_CONVERT_FILTERS_NUMBER_OF_ITEMS               (6)
#define BUILT_IN_CONVOLUTION_FILTERS_NUMBER_OF_ITEMS           (1)
#define BUILT_IN_EDIT_FILTERS_NUMBER_OF_ITEMS                  (20)
#define BUILT_IN_FIELD_FILTERS_NUMBER_OF_ITEMS                 (16)
#define BUILT_IN_FOCUS_FILTERS_NUMBER_OF_ITEMS                 (4)
#define BUILT_IN_FPS_FILTERS_NUMBER_OF_ITEMS                   (13)
#define BUILT_IN_HISTOGRAM_FILTERS_NUMBER_OF_ITEMS             (1)
#define BUILT_IN_LAYER_FILTERS_NUMBER_OF_ITEMS                 (12)
#define BUILT_IN_LEVELS_FILTERS_NUMBER_OF_ITEMS                (4)
#define BUILT_IN_MISC_FILTERS_NUMBER_OF_ITEMS                  (3)
#define BUILT_IN_RESAMPLE_FILTERS_NUMBER_OF_ITEMS              (10)
#define BUILT_IN_RESIZE_FILTERS_NUMBER_OF_ITEMS                (3)
#define BUILT_IN_SCRIPT_FUNCTIONS_NUMBER_OF_ITEMS              (79)
#define BUILT_IN_SOURCE_FILTERS_NUMBER_OF_ITEMS                (14)
#define BUILT_IN_TEXT_FILTERS_NUMBER_OF_ITEMS                  (6)
#define BUILT_IN_TRANSFORM_FILTERS_NUMBER_OF_ITEMS             (7)
#define BUILT_IN_MERGE_FILTERS_NUMBER_OF_ITEMS                 (3)
#define BUILT_IN_COLOR_FILTERS_NUMBER_OF_ITEMS                 (1)
#define BUILT_IN_DEBUG_FILTERS_NUMBER_OF_ITEMS                 (3)
#define BUILT_IN_IMAGE_FILTERS_NUMBER_OF_ITEMS                 (3)
#define BUILT_IN_TURN_FILTERS_NUMBER_OF_ITEMS                  (3)
#define BUILT_IN_CONDITIONAL_FILTERS_NUMBER_OF_ITEMS           (8)
#define BUILT_IN_CONDITIONAL_FUNCTIONS_FILTERS_NUMBER_OF_ITEMS (27)
#define BUILT_IN_PLUGIN_FUNCTIONS_NUMBER_OF_ITEMS              (1)
#define BUILT_IN_CPLUGIN_FILTERS_NUMBER_OF_ITEMS               (2)
#define BUILT_IN_CACHE_FILTERS_NUMBER_OF_ITEMS                 (2)
#define BUILT_IN_SSRC_FILTERS_NUMBER_OF_ITEMS                  (1)
#define BUILT_IN_SUPEREQ_FILTERS_NUMBER_OF_ITEMS               (1)
#define BUILT_IN_OVERLAY_FILTERS_NUMBER_OF_ITEMS               (1)
#define BUILT_IN_GRAYSCALE_FILTERS_NUMBER_OF_ITEMS             (2)
#define BUILT_IN_SWAP_FILTERS_NUMBER_OF_ITEMS                  (5)
#define BUILT_IN_SOUNDTOUCH_FILTERS_NUMBER_OF_ITEMS            (1)



}; // namespace avxsynth
#endif //__AVXSYNTH_H__
