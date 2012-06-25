# This script tests the AssumeTFF/BFF, Weave(), and ComplementParity() functions.
# Expected behavior:
#
# Interwoven lines of white and black.
# Left is black on top, right is white on top.

a = BlankClip(length=1, color=$000000, width=160, height=120)
b = BlankClip(length=1, color=$FFFFFF, width=160, height=120)

a + b
AssumeFieldBased()
StackHorizontal(AssumeBFF().ComplementParity().Weave(), AssumeTFF().ComplementParity().Weave())
KillAudio()
