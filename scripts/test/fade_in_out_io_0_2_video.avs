# This script tests the FadeIO family of functions
#
# Expected behavior:
# 
# Nine horizontal stripes, fading in and out.
# All fades out should end at frame 26 (black)
# All fades in should start at frame 50 (colored)
#
# An accompanying tone will fade in/out simultaneously.
# Total audio frames are 156000 and video frames 76.

BlankClip(width=480, height=360/9, length=72)
StackVertical(BlankClip(last, length=1) + FadeIn(24, $00FF00) + BlankClip(last, length=2), \
				BlankClip(last, length=1) + FadeIO(24, $FF0000) + BlankClip(last, length=1), \
				BlankClip(last, length=2) + FadeOut(24, $0000FF) + BlankClip(last, length=1), \
				BlankClip(last, length=2) + FadeIn0(24, $00FF00) + BlankClip(last, length=2), \
				BlankClip(last, length=2) + FadeIO0(24, $FF0000) + BlankClip(last, length=2), \
				BlankClip(last, length=2) + FadeOut0(24, $0000FF) + BlankClip(last, length=2), \
				FadeIn2(24, $00FF00) + BlankClip(last, length=2), \
				FadeIO2(24, $FF0000), \
				BlankClip(last, length=2) + FadeOut2(24, $0000FF))
AudioDub(FadeIO2(Tone(length=3), 48, fps=48))
KillAudio()
