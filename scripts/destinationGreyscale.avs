src_movie = FFVideoSource(source="/home/encodingtools/TestStreams/DestinationTruth/DestinationTruth.mpg", colorspace="rgb24")
#src_movie

src_width  = width(src_movie)
src_height = height(src_movie)
scaled_movie = SWScale(src_movie, src_width/3, src_height/3, "LANCZOS")
StackVertical(scaled_movie, greyscale(scaled_movie))
