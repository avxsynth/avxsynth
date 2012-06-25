s = Exist("setworkingdir.avs") ? "x" : Assert(false, "script not in its own path")

# 0 == success, -1 == fail
z = SetWorkingDir(tests_build_dir + "emptydir") != 0 ? Assert(false, "SetWorkingDir failed") : NOP

s = Exist("setworkingdir.avs") ? Assert(false, "emptydir is not empty") : ""

blank
