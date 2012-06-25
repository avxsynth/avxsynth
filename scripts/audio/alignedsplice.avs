# This script tests the AlignedSplice() function
#
# Expected behavior:
# 2.5 second of tone, 2.5 seconds silence, 2.5 seconds tone

a = BlankClip(length=5*24).KillAudio()
b = Tone(length=2.5)

AlignedSplice(AudioDub(a, b), AudioDub(a, b))
KillVideo()
