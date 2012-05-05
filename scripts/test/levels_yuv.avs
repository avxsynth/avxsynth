# This script tests the Levels() function in YUV
#
# Expected behavior:
#
# Two frames, ($A841C0) and ($9541C0), confirm with a hex editor

# output_y = [(input - input_low) / (input_high - input_low)]1/gamma * (output_high - output_low) + output_low
# output_uv = (input - 128) * (output_high - output_low) / (input_high - input_low) + 128
BlankClip(pixel_type="YV12", color_yuv=$4060A0, width=320, height=240, length=1)
# No coring:
# y = [(64 - 10) / (100 - 10)]^(1/1.5) * (220 - 40) + 40 == 168.0 (168)
# u = (96 - 128) * (220 - 40) / (100 - 10) + 128 == 64 (65 due to rounding)
# v = (160 - 128) * (220 - 40) / (100 - 10) + 128 == 192
#
# Coring:
# y_new = (64 - 16) * [255 / (235 - 16)] == 55.9
# y_levels = [(55.9 - 10) / (100 - 10)]^(1/1.5) * (220 - 40) + 40 == 154.9
# y = 154.9 * [(235 - 16) / 255] + 16 == 149.0 (149)
a = Levels(10, 1.5, 100, 40, 220, coring=false)
b = Levels(10, 1.5, 100, 40, 220, coring=true)
a + b
KillAudio()
