destination = FFVideoSource(source="/home/encodingtools/Video/DestinationTruth/DestinationTruth.mpg", colorspace="rgb32")
destination = SWScale(destination, 640, 360, "Lanczos")
blurredVideo = GeneralConvolution(destination, 0, "
   10 10 10 10 10
   10 10 10 10 10 
   10 10 16 10 10 
   10 10 10 10 10 
   10 10 10 10 10 ", 256, false)
stackvertical(destination, blurredVideo)
