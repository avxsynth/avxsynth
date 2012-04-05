# This script tests the ConvertAudio family of functions.
#
# Expected behavior:
#
# A single tone. The first two seconds will be noisy.

Tone(length=2.0)
a = ConvertAudioTo8Bit().ConvertAudioTo16Bit()
b = ConvertAudioTo24Bit().ConvertAudioTo16Bit()
c = ConvertAudioTo32Bit().ConvertAudioTo16Bit()
d = ConvertAudioToFloat().ConvertAudioTo16Bit()
a + b + c + d
