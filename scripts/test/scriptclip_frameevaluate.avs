# This script tests the ScriptClip() and FrameEvaluate() functions
#
# Expected behavior:
#
# The current frame number is printed in the corner. The clip
# gradually becomes blue in 16-unit increments.

BlankClip(length=15, width=320, height=240)

ScriptClip("BlankClip(last, color=s)")
FrameEvaluate("s = current_frame*16")

FrameEvaluate("t = String(current_frame)", after_frame=true)
ScriptClip("Subtitle(t)", after_frame=true)
KillAudio()
