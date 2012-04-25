#ErrorHandlingExternal
f1 = 2*261.6
f2 = 2*329.6
f3 = 2*392.0
f4 = 2*f1

clip1 = Tone(frequency=f1)
clip2 = DelayAudio(Tone(frequency=f2), 1.0)
clip3 = MergeChannels(clip1, clip2)
clip4 = DelayAudio(Tone(frequency=f3), 2.0)
clip5 = MergeChannels(clip3, clip4)
clip6 = DelayAudio(Tone(frequency=f4), 3.0)
clip7 = MergeChannels(clip5, clip6)

clip8 = ConvertAudioTo16Bit(ConvertToMono(clip7))

#
# the result of the two functions below should be
# unchanged audio volume
#
clip9 = AmplifydB(clip8, -30.0)
Amplify(clip9, 31.62)

