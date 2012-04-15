# This script tests the GeneralConvolution() function
#
# Expected behavior:
#
# Frame 1 - Blurred colorbars
# Frame 2 - Vertical edge mask of colorbars
#
# These examples were taken from Avisynth documentation

ColorBars(width=320, height=240)
Trim(0, -1)
a = GeneralConvolution(0, "
   10 10 10 10 10
   10 10 10 10 10 
   10 10 16 10 10 
   10 10 10 10 10 
   10 10 10 10 10 ", 256, False)
b = GeneralConvolution(128, "
   1  0 -1 
   2  0 -2 
   1  0 -1 ", 8)
a + b
KillAudio()
