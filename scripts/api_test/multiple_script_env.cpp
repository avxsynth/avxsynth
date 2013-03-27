#include <iostream>
#include "avxplugin.h"

using namespace std;
using namespace avxsynth;

// This program is intended to check correct reference
// counting when creating and deleting script environments.

int main() {
	try {
		cout << "Creating script environment 1..." << endl;
		IScriptEnvironment* env1 = CreateScriptEnvironment(3);

		cout << "Creating script environment 2..." << endl;
		IScriptEnvironment* env2 = CreateScriptEnvironment(3);

		cout << "Deleting script environment 1..." << endl;
		delete env1;

		cout << "Invoking BlankClip on env 2..." << endl;
		AVSValue ret = env2->Invoke("BlankClip", AVSValue(), 0);
		PClip clp = ret.AsClip();

		cout << "Reading frame 0 from env2..." << endl;
		PVideoFrame frm = clp->GetFrame(0, env2);
	} catch (AvisynthError &e) {
		cerr << "AvisynthError: " << e.msg << endl;
		return -1;
	} catch (...) {
		cerr << "unknown error" << endl;
		return -1;
	}

	return 0;
}
