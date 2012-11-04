#                                       -*- Automake -*-
# Process this file with automake to produce a Makefile.

nobase_pkginclude_HEADERS = \
	avxlog.h                                \
	avxplugin.h                             \
	utils/AvxTextRender.h                   \
	utils/AvxString.h                       \
	utils/Path.h                            \
	utils/Size.h                            \
	utils/AvxException.h                    \
	utils/TextConfig.h                      \
	utils/TextLayout.h                      \
	windowsPorts/WinNTLinux.h               \
	windowsPorts/excptLinux.h               \
	windowsPorts/basicDataTypeConversions.h \
	windowsPorts/WinDefLinux.h              \
	windowsPorts/windows2linux.h

uninstall-hook:
	-rmdir $(pkgincludedir)/utils
	-rmdir $(pkgincludedir)/windowsPorts
	-rmdir $(pkgincludedir)

