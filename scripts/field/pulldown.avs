# This script tests the Pulldown() function
#
# Expected behavior:
#
# One white frame followed by one black frame.
# Info display indicates video is NOT fieldbased.


a = BlankClip(length=1, color=$FFFFFF, width=320, height=240)
b = BlankClip(length=1, color=$FF00FF, width=320, height=240)
c = BlankClip(length=1, color=$FFFF00, width=320, height=240)
d = BlankClip(length=1, color=$00FFFF, width=320, height=240)
e = BlankClip(length=1, color=$000000, width=320, height=240)
a + b + c + d + e
AssumeFieldBased()
Pulldown(0, 4)
Info()
KillAudio()
