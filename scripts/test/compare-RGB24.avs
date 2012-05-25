# This script tests the Compare function in RGB24
#
# Test clip:
#
# Encoding is YV12 in FFV1. Each plane of an RGB24
# clip is encoded as a horizontal slice. This is due
# to a bug in ffms2 that prevents opening uncompressed
# AVIs
#
# The clip contains three frames.
# Frame 0: ColorBars
# Frame 1: ColorBars blurred
# Frame 2: ColorBars with R and B channels blurred
#
# Expected behavior:
#
# 0 deviation between G channel on frames 0,2
# 27.74 dB PSNR between R+B channel on frames 0,2 
# 28.20 dB PSNR on frames 0,1
# 34.08 dB PSNR on frames 1,2
# These values will be reported in logfiles

o = FFVideoSource("testclips/compare-RGB24-as-YV12.avi")
r = Crop(o, 0, 0, 0, 120)
g = Crop(o, 0, 120, 0, 120)
b = Crop(o, 0, 240, 0, 0)
MergeRGB(r, g, b)
a = Trim(0, -1)
b = Trim(1, 1)
c = Trim(2, 2)
comp1 = Compare(a, c, "G", logfile="compare-RGB24-AC-G.log")
comp2 = Compare(a, c, "RB", logfile="compare-RGB24-AC-RB.log")
comp3 = Compare(a, b, "", logfile="compare-RGB24-AB-RGB.log")
comp4 = Compare(b, c, "", logfile="compare-RGB24-BC-RGB.log")
StackVertical(comp1, comp2, comp3, comp4)