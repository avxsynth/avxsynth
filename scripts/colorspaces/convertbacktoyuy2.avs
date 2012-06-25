# This script tests the ConvertBackToYUY2() function
#
# Expected behavior:
#
# Two sets of ColorBars will be displayed. There will
# be visible bleeding between the bars on the left set
# but not on the right.
#
# The original chroma positioning bug appears to have
# been fixed so this test does not work on avxsynth.

o = ColorBars(width=320, height=480, pixel_type="YUY2").KillAudio()

o
ConvertToRGB32()
ConvertToYUY2()
ConvertToRGB32()
ConvertToYUY2()
ConvertToRGB32()
ConvertToYUY2()
ConvertToRGB32()
ConvertToYUY2()
ConvertToRGB32()
ConvertToYUY2()
a = last

o
ConvertToRGB32()
ConvertBackToYUY2()
ConvertToRGB32()
ConvertBackToYUY2()
ConvertToRGB32()
ConvertBackToYUY2()
ConvertToRGB32()
ConvertBackToYUY2()
ConvertToRGB32()
ConvertBackToYUY2()
b = last

StackHorizontal(a, b)
Trim(0, -1)
