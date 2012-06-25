#!/bin/sh

# Try to avoid wreaking havoc if this is run from the wrong path.
test ! -d tools/.libs/ && echo "a" && exit 1

# AvxSynth craps out if the autoload path has weird files.
test -f emptydir && echo "b" && exit 1
test -f autoload && echo "c" && exit 1
test -f output && echo "d" && exit 1
test ! -d emptydir && mkdir -p emptydir
test ! -d autoload && mkdir -p autoload
test ! -d output && mkdir -p output
rm -f autoload/* emptydir/*

# Due to some braindeadness in libtool, loading plugins by absolute path
# is broken when calling the libtool wrapper.
test ! -f ../apps/avxframeserver/avxFrameServer && echo "e" && exit 1
test ! -f ../plugins/avxffms2/.libs/libavxffms2.so && echo "f" && exit 1
cp ../plugins/avxffms2/.libs/libavxffms2.so autoload
cp tools/.libs/dummy_plugin2.so autoload
cp common.avsi autoload

# We must overwrite these to make sure a system copy of AvxSynth isn't used.
export PATH="../apps/avxframeserver:$PATH"
export AVXSYNTH_RUNTIME_PLUGIN_PATH=autoload
$@
