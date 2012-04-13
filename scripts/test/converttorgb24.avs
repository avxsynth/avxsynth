# This script tests the ConvertToRGB24 function
#
# Expected behavior:
#
# Output will be 2 frames long, each frame divided into
# four quadrants (NW, NE, SW, SE).
#
# When converted to RGB with the appropriate transform
# (Rec.601, TV levels, interlaced), the output should
# match the reference images converttorgb-REF1.png and
# converttorgb-REF2.png

a = BlankClip(color=$0000FF, pixel_type="YV12", width=320, height=120, length=1).AssumeFieldBased()
b = BlankClip(color=$00FF00, pixel_type="YV12", width=320, height=120, length=1).AssumeFieldBased()
Weave(Interleave(a, b)).KillAudio()
x = StackHorizontal(ConvertToRGB24(matrix="Rec709", interlaced=true), ConvertToRGB24(matrix="Rec709", interlaced=false))
y = StackHorizontal(ConvertToRGB24(matrix="Rec601", interlaced=true), ConvertToRGB24(matrix="Rec601", interlaced=false))
StackVertical(x, y)
u = last

ColorBars(width=320, height=240, pixel_type="YV12").KillAudio()
x = StackHorizontal(ConvertToRGB24(matrix="PC.709", interlaced=true), ConvertToRGB24(matrix="Rec709", interlaced=false))
y = StackHorizontal(ConvertToRGB24(matrix="PC.601", interlaced=true), ConvertToRGB24(matrix="Rec601", interlaced=false))
StackVertical(x, y)
Trim(0, -1)
AssumeFPS(a)
v = last

u + v
