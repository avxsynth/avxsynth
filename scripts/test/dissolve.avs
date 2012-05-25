# This script tests the Dissolve() function
#
# Expected behavior:
#
# Gradual fade from white to black at 1 second, with corresponding
# fade from high to low pitch. Repeated at 3 seconds.

a = BlankClip(color=$FFFFFF, length=48, width=320, height=240).AudioDub(Tone(frequency=880, length=2))
b = BlankClip(color=$000000, length=48, width=320, height=240).AudioDub(Tone(frequency=440, length=2))

x = Dissolve(a, b, 24)
y = AudioDub(Dissolve(a.KillAudio(), b.KillAudio(), 24), Dissolve(a.KillVideo(), b.KillVideo(), 48, fps=48))
x + y
