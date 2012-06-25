src_movie = FFVideoSource(source="/home/encodingtools/Video/DestinationTruth/DestinationTruth.mpg", colorspace="yv12")
#src_movie

#scaled_movie = ReduceBy2(src_movie)
#scaled_movie = HorizontalReduceBy2(src_movie)
scaled_movie = VerticalReduceBy2(src_movie)
scaled_movie

