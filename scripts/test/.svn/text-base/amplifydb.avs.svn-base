# This script tests the AmplifyDB() function.
#
# Expected behavior:
#
# Three tones will be played.
# The first tone will have reduced volume on the left channel.
# The second tone will be balanced.
# The third tone will have increased volume on the right channel.

o = Tone(length=3.0, level=0.25)
a = o.AmplifyDB(-10, 0)
b = o
c = o.AmplifyDB(0, 10)

a + b + c
