# This script tests the ShowFrameNumber / ShowSMPTE / ShowTime functions
#
# Expected behavior:
#
# Four sets of numbers:
# 1. Right-side of frame: The current frame number scrolling in yellow, large, serif.
# 2. Right-side of frame: A wall of the current frame number, blue with red outline.
# 3. Center of frame: The current timestamp + 1s with white glow, 45 degree clockwise rotation.
# 4. Bottom of frame: The current SMPTE timestamp + 10s in narrow font.

BlankClip(width=640, height=480, length=48).KillAudio()
ShowFrameNumber(scroll=true, offset=48, font="Times", size=48)
ShowFrameNumber(scroll=false, text_color=$0000FF, halo_color=$FF0000)
ShowSMPTE(offset_f=240, font_width=6)
ShowTime(offset_f=24, x=320, y=240, halo_color=$FFFFFF, font_angle=315)
