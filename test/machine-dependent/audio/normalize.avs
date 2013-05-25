# This script tests the normalize function.
#
# Expected behavior:
#
# Three tones of increasing volume.
#
# Limitations:
#
# bool "show" - Not currently testable due to lack of support for a+v

o = Tone(level=0.1, length=3.0)
a = o
b = Normalize(o, 0.5)
c = Normalize(o, 1.0)
a + b + c
