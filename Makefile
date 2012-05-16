include config.mak
include config.targets

ifeq ($(SRCPATH),.)
        SRCPATH="$(shell pwd)"
endif

DIRS = \
	avxcommon                          \
	avxsynth/builtinfunctions          \
	avxsynth/core                      \
	plugins/autocrop                   \
	plugins/avxffms2                   \
	plugins/avxframecapture            \
	plugins/avxsubtitle                \
	apps/avxframeserver/frameserverapp \
	apps/AVXEdit

INSTALLED = \
	$(libdir)/libavxcommon$(SONAME)           \
        $(libdir)/libavxbtinfncs$(SONAME)         \
	$(libdir)/libavxsynth$(SONAME)            \
	$(pluginsdir)/libautocrop$(SONAME)        \
	$(pluginsdir)/libavxffms2$(SONAME)        \
	$(pluginsdir)/libavxframecapture$(SONAME) \
	$(pluginsdir)/libavxsubtitle$(SONAME)     \
	$(bindir)/avxFrameServer$(EXE)            \
	$(bindir)/AVXEdit$(EXE)

avxcommon:
	$(MAKE) -f $(SRCPATH)/avxcommon/Makefile -C avxcommon
builtinfunctions: avxcommon
	$(MAKE) -f $(SRCPATH)/avxsynth/builtinfunctions/Makefile -C avxsynth/builtinfunctions
core: avxcommon builtinfunctions
	$(MAKE) -f $(SRCPATH)/avxsynth/core/Makefile -C avxsynth/core
autocrop: avxcommon
	$(MAKE) -f $(SRCPATH)/plugins/autocrop/Makefile -C plugins/autocrop
avxffms2: avxcommon
	$(MAKE) -f $(SRCPATH)/plugins/avxffms2/Makefile -C plugins/avxffms2
avxframecapture: avxcommon
	$(MAKE) -f $(SRCPATH)/plugins/avxframecapture/Makefile -C plugins/avxframecapture
avxsubtitle: avxcommon
	$(MAKE) -f $(SRCPATH)/plugins/avxsubtitle/Makefile -C plugins/avxsubtitle
frameserverlib: core
	$(MAKE) -f $(SRCPATH)/apps/avxframeserver/frameserverlib/Makefile -Capps/avxframeserver/frameserverlib
avxframeserver: frameserverlib
	$(MAKE) -f $(SRCPATH)/apps/avxframeserver/frameserverapp/Makefile -C apps/avxframeserver/frameserverapp
avxedit: frameserverlib
	$(MAKE) -f $(SRCPATH)/apps/AVXEdit/Makefile -C apps/AVXEdit

core-install: core
	$(MAKE) -f $(SRCPATH)/avxcommon/Makefile -C avxcommon install
	$(MAKE) -f $(SRCPATH)/avxsynth/builtinfunctions/Makefile -C avxsynth/builtinfunctions install
	$(MAKE) -f $(SRCPATH)/avxsynth/core/Makefile -C avxsynth/core install
autocrop-install: autocrop
	$(MAKE) -f $(SRCPATH)/plugins/autocrop/Makefile -C plugins/autocrop install
avxffms2-install: avxffms2
	$(MAKE) -f $(SRCPATH)/plugins/avxffms2/Makefile -C plugins/avxffms2 install
avxframecapture-install: avxframecapture
	$(MAKE) -f $(SRCPATH)/plugins/avxframecapture/Makefile -C plugins/avxframecapture install
avxsubtitle-install: avxsubtitle
	$(MAKE) -f $(SRCPATH)/plugins/avxsubtitle/Makefile -C plugins/avxsubtitle install
avxframeserver-install: avxframeserver
	$(MAKE) -f $(SRCPATH)/apps/avxframeserver/frameserverapp/Makefile -C apps/avxframeserver/frameserverapp install
avxedit-install: avxedit
	$(MAKE) -f $(SRCPATH)/apps/AVXEdit/Makefile -C apps/AVXEdit install

clean:
	for dir in $(DIRS); do \
		$(MAKE) -f $(SRCPATH)/$$dir/Makefile -C $$dir clean; \
	done

distclean: clean
	-rm config.log config.mak config.targets avxsynth.pc

test:
	echo "Testing..."

uninstall:
	-rm $(INSTALLED)
	-rmdir $(pluginsdir)
	-rmdir $(libdir)

.PHONY: avxcommon builtinfunctions core autocrop avxffms2 avxframecapture \
	avxsubtitle avxframeserver avxedit core-install autocrop-install \
	avxffms2-install avxframecapture-install avxsubtitle-install \
	avxframeserver-install avxedit-install default install clean distclean \
	test uninstall
