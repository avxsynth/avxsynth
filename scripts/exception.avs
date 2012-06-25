try {
	Assert(false, "RAISED AN EXCEPTION")
	s = "DIDNT RAISE AN EXCEPTION"
}
catch (err_msg) {
	len = StrLen("RAISED AN EXCEPTION")
	s = LeftStr(err_msg, len) == "RAISED AN EXCEPTION" ? "" : "DIDNT CATCH THE EXCEPTION"
}

StrLen(s) != 0 ? Assert(false, s) : blank
