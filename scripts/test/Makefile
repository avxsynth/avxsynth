include ../../config.mak

PATH=PATH=../../apps/avxframeserver/frameserverapp
LDPATH=LD_LIBRARY_PATH=../../avxsynth/core/:../../avxutils/:$${LD_LIBRARY_PATH}
PLUGINPATH=AVXSYNTH_RUNTIME_PLUGIN_PATH=../../plugins

default:
ifneq ($(SRCPATH),.)
	$(error Testing can not be done out-of-tree)
endif
	$(PATH) $(LDPATH) $(PLUGINPATH) ./AvxUnitTest.py AvxTestSuite-V.txt ../../avxtest-v.log
	$(PATH) $(LDPATH) $(PLUGINPATH) ./AvxUnitTest.py AvxTestSuite-A.txt ../../avxtest-a.log
