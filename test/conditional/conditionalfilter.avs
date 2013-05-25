# This script tests the ConditionalFilter() function
#
# Expected behavior:
#
# Five frames, in the following order:
# white, white, black, black, white

a = BlankClip(color=$FFFFFF, length=1, width=320, height=240)
b = BlankClip(color=$000000, length=1, width=320, height=240)

BlankClip(pixel_type="YV12", color_yuv=$800000)
ConditionalFilter(a, b, "AverageLuma()", "=", "128") ++ \
	ConditionalFilter(a, b, "AverageLuma()", ">", "127") ++ \
	ConditionalFilter(a, b, "AverageLuma()", "<", "127") ++ \
	ConditionalFilter(a, b, "AverageLuma()", ">", "129") ++ \
	ConditionalFilter(a, b, "AverageLuma()", "<", "129")
KillAudio()
