# This script tests the SelectEvery family of functions
#
# Expected behavior:
#
# 8 frames total, in the following order
# white, green, white, green, red, blue, red, green

a = BlankClip(color=$FFFFFF, width=320, height=240, length=1)
b = BlankClip(color=$FF0000, width=320, height=240, length=1)
c = BlankClip(color=$00FF00, width=320, height=240, length=1)
d = BlankClip(color=$0000FF, width=320, height=240, length=1)
Interleave(a, b, c, d)
a = SelectEvery(4, 0, 2)
b = SelectEven()
c = SelectOdd()
d = SelectRangeEvery(4, 2, 1).AssumeFPS(c)
a + b + c + d
KillAudio()
