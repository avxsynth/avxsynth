# This script tests the Animate() function
#
# Expected behavior:
#
# Three frames of colorbars. The last two are upside-down.

ColorBars().KillAudio()
Trim(0, 2)
ApplyRange(1, 2, "FlipVertical")
