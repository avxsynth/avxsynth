# This script tests the Merge() function in RGB
#
# Expected behavior:
#
# A grey ($DFDFDF) frame

a = BlankClip(color=$FFFFFF, width=320, height=240, length=1)
b = BlankClip(color=$808080, width=320, height=240, length=1)

Merge(a, b, weight=0.25)
KillAudio()
