# This script tests the MergeRGB() function
#
# Expected behavior:
#
# A bright pink frame ($FF00FF) stacked on top of a grey frame ($808080)

r = BlankClip(pixel_type="RGB32", color=$FF4040, width=320, height=240, length=1)
g = BlankClip(pixel_type="YUY2", color_yuv=$008080, width=320, height=240, length=1)
b = BlankClip(pixel_type="YV12", color_yuv=$FF8080, width=320, height=240, length=1)
a = BlankClip(pixel_type="YV12", color_yuv=$804040, width=320, height=240, length=1)

MergeARGB(a, r, g, b)
StackVertical(last, ShowAlpha())
KillAudio()
