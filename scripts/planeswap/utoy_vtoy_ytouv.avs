# This script tests the UToY(), VToY(), and YToUV() functions
#
# Expected behavior:
#
# An orange ($8040C0) frame

a = BlankClip(color_yuv=$404040, pixel_type="YV12", width=320, height=240, length=1)
b = BlankClip(color_yuv=$C0C0C0, pixel_type="YV12", width=320, height=240, length=1)
c = BlankClip(color_yuv=$808080, pixel_type="YV12", width=320, height=240, length=1)

x = UToY(a)
y = VtoY(b)
YToUV(x, y, c)
KillAudio()
