#include "AllBuiltInPlugins.h"

namespace avxsynth {

#ifdef MODULE_NAME
#undef MODULE_NAME
#endif

#define MODULE_NAME builtinfuncs::entryPoint

#if 0
_PixelClip PixelClip;
#endif

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

void add_built_in_functions_Audio_filters(IScriptEnvironment* env)
{
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Audio_filters, 
							"DelayAudio", "cf", DelayAudio::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Audio_filters, 
							"AmplifydB", "cf+", Amplify::Create_dB,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Audio_filters, 
							"Amplify", "cf+", Amplify::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Audio_filters, 
							"AssumeSampleRate", "ci", AssumeRate::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Audio_filters, 
							"Normalize", "c[volume]f[show]b", Normalize::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Audio_filters, 
							"MixAudio", "cc[clip1_factor]f[clip2_factor]f", MixAudio::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Audio_filters, 
							"ResampleAudio", "ci[]i", ResampleAudio::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Audio_filters, 
							"ConvertToMono", "c", ConvertToMono::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Audio_filters, 
							"EnsureVBRMP3Sync", "c", NULL, // TBD: this involves Cache, which depends on VideoFrame, etc....           EnsureVBRMP3Sync::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Audio_filters, 
							"MergeChannels", "c+", MergeChannels::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Audio_filters, 
							"MonoToStereo", "cc", MergeChannels::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Audio_filters, 
							"GetLeftChannel", "c", GetChannel::Create_left,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Audio_filters, 
							"GetRightChannel", "c", GetChannel::Create_right,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Audio_filters, 
							"GetChannel", "ci+", GetChannel::Create_n,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Audio_filters, 
							"GetChannels", "ci+", GetChannel::Create_n,     // Alias to ease use!
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Audio_filters, 
							"KillVideo", "c", KillVideo::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Audio_filters, 
							"KillAudio", "c", KillAudio::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Audio_filters, 
							"ConvertAudioTo16bit", "c", ConvertAudio::Create_16bit,   // in convertaudio.cpp
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Audio_filters, 
							"ConvertAudioTo8bit", "c", ConvertAudio::Create_8bit,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Audio_filters, 
							"ConvertAudioTo24bit", "c", ConvertAudio::Create_24bit,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Audio_filters, 
							"ConvertAudioTo32bit", "c", ConvertAudio::Create_32bit,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Audio_filters, 
							"ConvertAudioToFloat", "c", ConvertAudio::Create_float,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Audio_filters, 
							"ConvertAudio", "cii", ConvertAudio::Create_Any, // For plugins to Invoke()
							NULL);
}

void add_built_in_functions_Combine_filters(IScriptEnvironment* env)
{
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Combine_filters, 
							"StackVertical", "cc+", StackVertical::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Combine_filters, 
							"StackHorizontal", "cc+", StackHorizontal::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Combine_filters, 
							"ShowFiveVersions", "ccccc", ShowFiveVersions::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Combine_filters, 
							"Animate", "iis.*", Animate::Create,  // start frame, end frame, filter, start-args, end-args
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Combine_filters, 
							"Animate", "ciis.*", Animate::Create, 
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Combine_filters, 
							"ApplyRange", "ciis.*", Animate::Create_Range, 
							NULL);

}

void add_built_in_functions_Convert_filters(IScriptEnvironment* env)
{
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Convert_filters, 
							"ConvertToRGB", "c[matrix]s[interlaced]b", ConvertToRGB::Create,       // matrix can be "rec601", rec709", "PC.601" or "PC.709"
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Convert_filters, 
							"ConvertToRGB24", "c[matrix]s[interlaced]b", ConvertToRGB::Create24,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Convert_filters, 
							"ConvertToRGB32", "c[matrix]s[interlaced]b", ConvertToRGB::Create32,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Convert_filters, 
							"ConvertToYV12", "c[interlaced]b[matrix]s", ConvertToYV12::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Convert_filters, 
							"ConvertToYUY2", "c[interlaced]b[matrix]s", ConvertToYUY2::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Convert_filters, 
							"ConvertBackToYUY2", "c[matrix]s", ConvertBackToYUY2::Create,
							NULL);
}

void add_built_in_functions_Convolution_filters(IScriptEnvironment* env)
{
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Convolution_filters, 
							"GeneralConvolution", "c[bias]i[matrix]s[divisor]f[auto]b", GeneralConvolution::Create,
							NULL);
}


void add_built_in_functions_Edit_filters(IScriptEnvironment* env)
{
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Edit_filters, 
							"Trim", "cii[]b", Trim::Create,                       // first frame, last frame[, pad audio]
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Edit_filters, 
							"FreezeFrame", "ciii", FreezeFrame::Create,           // first frame, last frame, source frame
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Edit_filters, 
							"DeleteFrame", "ci+", DeleteFrame::Create,             // frame #
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Edit_filters, 
							"DuplicateFrame", "ci+", DuplicateFrame::Create,       // frame #
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Edit_filters, 
							"UnalignedSplice", "cc+", Splice::CreateUnaligned,    // clips
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Edit_filters, 
							"AlignedSplice", "cc+", Splice::CreateAligned,        // clips
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Edit_filters, 
							"Dissolve", "cc+i[fps]f", Dissolve::Create,           // clips, overlap frames[, fps]
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Edit_filters, 
							"AudioDub", "cc", AudioDub::Create, 
							(void*)0);          // video src, audio src
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Edit_filters, 
							"AudioDubEx", "cc", AudioDub::Create, 
							(void*)1);        // video! src, audio! src
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Edit_filters, 
							"Reverse", "c", Reverse::Create,                      // plays backwards
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Edit_filters, 
							"FadeOut0", "ci[color]i[fps]f", Create_FadeOut0,       // # frames[, color][, fps]
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Edit_filters, 
							"FadeOut", "ci[color]i[fps]f", Create_FadeOut,         // # frames[, color][, fps]
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Edit_filters, 
							"FadeOut2", "ci[color]i[fps]f", Create_FadeOut2,       // # frames[, color][, fps]
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Edit_filters, 
							"FadeIn0", "ci[color]i[fps]f", Create_FadeIn0,         // # frames[, color][, fps]
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Edit_filters, 
							"FadeIn", "ci[color]i[fps]f", Create_FadeIn,           // # frames[, color][, fps]
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Edit_filters, 
							"FadeIn2", "ci[color]i[fps]f", Create_FadeIn2,         // # frames[, color][, fps]
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Edit_filters, 
							"FadeIO0", "ci[color]i[fps]f", Create_FadeIO0,         // # frames[, color][, fps]
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Edit_filters, 
							"FadeIO", "ci[color]i[fps]f", Create_FadeIO,           // # frames[, color][, fps]
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Edit_filters, 
							"FadeIO2", "ci[color]i[fps]f", Create_FadeIO2,         // # frames[, color][, fps]
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Edit_filters, 
							"Loop", "c[times]i[start]i[end]i", Loop::Create,      // number of loops, first frame, last frames
							NULL);
}

void add_built_in_functions_Field_filters(IScriptEnvironment* env)
{
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Field_filters, 
							"ComplementParity", "c", ComplementParity::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Field_filters, 
							"AssumeTFF", "c", AssumeParity::Create, 
							(void*)true);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Field_filters, 
							"AssumeBFF", "c", AssumeParity::Create, 
							(void*)false);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Field_filters, 
							"AssumeFieldBased", "c", AssumeFieldBased::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Field_filters, 
							"AssumeFrameBased", "c", AssumeFrameBased::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Field_filters, 
							"SeparateFields", "c", SeparateFields::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Field_filters, 
							"Weave", "c", Create_Weave,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Field_filters, 
							"DoubleWeave", "c", Create_DoubleWeave,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Field_filters, 
							"Pulldown", "cii", Create_Pulldown,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Field_filters, 
							"SelectEvery", "cii*", SelectEvery::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Field_filters, 
							"SelectEven", "c", SelectEvery::Create_SelectEven,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Field_filters, 
							"SelectOdd", "c", SelectEvery::Create_SelectOdd,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Field_filters, 
							"Interleave", "c+", Interleave::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Field_filters, 
							"SwapFields", "c", Create_SwapFields,
							NULL);
//	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Field_filters, 
//							"Bob", "c[b]f[c]f[height]i", Create_Bob,
//							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Field_filters, 
							"SelectRangeEvery", "c[every]i[length]i[offset]i[audio]b", SelectRangeEvery::Create,
							NULL);
}

void add_built_in_functions_Focus_filters(IScriptEnvironment* env)
{
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Focus_filters, 
							"Blur", "cf[]f[mmx]b", Create_Blur,                     // amount [-1.0 - 1.5849625] -- log2(3)
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Focus_filters, 
							"Sharpen", "cf[]f[mmx]b", Create_Sharpen,               // amount [-1.5849625 - 1.0]
							NULL);
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE  							
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Focus_filters, 
							"TemporalSoften", "ciii[scenechange]i[mode]i", TemporalSoften::Create, // radius, luma_threshold, chroma_threshold
							NULL);
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE						
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Focus_filters, 
							"SpatialSoften", "ciii", SpatialSoften::Create,   // radius, luma_threshold, chroma_threshold
							NULL);
}


void add_built_in_functions_Fps_filters(IScriptEnvironment* env)
{
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Fps_filters, 
							"AssumeScaledFPS", "c[multiplier]i[divisor]i[sync_audio]b", AssumeScaledFPS::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Fps_filters, 
							"AssumeFPS", "ci[]i[sync_audio]b", AssumeFPS::Create,      // dst framerate, sync audio?
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Fps_filters, 
							"AssumeFPS", "cf[sync_audio]b", AssumeFPS::CreateFloat,    // dst framerate, sync audio?
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Fps_filters, 
							"AssumeFPS", "cs[sync_audio]b", AssumeFPS::CreatePreset, // dst framerate, sync audio?
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Fps_filters, 
							"AssumeFPS", "cc[sync_audio]b", AssumeFPS::CreateFromClip, // clip with dst framerate, sync audio?
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Fps_filters, 
							"ChangeFPS", "ci[]i[linear]b", ChangeFPS::Create,     // dst framerate, fetch all frames
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Fps_filters, 
							"ChangeFPS", "cf[linear]b", ChangeFPS::CreateFloat,   // dst framerate, fetch all frames
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Fps_filters, 
							"ChangeFPS", "cs[linear]b", ChangeFPS::CreatePreset, // dst framerate, fetch all frames
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Fps_filters, 
							"ChangeFPS", "cc[linear]b", ChangeFPS::CreateFromClip,// clip with dst framerate, fetch all frames
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Fps_filters, 
							"ConvertFPS", "ci[]i[zone]i[vbi]i", ConvertFPS::Create,      // dst framerate, zone lines, vbi lines
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Fps_filters, 
							"ConvertFPS", "cf[zone]i[vbi]i", ConvertFPS::CreateFloat,    // dst framerate, zone lines, vbi lines
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Fps_filters, 
							"ConvertFPS", "cs[zone]i[vbi]i", ConvertFPS::CreatePreset, // dst framerate, zone lines, vbi lines
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Fps_filters, 
							"ConvertFPS", "cc[zone]i[vbi]i", ConvertFPS::CreateFromClip, // clip with dst framerate, zone lines, vbi lines
							NULL);
}


void add_built_in_functions_Histogram_filters(IScriptEnvironment* env)
{
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Histogram_filters, 
							"Histogram", "c[mode]s", Histogram::Create,   // src clip
							NULL);
}

void add_built_in_functions_Layer_filters(IScriptEnvironment* env)
{
#if 0    
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Layer_filters, 
							"Mask", "cc", Mask::Create,     // clip, mask
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Layer_filters, 
							"ColorKeyMask", "ci[]i[]i[]i", ColorKeyMask::Create,    // clip, color, tolerance[B, toleranceG, toleranceR]
							NULL);
#endif							
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Layer_filters, 
							"ResetMask", "c", ResetMask::Create,
							NULL);
#if 0							
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Layer_filters, 
							"Invert", "c[channels]s", Invert::Create,
							NULL);
#endif							
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Layer_filters, 
							"ShowAlpha", "c[pixel_type]s", ShowChannel::Create, 
							(void*)3);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Layer_filters, 
							"ShowRed", "c[pixel_type]s", ShowChannel::Create, 
							(void*)2);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Layer_filters, 
							"ShowGreen", "c[pixel_type]s", ShowChannel::Create, 
							(void*)1);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Layer_filters, 
							"ShowBlue", "c[pixel_type]s", ShowChannel::Create, 
							(void*)0);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Layer_filters, 
							"MergeRGB",  "ccc[pixel_type]s", MergeRGB::Create, 
							(void*)0);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Layer_filters, 
							"MergeARGB", "cccc",             MergeRGB::Create, 
							(void*)1);
#if 0    
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Layer_filters, 
							"Layer", "cc[op]s[level]i[x]i[y]i[threshold]i[use_chroma]b", Layer::Create,
							NULL);
#endif							
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Layer_filters, 
							"Subtract", "cc", Subtract::Create,
							NULL);
}


void add_built_in_functions_Levels_filters(IScriptEnvironment* env)
{
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Levels_filters, 
							"Levels", "cifiii[coring]b", Levels::Create,        // src_low, gamma, src_high, dst_low, dst_high 
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Levels_filters, 
							"RGBAdjust", "c[r]f[g]f[b]f[a]f[rb]f[gb]f[bb]f[ab]f[rg]f[gg]f[bg]f[ag]f[analyze]b", RGBAdjust::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Levels_filters, 
							"Tweak", "c[hue]f[sat]f[bright]f[cont]f[coring]b[sse]b[startHue]f[endHue]f[maxSat]f[minSat]f[interp]f", Tweak::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Levels_filters, 
							"Limiter", "c[min_luma]i[max_luma]i[min_chroma]i[max_chroma]i[show]s", Limiter::Create,
							NULL);
}

void add_built_in_functions_Misc_filters(IScriptEnvironment* env)
{
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Misc_filters, 
							"FixLuminance", "cif", FixLuminance::Create,    // clip, intercept, slope
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Misc_filters, 
							"FixBrokenChromaUpsampling", "c", FixBrokenChromaUpsampling::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Misc_filters, 
							"PeculiarBlend", "ci", PeculiarBlend::Create,   // clip, cutoff    
							NULL);
}

void add_built_in_functions_Resampling_filters(IScriptEnvironment* env)
{
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Resample_filters, 
							"PointResize", "cii[src_left]f[src_top]f[src_width]f[src_height]f", Create_PointResize,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Resample_filters, 
							"BilinearResize", "cii[src_left]f[src_top]f[src_width]f[src_height]f", Create_BilinearResize,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Resample_filters, 
							"BicubicResize", "cii[b]f[c]f[src_left]f[src_top]f[src_width]f[src_height]f", Create_BicubicResize,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Resample_filters, 
							"LanczosResize", "cii[src_left]f[src_top]f[src_width]f[src_height]f[taps]i", Create_LanczosResize,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Resample_filters, 
							"Lanczos4Resize", "cii[src_left]f[src_top]f[src_width]f[src_height]f", Create_Lanczos4Resize,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Resample_filters, 
							"BlackmanResize", "cii[src_left]f[src_top]f[src_width]f[src_height]f[taps]i", Create_BlackmanResize,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Resample_filters, 
							"Spline16Resize", "cii[src_left]f[src_top]f[src_width]f[src_height]f", Create_Spline16Resize,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Resample_filters, 
							"Spline36Resize", "cii[src_left]f[src_top]f[src_width]f[src_height]f", Create_Spline36Resize,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Resample_filters, 
							"Spline64Resize", "cii[src_left]f[src_top]f[src_width]f[src_height]f", Create_Spline64Resize,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Resample_filters, 
							"GaussResize", "cii[src_left]f[src_top]f[src_width]f[src_height]f[p]f", Create_GaussianResize,
							NULL);
}


void add_built_in_functions_Resize_filters(IScriptEnvironment* env)
{
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Resize_filters, 
							"VerticalReduceBy2", "c", VerticalReduceBy2::Create,        // src clip
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Resize_filters, 
							"HorizontalReduceBy2", "c", HorizontalReduceBy2::Create,    // src clip
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Resize_filters, 
							"ReduceBy2", "c", Create_ReduceBy2,                         // src clip
							NULL);
}

void add_built_in_functions_Source_filters(IScriptEnvironment* env)
{
#if 0
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Source_filters, 
							"AVISource", "s+[audio]b[pixel_type]s[fourCC]s", AVISource::Create, 
							(void*) AVISource::MODE_NORMAL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Source_filters, 
							"AVIFileSource", "s+[audio]b[pixel_type]s[fourCC]s", AVISource::Create, 
							(void*) AVISource::MODE_AVIFILE);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Source_filters, 
							"WAVSource", "s+", AVISource::Create, 
							(void*) AVISource::MODE_WAV);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Source_filters, 
							"OpenDMLSource", "s+[audio]b[pixel_type]s[fourCC]s", AVISource::Create, 
							(void*) AVISource::MODE_OPENDML);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Source_filters, 
							"SegmentedAVISource", "s+[audio]b[pixel_type]s[fourCC]s", Create_SegmentedSource,
							(void*)0);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Source_filters, 
							"SegmentedDirectShowSource",
							// args               0      1      2       3       4            5          6         7            8
							"s+[fps]f[seek]b[audio]b[video]b[convertfps]b[seekzero]b[timeout]i[pixel_type]s",
							Create_SegmentedSource, 
							(void*)1);
// args             0         1       2        3            4     5                 6            7        8             9       10          11     12
#endif
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Source_filters, 
							"BlankClip", "[]c*[length]i[width]i[height]i[pixel_type]s[fps]f[fps_denominator]i[audio_rate]i[stereo]b[sixteen_bit]b[color]i[color_yuv]i[clip]c", Create_BlankClip,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Source_filters, 
							"BlankClip", "[]c*[length]i[width]i[height]i[pixel_type]s[fps]f[fps_denominator]i[audio_rate]i[channels]i[sample_type]s[color]i[color_yuv]i[clip]c", Create_BlankClip,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Source_filters, 
							"Blackness", "[]c*[length]i[width]i[height]i[pixel_type]s[fps]f[fps_denominator]i[audio_rate]i[stereo]b[sixteen_bit]b[color]i[color_yuv]i[clip]c", Create_BlankClip,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Source_filters, 
							"Blackness", "[]c*[length]i[width]i[height]i[pixel_type]s[fps]f[fps_denominator]i[audio_rate]i[channels]i[sample_type]s[color]i[color_yuv]i[clip]c", Create_BlankClip,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Source_filters, 
							"MessageClip", "s[width]i[height]i[shrink]b[text_color]i[halo_color]i[bg_color]i", Create_MessageClip,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Source_filters, 
							"ColorBars", "[width]i[height]i[pixel_type]s", ColorBars::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Source_filters, 
							"Tone", "[length]f[frequency]f[samplerate]i[channels]i[type]s[level]f", Tone::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Source_filters, 
                            "Version", "", Create_Version,
                            NULL);
}

void add_built_in_functions_Transform_filters(IScriptEnvironment* env) 
{
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Transform_filters, 
							"FlipVertical", "c", FlipVertical::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Transform_filters, 
							"FlipHorizontal", "c", FlipHorizontal::Create,     
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Transform_filters, 
							"Crop", "ciiii[align]b", Crop::Create,              // left, top, width, height *OR*
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Transform_filters, 
							"CropBottom", "ci", Create_CropBottom,      // bottom amount
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Transform_filters, 
							"AddBorders", "ciiii[color]i", AddBorders::Create,  // left, top, right, bottom [,color]
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Transform_filters, 
							"Letterbox", "cii[x1]i[x2]i[color]i", Create_Letterbox,       // top, bottom, [left], [right] [,color]
							NULL);
}

void add_built_in_functions_Merge_filters(IScriptEnvironment* env)
{
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Merge_filters,
							"Merge", "cc[weight]f", MergeAll::Create,  // src, src2, weight
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Merge_filters,
							"MergeChroma", "cc[chromaweight]f", MergeChroma::Create,  // src, chroma src, weight
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Merge_filters,
							"MergeLuma", "cc[lumaweight]f", MergeLuma::Create,      // src, luma src, weight
							NULL);
}

void add_built_in_functions_Color_filters(IScriptEnvironment* env)
{
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Color_filters, 
							"ColorYUV",
							"c[gain_y]f[off_y]f[gamma_y]f[cont_y]f" \
							"[gain_u]f[off_u]f[gamma_u]f[cont_u]f" \
							"[gain_v]f[off_v]f[gamma_v]f[cont_v]f" \
							"[levels]s[opt]s[matrix]s[showyuv]b[analyze]b[autowhite]b[autogain]b",
							Color::Create,
							NULL);
}

#if 0

void add_built_in_functions_Image_filters(IScriptEnvironment* env)
{
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Image_filters,
							"ImageWriter", "c[file]s[start]i[end]i[type]s[info]b", ImageWriter::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Image_filters,
							"ImageReader", "[file]s[start]i[end]i[fps]f[use_devil]b[info]b[pixel_type]s", ImageReader::Create,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Image_filters,
							"ImageSource", "[file]s[start]i[end]i[fps]f[use_devil]b[info]b[pixel_type]s", ImageReader::Create,
							NULL);
}

#endif

void add_built_in_functions_Turn_filters(IScriptEnvironment* env)
{
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Turn_filters, 
							"TurnLeft","c",Turn::Create_TurnLeft,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Turn_filters, 
							"TurnRight","c",Turn::Create_TurnRight,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Turn_filters, 
							"Turn180","c",Turn::Create_Turn180,
							NULL);
}

#if 0

void add_built_in_functions_SSRC_filters(IScriptEnvironment* env)
{
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_SSRC_filters, 
							"SSRC", "ci[fast]b", SSRC::Create,
							NULL);
}

void add_built_in_functions_SuperEq_filters(IScriptEnvironment* env)
{
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_SuperEq_filters,
							"SuperEQ", "cs", AVSsupereq::Create,
							NULL);
}

void add_built_in_functions_Overlay_filters(IScriptEnvironment* env)
{
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Overlay_filters, 
							"Overlay", "cc[x]i[y]i[mask]c[opacity]f[mode]s[greymask]b[output]s[ignore_conditional]b[PC_Range]b", Overlay::Create,
							NULL);
}
#endif

void add_built_in_functions_Greyscale_filters(IScriptEnvironment* env)
{
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Greyscale_filters, 
							"Greyscale", "c[matrix]s", Greyscale::Create,       // matrix can be "rec601", "rec709" or "Average"
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Greyscale_filters, 
							"Grayscale", "c[matrix]s", Greyscale::Create,
							NULL);
}

void add_built_in_functions_Swap_filters(IScriptEnvironment* env)
{
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Swap_filters, 
							"SwapUV","c", Swap::CreateUV,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Swap_filters, 
							"UToY","c", Swap::CreateUToY,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Swap_filters, 
							"VToY","c", Swap::CreateVToY,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Swap_filters, 
							"YToUV","cc", Swap::CreateYToUV,
							NULL);
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Swap_filters, 
							"YToUV","ccc", Swap::CreateYToYUV,
							NULL);
}

#if 0

void add_built_in_functions_Soundtouch_filters(IScriptEnvironment* env)
{
	env->AddBuiltInFunction(BUILT_IN_FUNCTIONS_FAMILY_INDEX_Soundtouch_filters, 
							"TimeStretch", "c[tempo]f[rate]f[pitch]f[sequence]i[seekwindow]i[overlap]i[quickseek]b[aa]i", AVSsoundtouch::Create,
							NULL);
}
#endif
__declspec(dllexport) const char* __stdcall AddAviSynthBuiltInFunctions(IScriptEnvironment* env)
{
    AVXLOG_INFO("%s", __FUNCTION__);
    
	add_built_in_functions_Audio_filters(env);
	add_built_in_functions_Combine_filters(env);
	add_built_in_functions_Convert_filters(env);
	add_built_in_functions_Convolution_filters(env);
	add_built_in_functions_Edit_filters(env);
	add_built_in_functions_Field_filters(env);
	add_built_in_functions_Focus_filters(env);
	add_built_in_functions_Fps_filters(env);
	add_built_in_functions_Histogram_filters(env);
	add_built_in_functions_Layer_filters(env);
	add_built_in_functions_Levels_filters(env);
	add_built_in_functions_Misc_filters(env);
	add_built_in_functions_Resampling_filters(env);	
	add_built_in_functions_Resize_filters(env);
	add_built_in_functions_Source_filters(env);
	add_built_in_functions_Transform_filters(env); 
	add_built_in_functions_Merge_filters(env);
	add_built_in_functions_Color_filters(env);
#if 0        
	add_built_in_functions_Image_filters(env);
#endif	
	add_built_in_functions_Turn_filters(env);
#if 0    
	add_built_in_functions_SSRC_filters(env);
	add_built_in_functions_SuperEq_filters(env);
	add_built_in_functions_Overlay_filters(env);
#endif	
	add_built_in_functions_Greyscale_filters(env);
	add_built_in_functions_Swap_filters(env);
#if 0        
	add_built_in_functions_Soundtouch_filters(env);
#endif
	return "BuiltInFunctions";
}

#ifdef __cplusplus
}
#endif // __cplusplus

}; // namespace avxsynth
