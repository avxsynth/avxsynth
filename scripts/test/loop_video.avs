# This script tests the Loop() function 
#
# Expected behavior:
#
# 5 seconds of tone + blankclip
# Frames 24-95 are white, the rest black.

a = BlankClip(color=$000000, length=24, width=320, height=240)
b = BlankClip(color=$FFFFFF, length=24, width=320, height=240)
AudioDub(a + b + a, Tone(length=3)) 
Loop(3, 24, 47)
KillAudio()
