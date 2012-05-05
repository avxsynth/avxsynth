# This script tests the Levels() function in RGB
#
# Expected behavior:
#
# A grey ($909090) frame, confirm with a hex editor

BlankClip(pixel_type="RGB32", color=$404040, width=320, height=240, length=1)
# output = [(input - input_low) / (input_high - input_low)]1/gamma * (output_high - output_low) + output_low
Levels(10, 1.5, 100, 5, 200) # [(64 - 10) / (100 - 10)]^(1/1.5) * (200 - 5) + 5 == 143.7 (144)
KillAudio()