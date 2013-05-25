# This script tests the Subtract() function in YUV
#
# Expected behavior:
#
# A grey ($9AACAC) frame, confirm with a hex editor

# Subtract formula: 0.5 + a - b on normalized [0, 1] range
a = BlankClip(pixel_type="YV12", color_yuv=$EBF0F0, width=320, height=240, length=1) # 1.0
b = BlankClip(pixel_type="YV12", color_yuv=$CFC4C4, width=320, height=240, length=1) # 0.875
Subtract(a, b) # 0.5 + 1.0 - 0.875 == 0.625 (Y: 7E + EB - CF == 9A, UV: 80 + F0 - C4 == AC)
KillAudio()
