#include <iostream>
#include <avxplugin.h>

#define cout std::cout
using namespace avxsynth;

// This program is intended to check correct reference
// counting when creating and deleting script environments.

int main() {

cout << "Creating script environment...\n";
IScriptEnvironment* env = CreateScriptEnvironment(3);
cout << "Deleting script environment...\n";
delete env;

cout << "Creating script environment...\n";
IScriptEnvironment* env2 = CreateScriptEnvironment(3);
cout << "Invoking BlankClip...\n";
AVSValue ret = env2->Invoke("blankclip", AVSValue(), 0);
PClip clp = ret.AsClip();
cout << "Retrieving frame 0...\n";
PVideoFrame frame = clp->GetFrame(0, env2);
const BYTE* read_ptr = frame->GetReadPtr();
if ( read_ptr[0] ) {
	cout << "Unexpected data\n";
	return -1;
}
cout << "Deleting script environment...\n";
delete env2;

return 0;
}
