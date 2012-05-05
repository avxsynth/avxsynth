# This script tests the Grayscale() function
#
# Expected behavior:
#
# See grayscale.png

ColorBars(width=320, height=240).Trim(0, -1)
a = Grayscale(matrix="Rec709")
b = Grayscale(matrix="Rec601")
c = Grayscale(matrix="Average")
d = ColorBars(width=320, height=240, pixel_type="YUY2").Trim(0, -1).Grayscale().ConvertToRGB32(matrix="Rec601")
StackVertical(StackHorizontal(a, b), StackHorizontal(c, d))
KillAudio()