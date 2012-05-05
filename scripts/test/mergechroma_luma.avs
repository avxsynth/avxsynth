# This script tests the MergeChroma() and MergeLuma() functions
#
# Expected behavior:
#
# A purple ($40BFBF) frame

a = BlankClip(pixel_type="YV12", color_yuv=$FFFFFF, width=320, height=240, length=1)
b = BlankClip(pixel_type="YV12", color_yuv=$000000, width=320, height=240, length=1)

MergeChroma(a, b, chromaweight=0.25)
MergeLuma(b, lumaweight=0.75)
KillAudio()