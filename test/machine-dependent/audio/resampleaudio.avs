# This script test the ResampleAudio function.
#
# Expected behavior:
#
# A smooth tone.

a = Tone(samplerate=24000, length=2.0)
b = Tone(samplerate=44100, length=1.0)
c = Tone(samplerate=48000, length=2.0)
d = Tone(samplerate=96000, length=2.0)
ResampleAudio(a, 48000) + ResampleAudio(b, 48000) + ResampleAudio(b, 48000*44100, 44100) +  ResampleAudio(c, 48000) + ResampleAudio(d, 48000)
