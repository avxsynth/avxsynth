# This script tests the ReduceBy2 family of functions
#
# Test clip:
#
# Encoding is YV12 in FFV1. Each plane of a RGB24
# clip is encoded as a horizontal slice. This is due
# to a bug in ffms2 that prevents opening uncompressed
# AVIs
#
# The clip consists of a 1:4 density grid of $FF0000
# red pixels on a $000000 black background.
#
# Expected behavior:
#
# Two identical 160x120 maroon ($400000) frames with a single
# black ($000000) line at the top.

FFVideoSource("../testclips/RGB-test-pattern.avi")
r = Crop(0, 0, 0, 240)
g = Crop(0, 240, 0, 240)
b = Crop(0, 480, 0, 0)
MergeRGB(r, g, b)

a = HorizontalReduceBy2().VerticalReduceBy2()
b = ReduceBy2()
a + b
