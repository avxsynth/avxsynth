# This script tests the Info() function
#
# Expected behavior:
# Overlays informational text about the video clip, including the line
# FieldBased (Separated) Video: YES

ColorBars().KillAudio().Trim(0, -1)
SeparateFields()
Info()
