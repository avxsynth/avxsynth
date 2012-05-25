# This script tests the DoubleWeave() function.
#
# Expected behavior:
# 
# First frame: interwoven green/red (green on top)
# Second frame: interwoven green/blue (green on top)

a = BlankClip(length=1, color=$FF0000, width=320, height=120)
b = BlankClip(length=1, color=$00FF00, width=320, height=120)
c = BlankClip(length=1, color=$0000FF, width=320, height=120)

a + b + c
AssumeFieldBased()
AssumeBFF()
DoubleWeave()
Trim(0, 1)
KillAudio()