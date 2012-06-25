# This script tests the MixAudio() function
#
# Expected behavior:
#
# Three tones of decreasing pitch

a = Tone(frequency=880, length=3.0)
b = Tone(frequency=440, length=3.0)
MixAudio(a, b, 0.75) + MixAudio(a, b, 0.5) + MixAudio(a, b, 0.25)
