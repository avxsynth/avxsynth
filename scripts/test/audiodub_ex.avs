# This script tests the AudioDub() and AudioDubEx() functions
#
# Expected behavior:
# Seconds 0-5 - black, high frequency
# Seconds 6-10 - white, low frequency

a1 = BlankClip(color=$FFFFFF, length=120, width=16, height=16).KillAudio()
a2 = BlankClip(color=$000000, length=120, width=16, height=16).KillAudio()
b1 = Tone(frequency=440, length=5.0)
b2 = Tone(frequency=880, length=5.0)

c1 = AudioDubEx(a1, a2)
c2 = AudioDubEx(a2, a1)
d1 = AudioDubEx(b1, b2)
d2 = AudioDubEx(b2, b1)

AudioDub(c1, d1) + AudioDub(c2, d2)
