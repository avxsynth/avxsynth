# Use this to test if explicit plugin loading works.

LoadPlugin(tests_build_dir + "tools/.libs/dummy_plugin.so")

dummy_function(BlankClip(width=1, height=1, length=1, pixel_type="RGB24", color=$123456))
