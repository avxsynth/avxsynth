# This script tests the Compare function in YV12
#
# Test clip:
#
# Encoding is YV12 in FFV1.
#
# The clip contains three frames.
# Frame 0: ColorBars, no alpha
# Frame 1: ColorBars blurred, no alpha
# Frame 2: ColorBars with R and B channels blurred and alpha
#
# Expected behavior:
#
# 0 deviation between Y channel on frames 0,2
# 29.63 dB PSNR between U+V channel on frames 0,2 
# 30.54 dB PSNR on frames 0,1
# 32.85 dB PSNR between A channel on frames 1,2
# These values will be reported in logfiles

FFVideoSource("testclips/compare-YV12.avi")
a = Trim(0, -1)
b = Trim(1, 1)
c = Trim(2, 2)
comp1 = Compare(a, c, "Y", logfile="compare-YV12-AC-Y.log")
comp2 = Compare(a, c, "UV", logfile="compare-YV12-AC-UV.log")
comp3 = Compare(a, b, "", logfile="compare-YV12-AB-YUV.log")
comp4 = Compare(b, c, "", logfile="compare-YV12-BC-YUV.log")
StackVertical(comp1, comp2, comp3, comp4)