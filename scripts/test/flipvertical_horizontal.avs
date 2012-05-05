# This script tests the FlipVertical() and FlipHorizontal() functions.
#
# Expected behavior:
#
# ColorBars, but upside down and backwards (blue stripe on bottom-left).

ColorBars(width=320, height=240)
Trim(0, -1)
FlipVertical()
FlipHorizontal()
KillAudio()
