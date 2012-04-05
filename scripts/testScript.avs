src_movie = FFVideoSource(source="/home/encodingtools/TestStreams/DestinationTruth/DestinationTruth.mpg", colorspace="yv12")
#src_movie

src_width  = width(src_movie)
src_height = height(src_movie)
scaled_movie = SWScale(src_movie, src_width/3, src_height/3, "LANCZOS")
#scaled_movie

global conformed_width = 0
global conformed_height = 0
global leftpad = 0
global rightpad = 0
global toppad = 0
global bottompad = 0
global desired_width = 0
global desired_height = 0

desired_width = float ( 640 )
desired_height = float ( 480 )
conformed_width  = width(scaled_movie)
conformed_height = height(scaled_movie)

sidePaddingNeeded = 0

conformed_width == desired_width ? Eval ("""
                leftpad = 0
                rightpad = 0
        """ ) : Eval ("""
                sidePaddingNeeded = int(desired_width - conformed_width)
        """ )

sidePaddingNeeded > 0 ? Eval ("""
                leftpad = round(float(sidePaddingNeeded/2))
                rightpad = sidePaddingNeeded - leftpad
        """ ) : Nop()

paddingNeeded = 0

conformed_height == desired_height ? Eval ( """
                toppad = 0
                bottompad = 0
        """ ) : Eval ( """
                paddingNeeded = int(desired_height - conformed_height)
        """ )

paddingNeeded > 0 ? Eval ( """
                toppad = round(float(paddingNeeded/2))
                bottompad = paddingNeeded - toppad
        """ ) : Nop()

scaled_movie = addborders ( scaled_movie, leftpad, toppad , rightpad, bottompad )
#scaled_movie

autocrop(scaled_movie, mode=0)



