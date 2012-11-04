#                                       -*- Automake -*-
# Process this file with automake to produce a Makefile.

nobase_pkginclude_HEADERS = \
	avxiface.h                              \
	avxlog.h                                \
	avxplugin.h                             \
	utils/AvxTextRender.h                   \
	utils/AvxString.h                       \
	utils/Path.h                            \
	utils/Size.h                            \
	utils/AvxException.h                    \
	utils/TextConfig.h                      \
	utils/TextLayout.h                      \
	windowsPorts/MMRegLinux.h               \
	windowsPorts/WinNTLinux.h               \
	windowsPorts/VfwLinux.h                 \
	windowsPorts/excptLinux.h               \
	windowsPorts/basicDataTypeConversions.h \
	windowsPorts/WinBaseLinux.h             \
	windowsPorts/WinDefLinux.h              \
	windowsPorts/windows2linux.h            \
	windowsPorts/WinGDILinux.h              \
	windowsPorts/UnknwnLinux.h

uninstall-hook:
	-rmdir $(pkgincludedir)/utils
	-rmdir $(pkgincludedir)/windowsPorts
	-rmdir $(pkgincludedir)

