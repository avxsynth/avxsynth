# This script tests the ConvertToMono function.
#
# Expected behavior:
#
# A smooth tone.
a = Tone(channels=2, length=3.0)
b = Tone(channels=4, length=3.0)
c = Tone(channels=6, length=3.0)
ConvertToMono(a) + ConvertToMono(b) + ConvertToMono(c)
