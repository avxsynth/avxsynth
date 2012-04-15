# This script tests the frame editing filters
#   Trim, FreezeFrame, DeleteFrame, DuplicateFrame
#
# Expected behavior:
# 7 frames, 0-1 white, 2-4 black, 5-6 white

a = BlankClip(color=$FFFFFF, length=5, width=320, height=240)
b = BlankClip(color=$000000, length=5, width=320, height=240)
a + b

Trim(3, 0) # black transition at frame 2
FreezeFrame(4, 6, 0) # replace frames 4-6 to white
DuplicateFrame(3) # insert black frame at position 4
DeleteFrame(7) # remove final black frame
KillAudio()