# This script tests the Turn() function
#
# Expected behavior:
#
# Three frames, in this order:
# 1) ColorBars rotated 90deg left
# 2) ColorBars rotated 90deg right
# 3) ColorBars rotated 180deg

ColorBars(width=320, height=320)
Trim(0, -1)

TurnLeft() + TurnRight() + Turn180()
KillAudio()