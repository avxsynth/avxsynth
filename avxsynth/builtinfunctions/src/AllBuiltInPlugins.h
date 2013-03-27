#ifndef __ALL_BUILT_IN_PLUGINS_H__
#define __ALL_BUILT_IN_PLUGINS_H__

#include "common/include/stdafx.h"
#include "common/include/core/avxsynth.h"
#include "avxlog.h"
#include "common/include/internal.h"
#include "builtinfunctions/src/audio/audio.h"
#include "builtinfunctions/src/filters/color.h"
#include "builtinfunctions/src/filters/combine.h"
#include "common/include/convert/convert.h"
#include "builtinfunctions/src/convert/convert_rgb.h"
#include "builtinfunctions/src/convert/convert_yuy2.h"
#include "builtinfunctions/src/filters/convolution.h"
#include "builtinfunctions/src/filters/edit.h"
#include "builtinfunctions/src/filters/field.h"
#include "builtinfunctions/src/filters/focus.h"
#include "builtinfunctions/src/filters/fps.h"
#include "builtinfunctions/src/filters/histogram.h"
#if 0
#include <ImageSeq.h> // doesn't exist in source tree
#endif
#include "builtinfunctions/src/filters/layer.h"
#include "builtinfunctions/src/filters/levels.h"
#include "builtinfunctions/src/filters/merge.h"
#include "builtinfunctions/src/filters/misc.h"
#include "builtinfunctions/src/filters/resample.h"
#include "builtinfunctions/src/filters/resize.h"
#include "builtinfunctions/src/filters/transform.h"
#include "builtinfunctions/src/filters/turn.h"
#include "builtinfunctions/src/filters/greyscale.h"
#include "builtinfunctions/src/filters/planeswap.h"
#if 0
#include "common/include/filters/overlay/overlay.h"
#include "sources/avi-source-private.h" // doesn't exist in source tree
#endif 
#include "builtinfunctions/src/sources/source-private.h"
#if 0
#include "core/src/audio/ssrc-convert.h"
#include "core/src/audio/supereq.h"
#include "audio/supereq_private.h" // doesn't exist in source tree
#include "audio/avs-soundtouch-private.h" // doesn't exist in source tree
#endif

namespace avxsynth {
	
}; // namespace avxsynth
#endif // __ALL_BUILT_IN_PLUGINS_H__
