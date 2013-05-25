# This script tests the MonoToStereo function.
#
# Expected behavior:
#
# Low pitch on left and high pitch on right.

a = Tone(frequency=440, channels=1)
b = Tone(frequency=880, channels=1)
MonoToStereo(a, b)
