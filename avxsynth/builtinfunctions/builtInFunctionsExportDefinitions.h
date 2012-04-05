#ifndef __BUILT_IN_FUNCTIONS_EXPORT_DEFINITIONS_H__
#define __BUILT_IN_FUNCTIONS_EXPORT_DEFINITIONS_H__

namespace avxsynth {
  
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

//__declspec(dllexport) const char* __stdcall AddAviSynthBuiltInFunctions(IScriptEnvironment* env);
const char* AddAviSynthBuiltInFunctions(IScriptEnvironment* env);

#ifdef __cplusplus
}
#endif // __cplusplus

}; // namespace avxsynth

#endif // __BUILT_IN_FUNCTIONS_EXPORT_DEFINITIONS_H__
