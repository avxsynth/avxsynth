#ifndef __ALL_BUILT_IN_PLUGINS_H__
#define __ALL_BUILT_IN_PLUGINS_H__

#include "stdafx.h"
#include "core/avxsynth.h"
#include "avxlog.h"
#include "internal.h"
#include "audio/audio.h"
#include "filters/color.h"
#include "filters/combine.h"
#include "convert/convert.h"
#include "convert/convert_rgb.h"
#include "convert/convert_yuy2.h"
#include "filters/convolution.h"
#include "filters/edit.h"
#include "filters/field.h"
#include "filters/focus.h"
#include "filters/fps.h"
#include "filters/histogram.h"
#if 0
#include <ImageSeq.h> // doesn't exist in source tree
#endif
#include "filters/layer.h"
#include "filters/levels.h"
#include "filters/merge.h"
#include "filters/misc.h"
#include "filters/resample.h"
#include "filters/resize.h"
#include "filters/transform.h"
#include "filters/turn.h"
#include "filters/greyscale.h"
#include "filters/planeswap.h"
#if 0
#include "filters/overlay/overlay.h"
#include "sources/avi-source-private.h" // doesn't exist in source tree
#endif 
#include "sources/source-private.h"
#if 0
#include "audio/ssrc-convert.h"
#include "audio/supereq.h"
#include "audio/supereq_private.h" // doesn't exist in source tree
#include "audio/avs-soundtouch-private.h" // doesn't exist in source tree
#endif

namespace avxsynth {
	
}; // namespace avxsynth
#endif // __ALL_BUILT_IN_PLUGINS_H__
