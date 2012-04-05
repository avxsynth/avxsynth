src_movie = FFVideoSource(source="/home/encodingtools/TestStreams/DestinationTruth/DestinationTruth.mpg", colorspace="yv12")
#src_movie

src_width  = width(src_movie)
src_height = height(src_movie)
scaled_movie = SWScale(src_movie, src_width/3, src_height/3, "LANCZOS")

StackVertical(StackHorizontal(TurnLeft(scaled_movie), TurnRight(scaled_movie)), StackHorizontal(Turn180(scaled_movie), BlankClip(width=80,height=360, pixel_type="YV12")))

