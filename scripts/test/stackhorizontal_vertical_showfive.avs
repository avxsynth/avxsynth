# This script tests the
# StackHorizontal / StackVertical / ShowFiveVersions functions
#
# Expected output:
#
# Frame 1 - white on left, black on right
# Frame 2 - white on top, black on bottom
# Frame 3 - alternating stripes, top: w/b/w, bottom: w/b

a = BlankClip(color=$FFFFFF, width=240, length=1)
b = BlankClip(color=$000000, width=240, length=1)
c = BlankClip(color=$FFFFFF, width=480, height=240, length=1)
d = BlankClip(color=$000000, width=480, height=240, length=1)
e = BlankClip(color=$FFFFFF, width=480/3, height=240, length=1)
f = BlankClip(color=$000000, width=480/3, height=240, length=1)

StackHorizontal(a, b) + StackVertical(c, d) + ShowFiveVersions(e, e, f, f, e)
KillAudio()