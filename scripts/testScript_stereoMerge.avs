#ErrorHandlingExternal

clip0 	= Tone(frequency=440)
clip0L  = GetChannels(clip0, 1)

clip1 	= Tone(frequency=700)
clip1R  = GetChannels(clip1, 2)

clip2   = MergeChannels(clip0L, clip1R)

clip3 	= Tone(frequency=440)
clip3L  = GetChannels(clip3, 1)

clip4 	= Tone(frequency=700)
clip4R  = GetChannels(clip4, 2)

clip5   = MergeChannels(clip3L, clip4R)

MergeChannels(clip2, clip5)


