#                                       -*- Automake -*-
# Process this file with automake to produce a Makefile.	

TESTS = \
	tools/avs_c_interface              \
	AvxTestSuite-SANITY.test           \
	AvxTestSuite-audio.test            \
	AvxTestSuite-colors.test           \
	AvxTestSuite-colorspaces.test      \
	AvxTestSuite-conditional.test      \
	AvxTestSuite-field.test            \
	AvxTestSuite-filewriting.test      \
	AvxTestSuite-focus.test            \
	AvxTestSuite-frameaverage.test     \
	AvxTestSuite-geometry.test         \
	AvxTestSuite-planeswap.test        \
	AvxTestSuite-textrender.test       \
	AvxTestSuite-timeline.test

TEST_EXTENSIONS = .test

XFAIL_TESTS = AvxTestSuite-audio.test AvxTestSuite-filewriting.test AvxTestSuite-textrender.test

TEST_LOG_COMPILER = "$(srcdir)/env.sh" $(PYTHON) "$(srcdir)/AvxUnitTest.py"

AM_TEST_LOG_FLAGS = --save-files --source-chdir $(srcdir) --output-chdir output

# Run sanity test first.  Unfortunately, there is no way to exit if it fails.
AvxTestSuite-audio.log: AvxTestSuite-SANITY.log
AvxTestSuite-colors.log: AvxTestSuite-SANITY.log
AvxTestSuite-colorspaces.log: AvxTestSuite-SANITY.log
AvxTestSuite-conditional.log: AvxTestSuite-SANITY.log
AvxTestSuite-field.log: AvxTestSuite-SANITY.log
AvxTestSuite-filewriting.log: AvxTestSuite-SANITY.log
AvxTestSuite-focus.log: AvxTestSuite-SANITY.log
AvxTestSuite-frameaverage.log: AvxTestSuite-SANITY.log
AvxTestSuite-geometry.log: AvxTestSuite-SANITY.log
AvxTestSuite-planeswap.log: AvxTestSuite-SANITY.log
AvxTestSuite-textrender.log: AvxTestSuite-SANITY.log
AvxTestSuite-timeline.log: AvxTestSuite-SANITY.log

check_PROGRAMS = tools/avsgrabber tools/avs_c_interface

check_LTLIBRARIES = tools/dummy_plugin.la tools/dummy_plugin2.la

tools_avsgrabber_SOURCES = tools/avsgrabber.cpp

tools_avsgrabber_CPPFLAGS = -I$(top_srcdir)/include $(AM_CPPFLAGS)

tools_avsgrabber_LDADD = $(top_builddir)/avxsynth/libavxsynth.la

tools_avs_c_interface_SOURCES = tools/avs_c_interface.c

tools_avs_c_interface_CPPFLAGS = \
	-I$(top_srcdir)/avxsynth/core/src/core \
	-I$(top_srcdir)/include                \
	$(AM_CPPFLAGS)

tools_avs_c_interface_LDADD = $(top_builddir)/avxsynth/libavxsynth.la

tools_dummy_plugin_la_SOURCES = tools/dummy_plugin.cpp

tools_dummy_plugin_la_CXXFLAGS = $(AM_CXXFLAGS) -shared

tools_dummy_plugin_la_CPPFLAGS = -I$(top_srcdir)/include $(AM_CPPFLAGS)

# -rpath /nowhere/ is a hack to force libtool to actually link something.
tools_dummy_plugin_la_LDFLAGS = \
	$(AM_LDFLAGS) -module -shared -avoid-version -export-dynamic -rpath /nowhere/

tools_dummy_plugin2_la_SOURCES = tools/dummy_plugin2.cpp

tools_dummy_plugin2_la_CXXFLAGS = $(AM_CXXFLAGS) -shared

tools_dummy_plugin2_la_CPPFLAGS = -I$(top_srcdir)/include $(AM_CPPFLAGS)

# -rpath /nowhere/ is a hack to force libtool to actually link something.
tools_dummy_plugin2_la_LDFLAGS = \
	$(AM_LDFLAGS) -module -shared -avoid-version -export-dynamic -rpath /nowhere/

clean-local:
	-rm -fr emptydir autoload output testclips/*.ffindex *.pyc

# Automake doesn't know about our test scripts.
EXTRA_DIST = \
	testclips/compare-RGB24-as-YV12.avi \
	testclips/compare-RGB32-as-YV12.avi \
	testclips/compare-YUY2-as-YV12.avi  \
	testclips/compare-YV12.avi          \
	testclips/RGB-test-pattern.avi      \
	AvxFrameServer.py                   \
	AvxTestCommon.py                    \
	AvxUnitTest.py                      \
	AvxTestSuite-SANITY.test            \
	AvxTestSuite-audio.test             \
	AvxTestSuite-colors.test            \
	AvxTestSuite-colorspaces.test       \
	AvxTestSuite-conditional.test       \
	AvxTestSuite-field.test             \
	AvxTestSuite-filewriting.test       \
	AvxTestSuite-focus.test             \
	AvxTestSuite-geometry.test          \
	AvxTestSuite-planeswap.test         \
	AvxTestSuite-timeline.test          \
	autoloadplugin.avs                  \
	exception.avs                       \
	import.avs                          \
	loadplugin.avs                      \
	setworkingdir.avs                   \
	env.sh                              \
	audio                               \
	colors                              \
	colorspaces                         \
	conditional                         \
	examples                            \
	field                               \
	filewriting                         \
	focus                               \
	frameaverage                        \
	geometry                            \
	machine-dependent                   \
	planeswap                           \
	refimages                           \
	textrender                          \
	timeline
