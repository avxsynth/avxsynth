src_movie = FFVideoSource(source="/home/encodingtools/TestStreams/DestinationTruth/DestinationTruth.mpg", colorspace="yv12")
#src_movie

src_width  = width(src_movie)
src_height = height(src_movie)
scaled_movie = SWScale(src_movie, src_width/3, src_height/3, "LANCZOS")
scaled_movieRGB = ConvertToRGB32(scaled_movie)
StackHorizontal(StackVertical(scaled_movie, Limiter(scaled_movie, 50, 200, 50, 200)), StackVertical(Tweak(scaled_movie, bright=100), ConvertToYV12(RGBAdjust(scaled_movieRGB, 2, 1, 1, 1))))