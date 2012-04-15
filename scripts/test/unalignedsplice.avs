# This script tests the UnalignedSplice() function
#
# Expected behavior:
# 5 seconds tone

a = BlankClip(length=5*24).KillAudio()
b = Tone(length=2.5)

UnalignedSplice(AudioDub(a, b), AudioDub(a, b))
KillVideo()