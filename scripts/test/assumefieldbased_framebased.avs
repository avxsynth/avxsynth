# This script tests the AssumeFieldBased() and AssumeFrameBased() functions.
#
# Expected behavior:
#
# 2 frames at 320x240, info display indicating video is NOT field-based.

BlankClip(width=320, height=240).KillAudio()
Trim(0, -1)
AssumeFieldBased()
Weave()
SeparateFields()
AssumeFrameBased()
Info()
