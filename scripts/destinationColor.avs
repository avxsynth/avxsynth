src_movie = FFVideoSource(source="/home/encodingtools/Video/DestinationTruth/DestinationTruth.mpg", colorspace="yv12")
#src_movie

src_width  = width(src_movie)
src_height = height(src_movie)
src_movie = SWScale(src_movie, src_width/3, src_height/3, "LANCZOS")

# This will adjust gamma for luma, while making luma smaller and chroma U greater:
StackVertical(src_movie, ColorYUV(src_movie, gamma_y=128, off_y=-16, off_u=5))

# Shows all colors. Frame 0 luma is 16, frame 1 luma is 17 and so on:
#ColorYUV(showyuv=true)