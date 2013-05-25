# This script tests the Crop/AddBorders family of functions.
#
# Expected behavior:
#
# Two frames, identically 320x240 ColorBars, with the outermost 
# 16 pixels blanked out in red.

a = ColorBars(width=320, height=240).Trim(0, -1)

Crop(a, 16, 16, -16, 240 - 24)
CropBottom(8)
AddBorders(16, 16, 16, 16, color=$FF0000)
x = last

LetterBox(a, 16, 16, 16, 16, color=$FF0000)
y = last

x + y
KillAudio()
