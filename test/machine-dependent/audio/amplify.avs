# This script tests the Amplify() function.
#
# Expected behavior:
#
# Three tones will be played.
# The first tone will have reduced volume on the left channel.
# The second tone will be balanced.
# The third tone will have increased volume on the right channel.
#
# Limitations:
#
# Negative gains (phase inversion) are not tested.

o = Tone(length=3.0, level=0.25)
a = o.Amplify(0.1, 1.0)
b = o
c = o.Amplify(1.0, 10)

a + b + c
