src_movie = FFVideoSource(source="/home/encodingtools/TestStreams/ElephantsDream/ElephantsDream-avc.mpg", colorspace="yv12")
src_width = width(src_movie)
MessageClip(string(src_width))

