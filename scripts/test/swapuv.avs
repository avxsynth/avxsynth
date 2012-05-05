# This script tests the SwapUV() function
#
# Expected behavior:
#
# A blue ($80C040) frame

BlankClip(pixel_type="YV12", color_yuv=$8040C0, width=320, height=240, length=1)
SwapUV()
KillAudio()