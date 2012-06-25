# This script tests the Blur() and Sharpen() functions
#
# Expected behavior:
#
# Four frames, in this order
# 1. ColorBars
# 2. Horizontally blurred
# 3. Blurred
# 4. Sharpened

ColorBars(width=320, height=240).Trim(0, -1)
a = Blur(0)
b = Blur(1.58, 0)
c = Blur(1.58)
d = Sharpen(1.0)
a + b + c + d
KillAudio()
