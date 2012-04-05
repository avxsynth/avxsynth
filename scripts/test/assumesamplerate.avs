# This script tests the AssumeSampleRate() function.
#
# Expected behavior:
#
# A smooth 10 second tone, identical to Tone().

a = Tone(frequency=880, length=2.5, samplerate=96000)
b = Tone(frequency=220, length=10, samplerate=24000)
AssumeSampleRate(a, 48000) + AssumeSampleRate(b, 48000)