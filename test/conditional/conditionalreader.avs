# This script tests the ConditionalReader() function
#
# Data files:
#
# conditionalreader-[int,float,bool].txt
#
# Expected behavior:
#
# The output will be divided into three stripes, as described.
# Frame 0 - White, Black, Black
# Frame 1-2 - White, Black, White
# Frame 3-5 - White, Black, Black
# Frame 6 - Grey, Green, Grey
# Frame 7 - Dark Grey, Black, Dark Grey
# Frame 8 - V. Dark Grey, Orange, Orange
# Frame 9 - Black, Red, Red
# Frame 10-14 - White, Black, Black

BlankClip(length=15, width=320, height=240)
ScriptClip("""
	a = BlankClip(color=val_int*65536 + val_int*256 + val_int, width=320, height=80)
	b = BlankClip(color=int(65536*val_float), width=320, height=80)
	c = val_bool ? a : b
	StackVertical(a, b, c)
	""")
ConditionalReader("conditionalreader-int.txt", "val_int")
ConditionalReader("conditionalreader-float.txt", "val_float")
ConditionalReader("conditionalreader-bool.txt", "val_bool")
KillAudio()
