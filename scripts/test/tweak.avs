# This script tests the Tweak() function
#
# Expected behavior:
#
# 12 frames total, each with diagnostic output from
# ColorYUV(analyze=true) overlaid. The expected output is
# provided in inline comments below.

# UV == <+0.5, -0.5>, HUE == 135deg in normalized units
o1 = BlankClip(pixel_type="YV12", color_yuv=$8040C0, height=240, length=1).KillAudio()
o2 = StackHorizontal(BlankClip(length=1, pixel_type="YV12", width=320, height=240, color_yuv=$101010), \
		BlankClip(length=1, pixel_type="YV12", width=320, height=240, color_yuv=$EBF0F0)).KillAudio()
o3 = ColorBars(pixel_type="YV12", height=240).AssumeFPS(o1).KillAudio().Trim(0, -1)

# Testing hue parameter, 90 degree rotations, expected values:
# UV = (64, 64) and (191, 192) (rounding)
Tweak(o1, hue=-90)
last + Tweak(o1, hue=90)

# Testing sat parameter, 50% gain, expected values:
# UV = (96, 160) and (32, 224)
last + Tweak(o1, sat=0.5)
last + Tweak(o1, sat=1.5)

# Testing bright parameter, +/-64 offset, expected Y values: 64, 192
last + Tweak(o1, bright=-64)
last + Tweak(o1, bright=64)

# Testing cont parameter, 50% gain, expected Y values: 72, 184 (gamma)
last + Tweak(o1, cont=0.5)
last + Tweak(o1, cont=1.5)

# Testing coring parameter, expected min/max:
# (0, 1, 1, 255, 255, 255) (16, 16, 16, 235, 240, 240)
last + Tweak(o2, cont=1.1, sat=1.1)

ColorYUV(analyze=true)

# Testing sat range parameters, expected behavior:
# Pink and blue (bars 2 and 7) are grey. The small bar below bar 1 is also grey.
last + Tweak(o3, sat=0, minsat=71, maxsat=72, interp=0)
# Additionally, bars 3-6 are now desaturated
last + Tweak(o3, sat=0, minsat=71, maxsat=72, interp=4)

# Testing hue range parameters, expected behavior:
# Yellow and red (bars 2 and 6) are now pink and purple
last + Tweak(o3, hue=90, starthue=90, endhue=180)
