src_movie = FFVideoSource(source="/home/encodingtools/TestStreams/DestinationTruth/DestinationTruth.mpg", colorspace="yv12")
#src_movie

src_width  = width(src_movie)
src_height = height(src_movie)
scaled_movie = SWScale(src_movie, src_width/3, src_height/3, "LANCZOS")
scaled_movie = ConvertToYUY2(scaled_movie)

StackVertical(scaled_movie, PeculiarBlend(scaled_movie, 180))

