src_movie = FFVideoSource(source="/home/encodingtools/TestStreams/DestinationTruth/DestinationTruth.mpg", colorspace="yv12")
#src_movie

src_width  = width(src_movie)
src_height = height(src_movie)
scaled_movie = SWScale(src_movie, src_width/3, src_height/3, "LANCZOS")
soften_movie = SpatialSoften(scaled_movie.ConvertToYUY2(), 4, 8, 8)
soften_movie = ConvertToYV12(soften_movie)
StackHorizontal(StackVertical(scaled_movie, Blur(scaled_movie, 1.58)), StackVertical(soften_movie, Sharpen(scaled_movie, 1)))
