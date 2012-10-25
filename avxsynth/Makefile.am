#                                       -*- Automake -*-
# Process this file with automake to produce a Makefile.

noinst_LTLIBRARIES = libavxbtinfncs.la

lib_LTLIBRARIES = libavxsynth.la

noinst_HEADERS = \
	common/include/core/cache.h                          \
	common/include/core/info.h                           \
	common/include/core/avxsynth.h                       \
	common/include/internal.h                            \
	common/include/stdafx.h                              \
	common/include/convert/convert.h                     \
	common/include/filters/overlay/blend_asm.h           \
	common/include/filters/overlay/overlayfunctions.h    \
	common/include/filters/overlay/imghelpers.h          \
	common/include/filters/overlay/overlay.h             \
	common/include/filters/overlay/444convert.h          \
	common/include/filters/text-overlay.h                \
	builtinfunctions/builtInFunctionsExportDefinitions.h

libavxbtinfncs_la_SOURCES = \
	builtinfunctions/src/AllBuiltInPluginsEntryPoint.cpp \
	builtinfunctions/src/audio/audio.cpp                 \
	builtinfunctions/src/core/alignplanar.cpp            \
	builtinfunctions/src/filters/color.cpp               \
	builtinfunctions/src/filters/combine.cpp             \
	builtinfunctions/src/filters/convolution.cpp         \
	builtinfunctions/src/filters/edit.cpp                \
	builtinfunctions/src/filters/field.cpp               \
	builtinfunctions/src/filters/focus.cpp               \
	builtinfunctions/src/filters/fps.cpp                 \
	builtinfunctions/src/filters/greyscale.cpp           \
	builtinfunctions/src/filters/histogram.cpp           \
	builtinfunctions/src/filters/layer.cpp               \
	builtinfunctions/src/filters/levels.cpp              \
	builtinfunctions/src/filters/merge.cpp               \
	builtinfunctions/src/filters/misc.cpp                \
	builtinfunctions/src/filters/planeswap.cpp           \
	builtinfunctions/src/filters/resample.cpp            \
	builtinfunctions/src/filters/resize.cpp              \
	builtinfunctions/src/filters/transform.cpp           \
	builtinfunctions/src/filters/turnfunc.cpp            \
	builtinfunctions/src/filters/turn.cpp                \
	builtinfunctions/src/sources/source.cpp              \
	builtinfunctions/src/convert/convert.cpp             \
	builtinfunctions/src/convert/convert_rgb.cpp         \
	builtinfunctions/src/convert/convert_yuy2.cpp        \
	builtinfunctions/src/convert/convert_yv12.cpp        \
	builtinfunctions/src/sources/source-private.h        \
	builtinfunctions/src/audio/audio.h                   \
	builtinfunctions/src/AllBuiltInPlugins.h             \
	builtinfunctions/src/convert/convert_yv12.h          \
	builtinfunctions/src/convert/convert_rgb.h           \
	builtinfunctions/src/convert/convert_yuy2.h          \
	builtinfunctions/src/filters/levels.h                \
	builtinfunctions/src/filters/resample.h              \
	builtinfunctions/src/filters/edit.h                  \
	builtinfunctions/src/filters/layer.h                 \
	builtinfunctions/src/filters/planeswap.h             \
	builtinfunctions/src/filters/histogram.h             \
	builtinfunctions/src/filters/turn.h                  \
	builtinfunctions/src/filters/misc.h                  \
	builtinfunctions/src/filters/transform.h             \
	builtinfunctions/src/filters/merge.h                 \
	builtinfunctions/src/filters/greyscale.h             \
	builtinfunctions/src/filters/resize.h                \
	builtinfunctions/src/filters/convolution.h           \
	builtinfunctions/src/filters/turnfunc.h              \
	builtinfunctions/src/filters/fps.h                   \
	builtinfunctions/src/filters/color.h                 \
	builtinfunctions/src/filters/combine.h               \
	builtinfunctions/src/filters/field.h                 \
	builtinfunctions/src/filters/focus.h

libavxbtinfncs_la_CPPFLAGS = \
	-I$(srcdir)/common/include         \
	-I$(srcdir)/common/include/convert \
	-I$(srcdir)/common/include/core    \
	-I$(srcdir)/common/include/filters \
	-I$(srcdir)/builtinfunctions/src   \
	-I$(top_srcdir)/include            \
	$(AM_CPPFLAGS)

libavxsynth_la_LIBADD = \
	$(top_builddir)/avxutils/libavxutils.la \
	libavxbtinfncs.la

libavxsynth_la_SOURCES = \
	core/src/audio/convertaudio.cpp                        \
	core/src/core/avxsynth_c.cpp                           \
	core/src/core/avxsynth.cpp                             \
	core/src/core/cache.cpp                                \
	core/src/core/main.cpp                                 \
	core/src/core/memcpy_amd.cpp                           \
	core/src/core/plugins.cpp                              \
	core/src/core/parser/expression.cpp                    \
	core/src/core/parser/script.cpp                        \
	core/src/core/parser/scriptparser.cpp                  \
	core/src/core/parser/tokenizer.cpp                     \
	core/src/filters/conditional/conditional.cpp           \
	core/src/filters/conditional/conditional_functions.cpp \
	core/src/filters/conditional/conditional_reader.cpp    \
	core/src/filters/debug.cpp                             \
	core/src/filters/text-overlay.cpp                      \
	core/src/sources/avi/cpuaccel.cpp                      \
	core/src/windowsPorts.cpp                              \
	core/src/core/clip_info.h                              \
	core/src/core/resource.h                               \
	core/src/core/memcpy_amd.h                             \
	core/src/core/parser/expression.h                      \
	core/src/core/parser/scriptparser.h                    \
	core/src/core/parser/tokenizer.h                       \
	core/src/core/parser/script.h                          \
	core/src/core/Error.h                                  \
	core/src/core/avxsynth_c.h                             \
	core/src/resource.h                                    \
	core/src/audio/paramlist.h                             \
	core/src/audio/avs-soundtouch.h                        \
	core/src/audio/math_shared.h                           \
	core/src/audio/supereq.h                               \
	core/src/audio/ssrc-convert.h                          \
	core/src/audio/ssrc.h                                  \
	core/src/windowsPorts.h                                \
	core/src/filters/debug.h                               \
	core/src/filters/conditional/conditional.h             \
	core/src/filters/conditional/conditional_functions.h   \
	core/src/filters/conditional/conditional_reader.h

pkginclude_HEADERS = \
	core/src/core/avxsynth_c.h

libavxsynth_la_CPPFLAGS = \
	-DAVXSYNTH_PLUGIN_PATH='"$(pkglibdir)"' \
	-I$(srcdir)/common/include              \
	-I$(srcdir)/common/include/core         \
	-I$(srcdir)/common/include/filters      \
	-I$(srcdir)/builtinfunctions            \
	-I$(srcdir)/core/src                    \
	-I$(top_srcdir)/include/utils           \
	-I$(top_srcdir)/include                 \
	$(AM_CPPFLAGS)

libavxsynth_la_LDFLAGS = -version-info 0:0:0 -export-dynamic $(AM_LDFLAGS)

# For some reason libtool refuses to run ldconfig
install-exec-hook:
	-ldconfig
