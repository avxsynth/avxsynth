#ErrorHandlingExternal

clip0 	= Tone(frequency=440)
#clip0L = GetLeftChannel(clip0)
clip0L  = GetChannels(clip0, 1)

clip1 	= Tone(frequency=700)
#clip1R	= GetRightChannel(clip1)
clip1R  = GetChannels(clip1, 2)

clip2   = MergeChannels(clip0L, clip1R)
clip3   = ConvertAudioTo32Bit(clip2)
AmplifydB(clip3, -12.0)


