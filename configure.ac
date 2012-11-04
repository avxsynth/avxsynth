#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# This autoconf file was written against Autoconf 2.68
# Automake files were written against Automake 1.11.3 and libtool 2.4.2

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([avxutils/src/AvxTextRender.cpp])

AC_INIT([avxsynth], [4.0])
AM_INIT_AUTOMAKE([foreign subdir-objects -Wall parallel-tests])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES]) # Automake 1.11 is required for silent build.

# Check for required programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
LT_INIT([dlopen disable-static])
PKG_PROG_PKG_CONFIG

# Create options for plugins and avxedit. Always build avxFrameServer.
AC_ARG_ENABLE([all],
  [AS_HELP_STRING(
    [--enable-all],
    [enable all optional components (individual switches override)]
  )]
)
AC_ARG_ENABLE([autocrop],
  [AS_HELP_STRING(
    [--enable-autocrop],
    [build autocrop plugin (default yes)]
  )]
)
AC_ARG_ENABLE([framecapture],
  [AS_HELP_STRING(
    [--enable-framecapture],
    [build framecapture plugin (default yes, requires libjpeg)]
  )]
)
AC_ARG_ENABLE([subtitle],
  [AS_HELP_STRING(
    [--enable-subtitle],
    [build subtitle plugin (default yes)]
  )]
)
AC_ARG_ENABLE([ffms2],
  [AS_HELP_STRING(
    [--enable-ffms2],
    [build ffms2 plugin (default yes, requires ffms2 and libav/ffmpeg)]
  )]
)
AC_ARG_ENABLE([avxedit],
  [AS_HELP_STRING(
    [--enable-avxedit],
    [build AVXEdit program (default yes, requires Qt4)]
  )]
)
AC_ARG_WITH([jpeg],
  [AS_HELP_STRING(
    [--with-jpeg=DIR],
    [specify libjpeg installation prefix]
  )]
)

# Handle enable_all .
if test "x$enable_all" = "xyes"; then
  test "x$enable_autocrop" = "x" && enable_autocrop="yes"
  test "x$enable_framecapture" = "x" && enable_framecapture="yes"
  test "x$enable_subtitle" = "x" && enable_subtitle="yes"
  test "x$enable_ffms2" = "x" && enable_ffms2="yes"
  test "x$enable_avxedit" = "x" && enable_avxedit="yes"
elif test "x$enable_all" = "xno"; then
  test "x$enable_autocrop" = "x" && enable_autocrop="no"
  test "x$enable_framecapture" = "x" && enable_framecapture="no"
  test "x$enable_subtitle" = "x" && enable_subtitle="no"
  test "x$enable_ffms2" = "x" && enable_ffms2="no"
  test "x$enable_avxedit" = "x" && enable_avxedit="no"
fi

# Handle disable_shared here in case libtool decides it can't build shared libs.
AS_IF(
  [test "x$enable_shared" = "xno"],
  [
    AC_MSG_WARN([plugins can not be built as static libraries])
    if
      test "x$enable_autocrop" = "xyes"     || \
      test "x$enable_framecapture" = "xyes" || \
      test "x$enable_subtitle" = "xyes"     || \
      test "x$enable_ffms2" = "xyes";
    then
      AC_MSG_ERROR([plugins are enabled but shared libraries are not])
    else
      AC_MSG_WARN([disabling all plugins])
    fi
    enable_autocrop="no"
    enable_framecapture="no"
    enable_subtitle="no"
    enable_ffms2="no"
  ]
)

# Library tests follow. Where supported, pkg-config is used to guess
# the appropriate includes and libs. However, pkg-config does not
# make any claim regarding functionality, so we check all results.

AC_DEFUN([SAVE_COMPILER_FLAGS],
  [
    _SAVED_CFLAGS="$CFLAGS" _SAVED_CXXFLAGS="$CXXFLAGS"
    _SAVED_LIBS="$LIBS" _SAVED_LDFLAGS="$LDFLAGS"
  ]
)
AC_DEFUN([RESTORE_COMPILER_FLAGS],
  [
    CFLAGS="$_SAVED_CFLAGS" CXXFLAGS="$_SAVED_CXXFLAGS"
    LIBS="$_SAVED_LIBS" LDFLAGS="$_SAVED_LDFLAGS"
  ]
)

# libtool can find these automatically, but not if the user requests
# a completely static executable.
AC_SEARCH_LIBS([dlopen], [dl dld],
  [], [AC_MSG_ERROR([unable to find dlopen() function])]
)
AC_SEARCH_LIBS([cos], [m],
  [], [AC_MSG_ERROR([unable to find cos() function])]
)

AC_LANG([C++])
PKG_CHECK_MODULES([log4cpp], [log4cpp])
SAVE_COMPILER_FLAGS
CXXFLAGS="$CXXFLAGS $log4cpp_CFLAGS" LIBS="$LIBS $log4cpp_LIBS"
AC_MSG_CHECKING([if log4cpp works])
AC_LINK_IFELSE(
  [AC_LANG_PROGRAM(
    [#include <log4cpp/SimpleLayout.hh>],
    [log4cpp::Layout* dummy = new log4cpp::SimpleLayout()]
  )],
  [AC_MSG_RESULT([yes])],
  [AC_MSG_ERROR([log4cpp does not work, see config.log])]
)
RESTORE_COMPILER_FLAGS

AC_LANG([C])
PKG_CHECK_MODULES([pangocairo], [pangocairo])
SAVE_COMPILER_FLAGS
CFLAGS="$CFLAGS $pangocairo_CFLAGS" LIBS="$LIBS $pangocairo_LIBS"
AC_MSG_CHECKING([if pangocairo works])
AC_LINK_IFELSE(
  [AC_LANG_PROGRAM(
    [#include <pango/pangocairo.h>],
    [pango_cairo_font_map_get_default()]
  )],
  [AC_MSG_RESULT([yes])],
  [AC_MSG_ERROR([pangocairo does not work, see config.log])]
)
RESTORE_COMPILER_FLAGS

# Tests for optional components follow.
# The tests work as follows:
#
# If the component is either enabled or unset, check for the required
# library X and set HAVE_X to either yes or no. In a separate statement,
# check HAVE_X and either enable or disable the component. If HAVE_X was
# not set, that means the user originally disabled the component and
# hence we disable it.

# These require no tests.
test "x$enable_autocrop" = "x" && enable_autocrop="yes"
test "x$enable_subtitle" = "x" && enable_subtitle="yes"

# Framecapture plugin requires libjpeg.
AC_LANG([C])
AS_IF(
  [test "x$enable_framecapture" = "xyes" || test "x$enable_framecapture" = "x"],
  [
    SAVE_COMPILER_FLAGS
    JPEG_PREFIX="$with_jpeg"
    if
      test ! "x$with_jpeg" = "xyes" && \
      test ! "x$with_jpeg" = "xno" && \
      test ! "x$with_jpeg" = "x";
    then
      jpeg_CFLAGS="-I\"$with_jpeg/include\""
      jpeg_LIBS="-L\"$with_jpeg/lib\""
    fi
    SAVE_COMPILER_FLAGS
    CFLAGS="$CFLAGS $jpeg_CFLAGS" LIBS="$LIBS $jpeg_LIBS"
    AC_CHECK_HEADER([jpeglib.h], [AC_SUBST([jpeg_CFLAGS])], [HAVE_JPEG="no"])
    AC_SEARCH_LIBS(
      [jpeg_destroy_compress], [jpeg],
      [
        if test "x$ac_cv_search_jpeg_destroy_compress" = "xnone required"; then
          jpeg_LIBS=""
        else
          jpeg_LIBS="$ac_cv_search_jpeg_destroy_compress"
        fi
        AC_SUBST([jpeg_LIBS])
      ],
      [HAVE_JPEG="no"]
    )
    test ! "x$HAVE_JPEG" = "xno" && HAVE_JPEG="yes"
    RESTORE_COMPILER_FLAGS
  ]
)
AS_IF(
  [test "x$HAVE_JPEG" = "xno"],
  [
    if test "x$enable_framecapture" = "xyes"; then
      AC_MSG_ERROR([libjpeg required for framecapture plugin])
    elif test "x$enable_framecapture" = "x"; then
      AC_MSG_WARN([libjpeg not found, disabling framecapture plugin])
      enable_framecapture="no"
    fi
  ],
  [test "x$HAVE_JPEG" = "xyes"],
  [enable_framecapture="yes"],
  [enable_framecapture="no"]
)

# ffms2 plugin requires libav and ffms2. We check for each part of libav
# separately to provide the user with more useful information.
AC_LANG([C])
AS_IF(
  [test "x$enable_ffms2" = "xyes" || test "x$enable_ffms2" = "x"],
  [
    # We add -shared to the LDFLAGS for checking to prevent turtles-all-the-way-down issues.
    # This has the unfortunate side effect of preventing symbol checking from working.
    # Note that when compiling we will need to link ffms2 before libav.
    PKG_CHECK_MODULES([ffms2], [ffms2], [], [true])
    SAVE_COMPILER_FLAGS
    CFLAGS="$CFLAGS $lt_prog_compiler_pic $ffms2_CFLAGS" LIBS="$LIBS $ffms2_LIBS"
    LDFLAGS="$LDFLAGS $lt_prog_compiler_pic -shared"
    AC_MSG_CHECKING([if ffms2 works])
    AC_LINK_IFELSE(
      [AC_LANG_PROGRAM(
        [#include <ffms.h>],
        [FFMS_GetEnabledSources()]
      )],
      [HAVE_FFMS2="yes"], [HAVE_FFMS2="no"]
    )
    AC_MSG_RESULT([$HAVE_FFMS2])
    RESTORE_COMPILER_FLAGS

    # The order is important -- ffms2 -> avformat -> avcodec -> postproc | swscale -> avutil.
    # Specifying the wrong order will cause undefined symbols when linking to static libs.
    PKG_CHECK_MODULES([libav], [libavformat libavcodec libswscale libpostproc libavutil], [], [true])

    # On x86_64, linking with static libav requires -Wl,-Bsymbolic
    SAVE_COMPILER_FLAGS
    CFLAGS="$CFLAGS $lt_prog_compiler_pic $libav_CFLAGS" LIBS="$LIBS $libav_LIBS"
    LDFLAGS="$LDFLAGS $lt_prog_compiler_pic -shared"
    AC_MSG_CHECKING([if libav works])
    AC_LINK_IFELSE(
      [AC_LANG_PROGRAM(
        [
          #include <libavcodec/avcodec.h>
          #include <libavformat/avformat.h>
          #include <libavutil/avutil.h>
          #include <libpostproc/postprocess.h>
          #include <libswscale/swscale.h>
        ],
        [
          avcodec_close(NULL);
          avformat_close_input(NULL);
          avutil_license();
          pp_free_context(NULL);
          sws_freeContext(NULL);
        ]
      )],
      [without_Bsymbolic="yes"], [without_Bsymbolic="no"]
    )
    AC_MSG_RESULT([$without_Bsymbolic])
    if test "x$without_Bsymbolic" = "xno"; then
      LDFLAGS="$LDFLAGS -Wl,-Bsymbolic"
      AC_MSG_CHECKING([if libav works with -Wl,-Bsymbolic])
      AC_LINK_IFELSE(
        [AC_LANG_PROGRAM(
          [
            #include <libavcodec/avcodec.h>
            #include <libavformat/avformat.h>
            #include <libavutil/avutil.h>
            #include <libpostproc/postprocess.h>
            #include <libswscale/swscale.h>
          ],
          [
            avcodec_close(NULL);
            avformat_close_input(NULL);
            avutil_license();
            pp_free_context(NULL);
            sws_freeContext(NULL);
          ]
        )],
        [with_Bsymbolic="yes"], [with_Bsymbolic="no"]
      )
      AC_MSG_RESULT([$with_Bsymbolic])
    fi
    RESTORE_COMPILER_FLAGS

    if test "x$without_Bsymbolic" = "xyes"; then
      HAVE_LIBAV="yes"
    elif test "x$with_Bsymbolic" = "xyes"; then
      libav_BSYMBOLIC="-Wl,-Bsymbolic" HAVE_LIBAV="yes"
    else
      HAVE_LIBAV="no"
    fi
    AC_SUBST([libav_BSYMBOLIC])
  ]
)
AS_IF(
  [test "x$HAVE_LIBAV" = "xno" || test "x$HAVE_FFMS2" = "xno"],
  [
   if test "x$enable_ffms2" = "xyes"; then
     AC_MSG_ERROR([ffmpeg/libav and ffms2 required for ffms2 plugin])
   elif test "x$enable_ffms2" = "x"; then
     AC_MSG_WARN([ffmpeg/libav and/or ffms2 not found, disabling ffms2 plugin])
     enable_ffms2="no"
   fi
  ],
  [test "x$HAVE_LIBAV" = "xyes" && test "x$HAVE_FFMS2" = "xyes"],
  [enable_ffms2="yes"],
  [enable_ffms2="no"]
)

# AVXEdit requires Qt. We must check for required Qt compilers as well as libs.
# This example adapted from http://qtnode.net/wiki/Qt_with_autotools .
AC_LANG([C++])
AS_IF(
  [test "x$enable_avxedit" = "xyes" || test "x$enable_avxedit" = "x"],
  [
    PKG_CHECK_MODULES([Qt], [QtCore QtGui >= 4],
      [
        test "x$MOC" = "x" && MOC=$("$PKG_CONFIG" --variable=moc_location QtCore)
        test "x$RCC" = "x" && RCC=$("$PKG_CONFIG" --variable=rcc_location QtCore)
        Qt_CFLAGS=$($PKG_CONFIG --cflags QtGui)
        Qt_LIBS=$($PKG_CONFIG --libs QtGui)
      ],
      [true]
    )
    AC_PATH_PROGS([MOC], [moc-qt4 moc], [moc])
    AC_PATH_PROG([RCC], [rcc])

# There's no way sane way to check rcc function, so we only test moc.
cat > myqt.h << EOF
#include <QObject>
class Test : public QObject
{
Q_OBJECT
public:
  Test() {}
  ~Test() {}
public slots:
  void receive() {}
signals:
  void send();
};
EOF

    SAVED_CXXFLAGS="$CXXFLAGS" SAVED_LIBS="$LIBS"
    CXXFLAGS="$CXXFLAGS $Qt_CFLAGS" LIBS="$LIBS $Qt_LIBS"

    AC_MSG_CHECKING([if $MOC works])
    if "$MOC" myqt.h -o moc_myqt.cpp 2>&AS_MESSAGE_LOG_FD; then
      HAVE_MOC="yes"; else HAVE_MOC="no"
    fi
    AC_MSG_RESULT([$HAVE_MOC])

    AC_MSG_CHECKING([if moc'd headers can be compiled])
    if "$CXX" $CXXFLAGS -c -o moc_myqt.o moc_myqt.cpp 2>&AS_MESSAGE_LOG_FD; then
      HAVE_MOC="yes"; else HAVE_MOC="no"
    fi
    AC_MSG_RESULT([$HAVE_MOC])

    # The other checks are for user diagnostics only.
    LIBS="moc_myqt.o $LIBS"
    AC_MSG_CHECKING([if Qt applications can be compiled])
    AC_LINK_IFELSE(
      [AC_LANG_PROGRAM(
        [
          #include "myqt.h"
          #include <QApplication>
        ],
        [
          int argc=0;
          QApplication app(argc, 0);
          Test t;
          QObject::connect(&t, SIGNAL(send()), &t, SLOT(receive()));
        ]
      )],
      [HAVE_QT="yes"], [HAVE_QT="no"]
    )
    AC_MSG_RESULT([$HAVE_QT])
    CXXFLAGS="$SAVED_CXXFLAGS" LIBS="$SAVED_LIBS"

    rm -f moc_myqt.cpp moc_myqt.o myqt.h
  ]
)
AS_IF(
  [test "x$HAVE_QT" = "xno" || test "x$RCC" = "x"],
  [
    if test "x$enable_avxedit" = "xyes"; then
      AC_MSG_ERROR([Qt4 libraries and moc+rcc are required for AVXEdit])
    elif test "x$enable_avxedit" = "x"; then
      AC_MSG_WARN([Required Qt4 libraries and tools not found, disabling AVXEdit])
      enable_avxedit="no"
    fi
  ],
  [test "x$HAVE_QT" = "xyes" && test ! "x$RCC" = "x"],
  [enable_avxedit="yes"],
  [enable_avxedit="no"]
)

# Test harness programs check.
AC_CHECK_PROGS([PYTHON], [python2.7 python3 python2 python], [python])
AS_IF(
  [test ! "x$PYTHON" = "xpython2.7"],
  [AC_MSG_WARN([detected python not python2.7, test suite may not work])]
)

# Record enabled features
AM_CONDITIONAL([OPT_autocrop], [test "x$enable_autocrop" = "xyes"])
AM_CONDITIONAL([OPT_framecapture], [test "x$enable_framecapture" = "xyes"])
AM_CONDITIONAL([OPT_subtitle], [test "x$enable_subtitle" = "xyes"])
AM_CONDITIONAL([OPT_ffms2], [test "x$enable_ffms2" = "xyes"])
AM_CONDITIONAL([OPT_avxedit], [test "x$enable_avxedit" = "xyes"])

AC_CONFIG_FILES(
  [
    Makefile
    avxutils/Makefile
    avxsynth/Makefile
    include/Makefile
    plugins/autocrop/Makefile
    plugins/avxffms2/Makefile
    plugins/avxframecapture/Makefile
    plugins/avxsubtitle/Makefile
    apps/avxframeserver/Makefile
    apps/AVXEdit/Makefile
    scripts/Makefile
    scripts/common.avsi
    avxsynth.pc
    avxutils.pc
  ]
)
AC_OUTPUT

cat << EOF

Configure completed successfully.

Build settings:
CXXFLAGS: $CXXFLAGS
CPPFLAGS: $CPPFLAGS
LDFLAGS: $LDFLAGS
LIBS: $LIBS

Library specific build settings:
log4cpp:
    CFLAGS: $log4cpp_CFLAGS
    LIBS: $log4cpp_LIBS
pangocairo:
    CFLAGS: $pangocairo_CFLAGS 
    LIBS: $pangocairo_LIBS
jpeg:
    CFLAGS: $jpeg_CFLAGS
    LIBS: $jpeg_LIBS
ffms2:
    CFLAGS: $ffms2_CFLAGS
    LIBS: $ffms2_LIBS
libav:
    CFLAGS: $libav_CFLAGS
    LIBS: $libav_LIBS
Qt:
    moc: $MOC
    rcc: $RCC
    CFLAGS: $Qt_CFLAGS
    LIBS: $Qt_LIBS

Enabled optional components:
autocrop: $enable_autocrop
avxframecapture: $enable_framecapture
avxsubtitle: $enable_subtitle
avxffms2: $enable_ffms2
AVXEdit: $enable_avxedit

Has a library not been detected?
Review config.log and set relevant environment variables.

Library linking style:
Shared libraries: $enable_shared
Static libraries: $enable_static

To compile program, run:
    make

To install, run:
    make [[DESTDIR=...]] install
Remember to run ldconfig after installing libraries to their
final destination. Additionally, DESTDIR-installed libraries
require "libtool --finish <dir>" after their final install.

Other valid targets:
    check dist clean distclean
EOF
