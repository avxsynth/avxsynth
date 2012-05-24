# This script tests the ShowRed() / ShowGreen() / ShowBlue() family of functions
#
# Expected behavior:
#
# Three frames, with values $404040, $808080, $B0B0B0

BlankClip(width=320, height=240, color=$4080B0, length=1)
ShowRed() + ShowGreen() + ShowBlue()
KillAudio()
