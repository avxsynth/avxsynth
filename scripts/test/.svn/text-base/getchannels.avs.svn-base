# This script tests the GetChannel family of functions.
#
# Expected behavior:
#
# A series of tones in the following order.
# 1. A low frequency tone.
# 2. A high frequency tone.
# 3. Low frequency on the left, high frequency on the right.
# 4. High frequency on the left, low frequency on the right.

a = Tone(frequency=440, channels=1, length=2.0)
b = Tone(frequency=880, channels=1, length=2.0)
c = MergeChannels(a, b)

x1 = GetLeftChannel(c)
y1 = GetChannel(c, 1)
x2 = GetRightChannel(c)
y2 = GetChannel(c, 2)
z  = GetChannels(c, 2, 1)
MergeChannels(x1, y1) + MergeChannels(x2, y2) + c + z
