# This script tests the Info() function
#
# Expected behavior:
# Two frames having information about the video overlaid.
# The first frame should have "FieldBased (Separated) Video: NO"
# The second frame should have two sets of info each reading
# "FieldBased (Separated) Video: YES"

ColorBars().KillAudio().Trim(0, -1)
a = Info()
b = SeparateFields().Info()
c = StackVertical(SelectEven(b), SelectOdd(b))
a + c
