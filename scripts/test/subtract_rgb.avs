# This script tests the Subtract() function in RGB
#
# Expected behavior:
#
# A grey ($A0A0A0) frame

# Subtract formula: 0.5 + a - b on normalized [0, 1] range
a = BlankClip(color=$FFFFFF, width=320, height=240, length=1) # 1.0
b = BlankClip(color=$DFDFDF, width=320, height=240, length=1) # 0.875
Subtract(a, b) # 0.5 + 1.0 - 0.875 == 0.625 (80 + FF - DF == A0)
KillAudio()