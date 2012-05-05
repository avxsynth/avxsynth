# This script tests the MessageClip() function
#
# Expected behavior:
#
# Two frames with "This is a message" printed in red with green
# outline against blue background. The second frame will be centered
# and noticeably upscaled.


a = MessageClip("This is a message", width=320, height=240, shrink=false, text_color=$FF0000, halo_color=$00FF00, bg_color=$0000FF).Trim(0, -1)
b = MessageClip("This is a message", width=320, height=240, shrink=true, text_color=$FF0000, halo_color=$00FF00, bg_color=$0000FF).Trim(0, -1)

a + b.BilinearResize(width(a), height(a))
KillAudio()
