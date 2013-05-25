firstNameInitial = UCase("m")
firstName = firstNameInitial + LCase(RevStr("NALI"))

middleLetter = Ucase("m")
dot = "."
lastNameInitial = UCase("s")
lastName = lastNameInitial + LCase("TEVANOVIC")

delimiter = " "
completeName = firstName + delimiter + middleLetter + dot + delimiter + lastName
len = StrLen(completeName)

leftPart  = LeftStr(completename, 5)
rightPart = RightStr(completename, 10)
pos = Findstr(completename, ".")
midPart   = MidStr(completename, 6, 4)
MessageClip(leftPart + midPart + rightPart + " has " + String(len) + " characters; dot position = " + String(pos))
