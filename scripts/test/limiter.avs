# This script tests the Limiter() function
#
# Expected behavior:
#
# Five frames, as described below:
# 0. Left: pink ($DDDDDD), right: dark green ($121212)
# 1. Top-left: green, top-right: red, bottom: same as 0
# 2. Top-left: green, top-right: red, bottom: white/black
# 3. Top: yellow, bottom: same as 0
# 4. Top-left: purple, top-right: green, bottom: same as 2

a = BlankClip(pixel_type="YV12", width=160, height=120, length=1, color_yuv=$DEDEDE)
b = BlankClip(pixel_type="YV12", width=160, height=120, length=1, color_yuv=$111111)
c = BlankClip(pixel_type="YV12", width=160, height=120, length=1, color_yuv=$DDDDDD)
d = BlankClip(pixel_type="YV12", width=160, height=120, length=1, color_yuv=$121212)
o = StackVertical(StackHorizontal(a, b), StackHorizontal(c, d))
Limiter(o, 18, 221, 18, 221)
last + Limiter(o, 18, 221, 18, 221, show="luma")
last + Limiter(o, 18, 221, 18, 221, show="luma_grey")
last + Limiter(o, 18, 221, 18, 221, show="chroma")
last + Limiter(o, 18, 221, 18, 221, show="chroma_grey")
KillAudio()