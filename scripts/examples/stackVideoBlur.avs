elephant = FFVideoSource(source="/home/encodingtools/Video/ElephantsDream/ElephantsDream-avc.mpg", colorspace="rgb32")
elephant = Crop(elephant, 0, 0, 640, 180)
elephant = Trim(elephant, 240, 539)
elephant = Subtitle(elephant, "width = " + String(width(elephant)) + " height = " + String(height(elephant)))
destination = FFVideoSource(source="/home/encodingtools/Video/DestinationTruth/DestinationTruth.mpg", colorspace="rgb32")
destination = SWScale(destination, 640, 360, "Lanczos")
destination = Trim(destination, 0, 299)
blurred = GeneralConvolution(stackvertical(destination, elephant), 0, "
   10 10 10 10 10
   10 10 10 10 10 
   10 10 16 10 10 
   10 10 10 10 10 
   10 10 10 10 10 ", 256, false)
stackhorizontal(stackvertical(elephant, destination), blurred)
