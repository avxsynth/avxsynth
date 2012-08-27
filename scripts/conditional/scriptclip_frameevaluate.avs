# This script tests the ScriptClip() and FrameEvaluate() functions
#
# Expected behavior:
#
# The top of the frame fades from black to blue in 16-unit increments.
# The bottom of the frame fades from black to green in 16-unit increments.

full = BlankClip(length=15, width=320, height=240)
half = BlankClip(length=15, width=320, height=120)

ScriptClip(full, "BlankClip(full, color=s)")
FrameEvaluate("s = current_frame*16")

FrameEvaluate("t = current_frame*16*256", after_frame=true)
ScriptClip("StackVertical(Crop(last, 0, 0, 0, 120), BlankClip(half, color=t))", after_frame=true)
KillAudio()
