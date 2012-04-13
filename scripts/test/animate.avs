# This script tests the Animate() function
#
# Expected behavior:
#
# Two identical scrolling versions of the Version() function.
# This test was adapted from Avisynth documentation.

ver = Version()
StackVertical(Animate(0,47,"Crop", ver,0,0,64,80, ver,width(ver)-64,0,0,80), 
			\ Animate(ver,0,47,"Crop", 0,0,64,80, width(ver)-64,0,64,80))
Trim(0, 47)