# This script tests the AssumeFieldBased() and AssumeFrameBased() functions.
#
# Expected behavior:
#
# 2 frames at 320x240.  An exception occurs on failure.

BlankClip(width=320, height=240).KillAudio()
Trim(0, -1)
AssumeFieldBased()
saved = last
Assert(IsFieldBased(), "must be field-based after AssumeFieldBased()")
saved

Weave()
SeparateFields()
AssumeFrameBased()
saved = last
Assert(IsFrameBased(), "must be frame-based after AssumeFrameBased()")
saved
