# This script tests the ConvertToYV12 function
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
# Output will be 2 frames long, each frame divided into
# four quadrants (NW, NE, SW, SE).
#
# When converted to RGB with the appropriate transform
# (Rec.601, TV levels, interlaced), the output should
# match the reference images converttoyv12-REF1.png and
# converttoyv12-REF2.png

FFVideoSource("../testclips/RGB-test-pattern.avi")
r = Crop(0, 0, 0, 240)
g = Crop(0, 240, 0, 240)
b = Crop(0, 480, 0, 0)
MergeRGB(r, g, b)
x = StackHorizontal(ConvertToYV12(matrix="Rec709", interlaced=true), ConvertToYV12(matrix="Rec601", interlaced=true))
y = StackHorizontal(ConvertToYV12(matrix="Rec709", interlaced=false), ConvertToYV12(matrix="Rec601", interlaced=false))
StackVertical(x, y)
a = last

ColorBars(width=320, height=240).KillAudio()
x = StackHorizontal(ConvertToYV12(matrix="PC.709", interlaced=true), ConvertToYV12(matrix="PC.601", interlaced=true))
y = StackHorizontal(ConvertToYV12(matrix="PC.709", interlaced=false), ConvertToYV12(matrix="PC.601", interlaced=false))
StackVertical(x, y)
Trim(0, -1)
AssumeFPS(a)
b = last

a + b
