# This script tests the WriteFile family of functions
#
# Expected behavior:
# A text file "writefile.txt" is created with the following text:
#
# THIS IS THE HEADER
# 0
# LESS THAN THREE
# 1
# LESS THAN THREE
# 2
# LESS THAN THREE
# 3
# 4
# THIS IS THE END
# SDFG

BlankClip(length=5)
WriteFileStart("writefile.txt", """ "THIS IS THE HEADER" """)
WriteFileEnd("writefile.txt", """ "THIS IS THE END" """)
WriteFile("writefile.txt", "current_frame")
WriteFileIf("writefile.txt", "current_frame < 3", """ "LESS THAN THREE" """)
KillAudio()
