// Prints the string representation of an AVS value to stdout.
// Probably not safe against buffer overflow etc.

#include "avxplugin.h"
#include <stdio.h>

using namespace avxsynth;

int main(int argc, char *argv[]) {
	if (argc < 3) {
		fprintf(stderr, "Usage: %s script.avs varname\n", argv[0]);
		return 2;
	}

	IScriptEnvironment *env;
	const char *buf;
	AVSValue val, newval;

	env = CreateScriptEnvironment();
	try {
		env->Invoke("Import", argv[1]);
		val = env->GetVar(argv[2]);
		newval = env->Invoke("string", val);
		buf = newval.AsString();
		printf("%s", buf);
	}
	catch (...) {
		fprintf(stderr, "???\n");
		delete env;
		return 2;
	}
	delete env;
	return 0;
}
