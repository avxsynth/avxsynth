# This script tests the Interleave() and SwapFields() functions
#
# Expected behavior:
#
# Two frames, alternating lines of black and white, white on top

a = BlankClip(width=320, height=120, color=$FFFFFF, length=2)
b = BlankClip(width=320, height=120, color=$000000, length=2)
Interleave(a, b)
AssumeFieldBased()
AssumeBFF()
Weave()
SwapFields()
KillAudio()
