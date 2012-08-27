# This script tests the RGBAdjust() function
#
# Expected behavior:
#
# 6 frames total.  The expected output is provided in inline comments
# Compliance can be checked with the command RGBAdjust(analyze=true).
# Alpha should remain unchanged at $808080.

o = BlankClip(pixel_type="YV12", length=1, height=240, color_yuv=$808080)
o1 = MergeARGB(o, o, o, o) # BlankClip can't set the alpha

# Testing scaling parameters, 50% gain, expected average: 192, 64
RGBAdjust(o1, r=1.5, g=1.5, b=1.5, a=1.5)
last + RGBAdjust(o1, r=0.5, g=0.5, b=0.5, a=0.5)

# Testing bias parameters, +/- 64, expected average: 192, 64
last + RGBAdjust(o1, rb=64, gb=64, bb=64, ab=64)
last + RGBAdjust(o1, rb=-64, gb=-64, bb=-64, ab=-64)

# Testing gamma parameter 1.5, 0.5, expected average: 161, 64
last + RGBAdjust(o1, rg=1.5, gg=1.5, bg=1.5, ag=1.5)
last + RGBAdjust(o1, rg=0.5, gg=0.5, bg=0.5, ag=0.5)
