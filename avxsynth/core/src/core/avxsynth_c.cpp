// Avisynth C Interface Version 0.20 stdcall
// Copyright 2003 Kevin Atkinson
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//

#include "stdafx.h"
#include <stdarg.h>
#include <dlfcn.h>
#include "../internal.h"
#include "avxsynth_c.h"

using namespace avxsynth;

struct AVS_Clip 
{
	PClip clip;
	IScriptEnvironment * env;
	const char * error;
	AVS_Clip() : env(0), error(0) {}
};

struct AVS_ScriptEnvironment
{
	IScriptEnvironment * env;
	const char * error;
	AVS_ScriptEnvironment(IScriptEnvironment * e = 0) 
		: env(e), error(0) {}
};

class C_VideoFilter : public IClip {
public: // but don't use
	AVS_Clip child;
	AVS_ScriptEnvironment env;
	AVS_FilterInfo d;
public:
	C_VideoFilter() {memset(&d,0,sizeof(d));}
	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
	void __stdcall GetAudio(void * buf, __int64 start, __int64 count, IScriptEnvironment* env);
	const VideoInfo & __stdcall GetVideoInfo();
	bool __stdcall GetParity(int n);
	void __stdcall SetCacheHints(int cachehints,size_t frame_range);
	__stdcall ~C_VideoFilter();
};

/////////////////////////////////////////////////////////////////////
//
//
//

void AVSC_CC avs_release_video_frame(AVS_VideoFrame * f)
{
	((PVideoFrame *)&f)->~PVideoFrame();
}

AVS_VideoFrame * AVSC_CC avs_copy_video_frame(AVS_VideoFrame * f)
{
  AVS_VideoFrame * fnew;
  new ((PVideoFrame *)&fnew) PVideoFrame(*(PVideoFrame *)&f);
  return fnew;
}


/////////////////////////////////////////////////////////////////////
//
// C_VideoFilter
//

PVideoFrame C_VideoFilter::GetFrame(int n, IScriptEnvironment* env) 
{
	if (d.get_frame) {
		d.error = 0;
		AVS_VideoFrame * f = d.get_frame(&d, n);
		if (d.error)
			throw AvisynthError(d.error);
		PVideoFrame fr((VideoFrame *)f);
    ((PVideoFrame *)&f)->~PVideoFrame();  
    return fr;
	} else {
		return d.child->clip->GetFrame(n, env); 
	}
}

void __stdcall C_VideoFilter::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) 
{
	if (d.get_audio) {
		d.error = 0;
		d.get_audio(&d, buf, start, count);
		if (d.error)
			throw AvisynthError(d.error);
	} else {
		d.child->clip->GetAudio(buf, start, count, env);
	}
}

const VideoInfo& __stdcall C_VideoFilter::GetVideoInfo() 
{
	return *(VideoInfo *)&d.vi; 
}

bool __stdcall C_VideoFilter::GetParity(int n) 
{
	if (d.get_parity) {
		d.error = 0;
		int res = d.get_parity(&d, n);
		if (d.error)
			throw AvisynthError(d.error);
		return !!res;
	} else {
		return d.child->clip->GetParity(n);
	}
}

void __stdcall C_VideoFilter::SetCacheHints(int cachehints, size_t frame_range) 
{
	if (d.set_cache_hints) {
		d.error = 0;
		d.set_cache_hints(&d, cachehints, frame_range);
		if (d.error)
			throw AvisynthError(d.error);
	}
	// We do not pass cache requests upwards, only to the next filter.
}

C_VideoFilter::~C_VideoFilter()
{
	if (d.free_filter)
		d.free_filter(&d);
}

/////////////////////////////////////////////////////////////////////
//
// AVS_Clip
//

void AVSC_CC avs_release_clip(AVS_Clip * p)
{
	delete p;
}

AVS_Clip * AVSC_CC avs_copy_clip(AVS_Clip * p)
{
	return new AVS_Clip(*p);
}

const char * AVSC_CC avs_clip_get_error(AVS_Clip * p) // return 0 if no error
{
	return p->error;
}

int AVSC_CC avs_get_version(AVS_Clip * p)
{
  return p->clip->GetVersion();
}

const AVS_VideoInfo * AVSC_CC avs_get_video_info(AVS_Clip  * p)
{
  return  (const AVS_VideoInfo  *)&p->clip->GetVideoInfo();
}


AVS_VideoFrame * AVSC_CC avs_get_frame(AVS_Clip * p, int n)
{
	p->error = 0;
	try {
		PVideoFrame f0 = p->clip->GetFrame(n,p->env);
		AVS_VideoFrame * f;
		new((PVideoFrame *)&f) PVideoFrame(f0);
		return f;
	} catch (AvisynthError err) {
		p->error = err.msg;
		return 0;
	} 
}

int AVSC_CC avs_get_parity(AVS_Clip * p, int n) // return field parity if field_based, else parity of first field in frame
{
	try {
		p->error = 0;
		return p->clip->GetParity(n);
	} catch (AvisynthError err) {
		p->error = err.msg;
		return -1;
	} 
}

int AVSC_CC avs_get_audio(AVS_Clip * p, void * buf, INT64 start, INT64 count) // start and count are in samples
{
	try {
		p->error = 0;
		p->clip->GetAudio(buf, start, count, p->env);
		return 0;
	} catch (AvisynthError err) {
		p->error = err.msg;
		return -1;
	} 
}

int AVSC_CC avs_set_cache_hints(AVS_Clip * p, int cachehints, size_t frame_range)  // We do not pass cache requests upwards, only to the next filter.
{
	try {
		p->error = 0;
		p->clip->SetCacheHints(cachehints, frame_range);
		return 0;
	} catch (AvisynthError err) {
		p->error = err.msg;
		return -1;
	}
}

//////////////////////////////////////////////////////////////////
//
//
//
AVS_Clip * AVSC_CC avs_take_clip(AVS_Value v, AVS_ScriptEnvironment * env)
{
	AVS_Clip * c = new AVS_Clip;
	c->env  = env->env;
	c->clip = (IClip *)v.d.clip;
	return c;
}

void AVSC_CC avs_set_to_clip(AVS_Value * v, AVS_Clip * c)
{
	new(v) AVSValue(c->clip);
}

void AVSC_CC avs_copy_value(AVS_Value * dest, AVS_Value src)
{
	new(dest) AVSValue(*(const AVSValue *)&src);
}

void AVSC_CC avs_release_value(AVS_Value v)
{
	((AVSValue *)&v)->~AVSValue();
}

//////////////////////////////////////////////////////////////////
//
//
//

AVS_Clip * AVSC_CC avs_new_c_filter(AVS_ScriptEnvironment * e,
							AVS_FilterInfo * * fi,
							AVS_Value child, int store_child)
{
	C_VideoFilter * f = new C_VideoFilter();
	AVS_Clip * ff = new AVS_Clip();
	ff->clip = f;
	ff->env  = e->env;
	f->env.env = e->env;
	f->d.env = &f->env;
	if (store_child) {
		_ASSERTE(child.type == 'c');
		f->child.clip = (IClip *)child.d.clip;
		f->child.env  = e->env;
		f->d.child = &f->child;
	}
	*fi = &f->d;
	if (child.type == 'c')
		f->d.vi = *(const AVS_VideoInfo *)(&((IClip *)child.d.clip)->GetVideoInfo());
	return ff;
}

/////////////////////////////////////////////////////////////////////
//
// AVS_ScriptEnvironment::add_function
//

struct C_VideoFilter_UserData {
	void * user_data;
	AVS_ApplyFunc func;
};

AVSValue __cdecl create_c_video_filter(AVSValue args, void * user_data,
									                     IScriptEnvironment * e0)
{
	C_VideoFilter_UserData * d = (C_VideoFilter_UserData *)user_data;
	AVS_ScriptEnvironment env(e0);
	AVS_Value res = (d->func)(&env, *(AVS_Value *)&args, d->user_data);
	if (res.type == 'e') {
    throw AvisynthError(res.d.string);
	} else {
    AVSValue val;
    val = (*(const AVSValue *)&res);
    ((AVSValue *)&res)->~AVSValue();
		return val;
	}
}

int AVSC_CC 
  avs_add_function(AVS_ScriptEnvironment * p, const char * name, const char * params, 
				   AVS_ApplyFunc applyf, void * user_data)
{
	C_VideoFilter_UserData *dd, *d = new C_VideoFilter_UserData;
	p->error = 0;
	d->func = applyf;
	d->user_data = user_data;
	dd = (C_VideoFilter_UserData *)p->env->SaveString((const char *)d, sizeof(C_VideoFilter_UserData));
	delete d;
	try {
		p->env->AddFunction(name, params, create_c_video_filter, dd);
	} catch (AvisynthError & err) {
		p->error = err.msg;
		return -1;
	} 
	return 0;
}

/////////////////////////////////////////////////////////////////////
//
// AVS_ScriptEnvironment
//

const char * AVSC_CC avs_get_error(AVS_ScriptEnvironment * p) // return 0 if no error
{
	return p->error;
}

long AVSC_CC avs_get_cpu_flags(AVS_ScriptEnvironment * p)
{
	p->error = 0;
	return p->env->GetCPUFlags();
}

char * AVSC_CC avs_save_string(AVS_ScriptEnvironment * p, const char* s, int length)
{
	p->error = 0;
	return p->env->SaveString(s, length);
}

char * AVSC_CC avs_sprintf(AVS_ScriptEnvironment * p, const char* fmt, ...)
{
	p->error = 0;
	va_list vl;
	va_start(vl, fmt);
	char * v = p->env->VSprintf(fmt, vl);
	va_end(vl);
	return v;
}

 // note: val is really a va_list; I hope everyone typedefs va_list to a pointer
char * AVSC_CC avs_vsprintf(AVS_ScriptEnvironment * p, const char* fmt, va_list val)
{
	p->error = 0;
	return p->env->VSprintf(fmt, val);
}

int AVSC_CC avs_function_exists(AVS_ScriptEnvironment * p, const char * name)
{
	p->error = 0;
	return p->env->FunctionExists(name);
}

AVS_Value AVSC_CC avs_invoke(AVS_ScriptEnvironment * p, const char * name, AVS_Value args, const char * * arg_names)
{
	AVS_Value v = {0,0};
	p->error = 0;
	try {
		AVSValue v0 = p->env->Invoke(name, *(AVSValue *)&args, arg_names);
		new ((AVSValue *)&v) AVSValue(v0);
	} catch (IScriptEnvironment::NotFound) {
    p->error = "Function Not Found";
	} catch (AvisynthError err) {
		p->error = err.msg;
	}
  if (p->error)
    v = avs_new_value_error(p->error);
	return v;
}

AVS_Value AVSC_CC avs_get_var(AVS_ScriptEnvironment * p, const char* name)
{
	AVS_Value v = {0,0};
	p->error = 0;
	try {
		AVSValue v0 = p->env->GetVar(name);
		new ((AVSValue *)&v) AVSValue(v0);
	} catch (IScriptEnvironment::NotFound) {}
	return v;
}

int AVSC_CC avs_set_var(AVS_ScriptEnvironment * p, const char* name, AVS_Value val)
{
	p->error = 0;
	try {
		return p->env->SetVar(p->env->SaveString(name), *(const AVSValue *)(&val));
	} catch (AvisynthError err) {
		p->error = err.msg;
		return -1;
	}
}

int AVSC_CC avs_set_global_var(AVS_ScriptEnvironment * p, const char* name, AVS_Value val)
{
	p->error = 0;
	try {
		return p->env->SetGlobalVar(p->env->SaveString(name), *(const AVSValue *)(&val));
	} catch (AvisynthError err) {
		p->error = err.msg;
		return -1;
	}
}

AVS_VideoFrame * AVSC_CC avs_new_video_frame_a(AVS_ScriptEnvironment * p, const AVS_VideoInfo *  vi, int align)
{
	p->error = 0;
	try {
	PVideoFrame f0 = p->env->NewVideoFrame(*(const VideoInfo *)vi, align);
	AVS_VideoFrame * f;
	new((PVideoFrame *)&f) PVideoFrame(f0);
	return f;
	} catch(AvisynthError err) {
		p->error = err.msg;
	}
	return 0;
}

int AVSC_CC avs_make_writable(AVS_ScriptEnvironment * p, AVS_VideoFrame * * pvf)
{
	p->error = 0;
	try {
		return p->env->MakeWritable((PVideoFrame *)(pvf));
	} catch (AvisynthError err) {
		p->error = err.msg;
	}
	return -1;
}

void AVSC_CC avs_bit_blt(AVS_ScriptEnvironment * p, unsigned char* dstp, int dst_pitch, const unsigned char* srcp, int src_pitch, int row_size, int height)
{
	p->error = 0;
	try {
	p->env->BitBlt(dstp, dst_pitch, srcp, src_pitch, row_size, height);
	} catch (AvisynthError err) {
		p->error = err.msg;
	}
}

struct ShutdownFuncData
{
  AVS_ShutdownFunc func;
  void * user_data;
};

void __cdecl shutdown_func_bridge(void* user_data, IScriptEnvironment* env)
{
  ShutdownFuncData * d = (ShutdownFuncData *)user_data;
  AVS_ScriptEnvironment e;
  e.env = env;
  d->func(d->user_data, &e);
}

void AVSC_CC avs_at_exit(AVS_ScriptEnvironment * p, 
                           AVS_ShutdownFunc function, void * user_data)
{
  p->error = 0;
  ShutdownFuncData *dd, *d = new ShutdownFuncData;
  d->func = function;
  d->user_data = user_data;
  dd = (ShutdownFuncData *)p->env->SaveString((const char *)d, sizeof(ShutdownFuncData));
  delete d;
  p->env->AtExit(shutdown_func_bridge, dd);
}

int AVSC_CC avs_check_version(AVS_ScriptEnvironment * p, int version)
{
	p->error = 0;
	try {
		p->env->CheckVersion(version);
		return 0;
	} catch (AvisynthError err) {
		p->error = err.msg;
		return -1;
	}
}

AVS_VideoFrame * AVSC_CC avs_subframe(AVS_ScriptEnvironment * p, AVS_VideoFrame * src0, 
							  int rel_offset, int new_pitch, int new_row_size, int new_height)
{
	p->error = 0;
	try {
		PVideoFrame f0 = p->env->Subframe((VideoFrame *)src0, rel_offset, new_pitch, new_row_size, new_height);
		AVS_VideoFrame * f;
		new((PVideoFrame *)&f) PVideoFrame(f0);
		return f;
	} catch (AvisynthError err) {
		p->error = err.msg;
		return 0;
	}
}

AVS_VideoFrame * AVSC_CC avs_subframe_planar(AVS_ScriptEnvironment * p, AVS_VideoFrame * src0, 
							  int rel_offset, int new_pitch, int new_row_size, int new_height,
							  int rel_offsetU, int rel_offsetV, int new_pitchUV)
{
	p->error = 0;
	try {
		PVideoFrame f0 = p->env->SubframePlanar((VideoFrame *)src0, rel_offset, new_pitch, new_row_size,
												new_height, rel_offsetU, rel_offsetV, new_pitchUV);
		AVS_VideoFrame * f;
		new((PVideoFrame *)&f) PVideoFrame(f0);
		return f;
	} catch (AvisynthError err) {
		p->error = err.msg;
		return 0;
	}
}

int AVSC_CC avs_set_memory_max(AVS_ScriptEnvironment * p, int mem)
{
	p->error = 0;
	try {
		return p->env->SetMemoryMax(mem);
	} catch (AvisynthError err) {
		p->error = err.msg;
		return -1;
	}
}

int AVSC_CC avs_set_working_dir(AVS_ScriptEnvironment * p, const char * newdir)
{
	p->error = 0;
	try {
		return p->env->SetWorkingDir(newdir);
	} catch (AvisynthError err) {
		p->error = err.msg;
		return -1;
	}
}
/////////////////////////////////////////////////////////////////////
//
// 
//
AVS_ScriptEnvironment * AVSC_CC avs_create_script_environment(int version)
{
	AVS_ScriptEnvironment * e = new AVS_ScriptEnvironment;
	try {
	e->env = CreateScriptEnvironment(version);
	} catch (AvisynthError err) {
		e->error = err.msg;
		e->env = 0;
	}
	return e;
}


/////////////////////////////////////////////////////////////////////
//
// 
//
void AVSC_CC avs_delete_script_environment(AVS_ScriptEnvironment * e)
{
	if (e) {
		if (e->env) delete e->env;
		delete e;
	}
}


/////////////////////////////////////////////////////////////////////
//
// 
//

typedef const char * (AVSC_CC *AvisynthCPluginInitFunc)(AVS_ScriptEnvironment* env);

AVSValue __cdecl load_c_plugin(AVSValue args, void * user_data, 
					           IScriptEnvironment * env)
{
	const char * filename = args[0].AsString();
	
    HMODULE plugin = dlopen(filename, RTLD_NOW | RTLD_GLOBAL);
	if (!plugin)
		env->ThrowError("Unable to load C Plugin: \"%s\", error=0x%x", filename, dlerror());
		
    AvisynthCPluginInitFunc func = 0;
	func = (AvisynthCPluginInitFunc)dlsym(plugin, "avisynth_c_plugin_init");
	if (!func)
		env->ThrowError("Not An Avisynth 2 C Plugin: %s", filename);
	AVS_ScriptEnvironment e;
	e.env = env;
	AVS_ScriptEnvironment *pe;
	pe = &e;
	const char *s = NULL;
	int callok = 1; // (stdcall)
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
	__asm // Tritical - Jan 2006
	{
		push eax
		push edx

		push 0x12345678		// Stash a known value

		mov eax, pe			// Env pointer
		push eax			// Arg1
		call func			// avisynth_c_plugin_init

		lea edx, s			// return value is in eax
		mov DWORD PTR[edx], eax

		pop eax				// Get top of stack
		cmp eax, 0x12345678	// Was it our known value?
		je end				// Yes! Stack was cleaned up, was a stdcall

		lea edx, callok
		mov BYTE PTR[edx], 0 // Set callok to 0 (_cdecl)

		pop eax				// Get 2nd top of stack
		cmp eax, 0x12345678	// Was this our known value?
		je end				// Yes! Stack is now correctly cleaned up, was a _cdecl

		mov BYTE PTR[edx], 2 // Set callok to 2 (bad stack)
end:
		pop edx
		pop eax
	}
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
	if (callok == 2)
		env->ThrowError("Avisynth 2 C Plugin '%s' has corrupted the stack.", filename);
#ifndef AVSC_USE_STDCALL
	if (callok != 0)
		env->ThrowError("Avisynth 2 C Plugin '%s' has wrong calling convention! Must be _cdecl.", filename);
#else // AVSC_USE_STDCALL
	if (callok != 1)
		env->ThrowError("Avisynth 2 C Plugin '%s' has wrong calling convention! Must be stdcall.", filename);
#endif // AVSC_USE_STDCALL
	if (s == 0)
		env->ThrowError("Avisynth 2 C Plugin '%s' returned a NULL pointer.", filename);

	return AVSValue(s);
}

namespace avxsynth {
    
AVSFunction CPlugin_filters[] = {
    {"LoadCPlugin", "s", load_c_plugin },
    {"Load_Stdcall_Plugin", "s", load_c_plugin },
    { 0 }
};

}; // namespace avxsynth
