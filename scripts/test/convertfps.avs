# This script tests the ConvertFPS() function
#
# Expected behavior:
#
# 3 frames, in this order
# white, 50% grey, black

x = BlankClip(color=$FFFFFF, fps=24, width=320, height=240, length=1)
y = BlankClip(color=$000000, fps=24, width=320, height=240, length=1)

x + y + x + y
ConvertFPS(16)
KillAudio()