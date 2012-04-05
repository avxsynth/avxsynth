src_movie = FFVideoSource(source="/home/encodingtools/TestStreams/DestinationTruth/DestinationTruth.mpg", colorspace="rgb32")
#src_movie

src_width  = width(src_movie)
src_height = height(src_movie)
scaled_movie = SWScale(src_movie, src_width/3, src_height/3, "LANCZOS")

#soften_movie = SpatialSoften(scaled_movie.ConvertToYUY2(), 4, 8, 8)
#soften_movie = ConvertToYV12(soften_movie)
StackVertical(scaled_movie, ShowGreen(scaled_movie))
