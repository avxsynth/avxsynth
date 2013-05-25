# This script tests the ResetMask() and ShowAlpha() functions
#
# Expected behavior:
#
# A white ($FFFFFF) frame.

BlankClip(length=1, width=320, height=240)
ResetMask()
ShowAlpha()
KillAudio()
