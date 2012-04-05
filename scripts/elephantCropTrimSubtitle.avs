FFVideoSource(source="/home/encodingtools/Video/ElephantsDream/ElephantsDream-avc.mpg", colorspace="yv12")
Crop(0, 0, 704, 480)
Trim(240,0)
Subtitle("width = " + String(width(last)) + " height = " + String(height(last)))
