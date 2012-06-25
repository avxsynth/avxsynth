# This script tests the SeparateFields() function.
#
# Expected behavior:
#
# Two 1x2 frames. First is black on white, second is white on black.

StackVertical(BlankClip(height=1, color=$FFFFFF, width=1, length=1), BlankClip(height=1, width=1, color=$000000, length=1))
StackVertical(AssumeBFF().SeparateFields(), AssumeTFF().SeparateFields())
KillAudio()
