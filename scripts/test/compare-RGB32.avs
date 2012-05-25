# This script tests the Compare function in RGB32
#
# Test clip:
#
# Encoding is YV12 in FFV1. Each plane of an RGB32
# clip is encoded as a horizontal slice. This is due
# to a bug in ffms2 that prevents opening uncompressed
# AVIs
#
# The clip contains three frames.
# Frame 0: ColorBars, no alpha
# Frame 1: ColorBars blurred, no alpha
# Frame 2: ColorBars with R and B channels blurred and alpha
#
# Expected behavior:
#
# 0 deviation between G channel on frames 0,2
# 27.74 dB PSNR between R+B channel on frames 0,2 
# 28.20 dB PSNR on frames 0,1
# 8.07 dB PSNR between A channel on frames 1,2
# These values will be reported in logfiles

o = FFVideoSource("testclips/compare-RGB32-as-YV12.avi")
a = Crop(o, 0, 0, 0, 120)
r = Crop(o, 0, 120, 0, 120)
g = Crop(o, 0, 240, 0, 120)
b = Crop(o, 0, 360, 0, 0)
MergeARGB(a, r, g, b)
PointResize(width*2, height*2)
a = Trim(0, -1)
b = Trim(1, 1)
c = Trim(2, 2)
comp1 = Compare(a, c, "G", logfile="compare-RGB32-AC-G.log")
comp2 = Compare(a, c, "RB", logfile="compare-RGB32-AC-RB.log")
comp3 = Compare(a, b, "", logfile="compare-RGB32-AB-RGB.log")
comp4 = Compare(b, c, "A", logfile="compare-RGB32-BC-A.log")
StackVertical(comp1, comp2, comp3, comp4)