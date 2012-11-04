#                                       -*- Automake -*-
# Process this file with automake to produce a Makefile.

ACLOCAL_AMFLAGS = -I m4

if OPT_autocrop
  AUTOCROP_SUBDIRS = plugins/autocrop
endif

if OPT_framecapture
  AVXFRAMECAPTURE_SUBDIRS = plugins/avxframecapture
endif

if OPT_subtitle
  AVXSUBTITLE_SUBDIRS = plugins/avxsubtitle
endif

if OPT_ffms2
  AVXFFMS2_SUBDIRS = plugins/avxffms2
endif

if OPT_avxedit
  AVXEDIT_SUBDIRS = apps/AVXEdit
endif

SUBDIRS = \
  avxutils                   \
  avxsynth                   \
  apps/avxframeserver        \
  $(AUTOCROP_SUBDIRS)        \
  $(AVXFRAMECAPTURE_SUBDIRS) \
  $(AVXSUBTITLE_SUBDIRS)     \
  $(AVXFFMS2_SUBDIRS)        \
  $(AVXEDIT_SUBDIRS)         \
  include                    \
  scripts

dist_doc_DATA = README.md

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = avxsynth.pc avxutils.pc

uninstall-hook:
	-rmdir $(pkglibdir)

