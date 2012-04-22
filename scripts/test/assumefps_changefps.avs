# This script tests the ChangeFPS() function
#
# Expected behavior:
#
# 10 frames, in this order:
# white x3, black x2, white x3, black x2
#
# Clip framerate is 24000/1001 (23.976)

x = BlankClip(color=$FFFFFF, fps=23.976, width=320, height=60, length=1)
y = BlankClip(color=$000000, fps=23.976, width=320, height=60, length=1)

x + y + x + y
a = ChangeFPS(59.940)
b = ChangeFPS(60000, 1001)
c = ChangeFPS(b)
d = ChangeFPS("ntsc_double")
StackVertical(a, b, c, d)
AssumeFPS(24000, 1001)
KillAudio()
