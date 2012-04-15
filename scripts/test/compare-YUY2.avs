# This script tests the Compare function in YUY2
#
# Test clip:
#
# Encoding is YV12 in FFV1. The luma channel of a YUY2 clip
# is on top, with the U and V chroma channels on the bottom-
# left and bottom-right, respectively. This is due
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
# 0 deviation between Y channel on frames 0,2
# 30.14 dB PSNR between U+V channel on frames 0,2 
# 30.59 dB PSNR on frames 0,1
# 34.10 dB PSNR between A channel on frames 1,2
# These values will be reported in logfiles

o = FFVideoSource("testclips/compare-YUY2-as-YV12.avi")
y = Crop(o, 0, 0, 0, 120)
u = Crop(o, 0, 120, 80, 0)
v = Crop(o, 80, 120, 0, 0)
MergeChroma(ConvertToYUY2(y), YToUV(ConvertToYUY2(u), ConvertToYUY2(v)))
a = Trim(0, -1)
b = Trim(1, 1)
c = Trim(2, 2)
comp1 = Compare(a, c, "Y", logfile="compare-YUY2-AC-Y.log")
comp2 = Compare(a, c, "UV", logfile="compare-YUY2-AC-UV.log")
comp3 = Compare(a, b, "", logfile="compare-YUY2-AB-YUV.log")
comp4 = Compare(b, c, "", logfile="compare-YUY2-BC-YUV.log")
StackVertical(comp1, comp2, comp3, comp4)