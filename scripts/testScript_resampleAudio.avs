#ErrorHandlingExternal

clip0 	= Tone(frequency=440)
#clip0L = GetLeftChannel(clip0)
clip0L  = GetChannels(clip0, 1)

clip1 	= Tone(frequency=17000)
#clip1R	= GetRightChannel(clip1)
clip1R  = GetChannels(clip1, 2)

clip2   = MergeChannels(clip0L, clip1R)
clip3   = clip2 #ConvertAudioToFloat(clip2)
clip4   = ResampleAudio(clip3, 32000)
ConvertAudioTo16Bit(clip4)
#Normalize(clip3, 0.2)



