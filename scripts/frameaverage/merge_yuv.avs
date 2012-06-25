# This script tests the Merge() function in YUV
#
# Expected behavior:
#
# A pink ($DFDFDF) frame

a = BlankClip(color_yuv=$FFFFFF, pixel_type="YV12", width=320, height=240, length=1)
b = BlankClip(color_yuv=$808080, pixel_type="YV12", width=320, height=240, length=1)

Merge(a, b, weight=0.25)
