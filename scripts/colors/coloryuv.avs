# This script tests the ColorYUV() function
#
# Expected behavior:
#
# 12 frames total, each with diagnostic output from
# ColorYUV(analyze=true) overlaid. The expected output is
# provided in inline comments below.

# For automated testing environment.
try {
	Import("../common.avs")
}
catch(err_msg) {}

o1 = BlankClip(length=1, pixel_type="YV12", height=240, color_yuv=$808080)
o2 = StackHorizontal(BlankClip(length=1, pixel_type="YV12", width=320, height=240, color_yuv=$408080), \
		BlankClip(length=1, pixel_type="YV12", width=320, height=240, color_yuv=$C08080))
o3 = StackHorizontal(BlankClip(length=1, pixel_type="YV12", width=320, height=240, color_yuv=$101010), \
		BlankClip(length=1, pixel_type="YV12", width=320, height=240, color_yuv=$EBF0F0))
o4 = StackHorizontal(BlankClip(length=1, pixel_type="YV12", width=320, height=240, color_yuv=$000000), \
		BlankClip(length=1, pixel_type="YV12", width=320, height=240, color_yuv=$FFFFFF))
o5 = StackHorizontal(BlankClip(length=1, pixel_type="YV12", width=320, height=240, color_yuv=$202020), \
		BlankClip(length=1, pixel_type="YV12", width=320, height=240, color_yuv=$404040))

# Testing gain parameter, 50% gain, expected average: 192, 64
ColorYUV(o1, gain_y=128, gain_u=128, gain_v=128)
last + ColorYUV(o1, gain_y=-128, gain_u=-128, gain_v=-128)

# Testing off parameter, +/- 64 offset, expected average: 192, 64
last + ColorYUV(o1, off_y=64, off_u=64, off_v=64)
last + ColorYUV(o1, off_y=-64, off_u=-64, off_v=-64)

# Testing gamma parameter 1.5, 0.5, expected average: 161, 64
last + ColorYUV(o1, gamma_y=128)
last + ColorYUV(o1, gamma_y=-128)

# Testing cont parameter, +/-50% expansion, expected min/max:
# (32, 224) (96, 160)
last + ColorYUV(o2, cont_y=128, cont_u=128, cont_v=128)
last + ColorYUV(o2, cont_y=-128, cont_u=-128, cont_v=-128)

# Testing levels parameter, expected min/max:
# (0, 1, 1, 255, 255, 255) (16, 16, 16, 235, 240, 240)
last + ColorYUV(o3, levels="TV->PC")
last + ColorYUV(o4, levels="PC->TV")

# Testing opt=coring parameter, expected min/max:
# (16, 16, 16, 235, 240, 240)
last + ColorYUV(o3, cont_y=10, cont_u=10, cont_v=10, opt="coring")

# Testing autowhite+autogain parameter, expected min/max:
# (16, 111, 111, 236, 143, 143)
last + ColorYUV(o5, autogain=true, autowhite=true)

# Testing showYUV parameter:
last + ColorYUV(showYUV=true).PointResize(width, height).Trim(0, -1)

ColorYUV(analyze=true)
