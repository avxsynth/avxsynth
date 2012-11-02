/*
Dummy plugin:
This implements an Avisynth function "dummy_function(void a)"
that returns "a" unchanged. It will be used to verify that
loading of plugins works.

This code is made available to the public domain, but is
also subject to any licensing terms in Avi/AvxSynth.
*/

#include <avxplugin.h>

typedef avxsynth::AVSValue AVSValue;
typedef avxsynth::IScriptEnvironment IScriptEnvironment;

static AVSValue __cdecl dummy(AVSValue Args, void* UserData, IScriptEnvironment* Env) {
    return Args[0];
}

extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment *Env) {
    Env->AddFunction("dummy_function2", ".", dummy, 0);
    return "hai";
}
