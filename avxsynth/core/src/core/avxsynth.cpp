// Avisynth v4.0  Copyright 2007 Ben Rudiak-Gould et al.
// http://www.avisynth.org
// http://www.avxsynth.org


// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .
//
// Linking Avisynth statically or dynamically with other modules is making a
// combined work based on Avisynth.  Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Avisynth give you
// permission to link Avisynth with independent modules that communicate with
// Avisynth solely through the interfaces defined in avisynth.h, regardless of the license
// terms of these independent modules, and to copy and distribute the
// resulting combined work under terms of your choice, provided that
// every copy of the combined work is accompanied by a complete copy of
// the source code of Avisynth (the version of Avisynth used to produce the
// combined work), being distributed under the terms of the GNU General
// Public License plus this exception.  An independent module is a module
// which is not derived from or based on Avisynth, such as 3rd-party filters,
// import and export plugins, or graphical user interfaces.

#include "stdafx.h"
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>

#include <string>
using std::string;

#include "../internal.h"
#include "./parser/script.h"
#include "memcpy_amd.h"
#include "cache.h"
#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <vector>
#ifdef __APPLE__
#include <mach/host_info.h>
#include <mach/mach_host.h>
#elif defined(BSD)
#include <sys/sysctl.h>
#endif
#include "windowsPorts.h"
#include "builtInFunctionsExportDefinitions.h"
#include "avxlog.h"

namespace avxsynth {
  
#ifdef MODULE_NAME
#undef MODULE_NAME
#endif

#define MODULE_NAME core::avxsynth

const char _AVS_VERSTR[]    = AVS_VERSTR;
const char _AVS_COPYRIGHT[] = AVS_AVX_SYNTH AVS_COPYRIGHT AVS_VERSTR;

_PixelClip PixelClip;

#ifdef _MSC_VER
  #define strnicmp(a,b,c) _strnicmp(a,b,c)
  #define strdup(a) _strdup(a)
#endif

extern AVSFunction Plugin_functions[], 
                   Script_functions[], Text_filters[], Debug_filters[],
                   Conditional_filters[], Conditional_funtions_filters[],
                   CPlugin_filters[], Cache_filters[]
                   ;
                   
std::vector<std::vector<AVSFunction> > builtInFunctions;

// Global statistics counters
struct MEM_STATS{
  unsigned long CleanUps;
  unsigned long Losses;
  unsigned long PlanA1;
  unsigned long PlanA2;
  unsigned long PlanB;
  unsigned long PlanC;
  unsigned long PlanD;
  char tag[36];
} g_Mem_stats = {0, 0, 0, 0, 0, 0, 0, "CleanUps, Losses, Plan[A1,A2,B,C,D]"};

//const HKEY RegRootKey = HKEY_LOCAL_MACHINE;
const char RegAvisynthKey[] = "Software\\Avisynth";
const char RegPluginDir[] = "PluginDir2_5";

extern const char* loadplugin_prefix;

// in plugins.cpp
AVSValue LoadPlugin(AVSValue args, void* user_data, IScriptEnvironment* env);
void FreeLibraries(void* loaded_plugins, IScriptEnvironment* env);

//extern const char* loadplugin_prefix;  // in plugin.cpp


class LinkedVideoFrame {
public:
  LinkedVideoFrame* next;
  VideoFrame vf;
};

class RecycleBin {  // Tritical May 2005
public:
	LinkedVideoFrame* g_VideoFrame_recycle_bin;
	RecycleBin() : g_VideoFrame_recycle_bin(NULL) { };
	~RecycleBin()
	{
		for (LinkedVideoFrame*i=g_VideoFrame_recycle_bin; i;)
		{
			LinkedVideoFrame* j = i->next;
			operator delete(i);
			i = j;
		}
	}
};


// Tsp June 2005 the heap is cleared when ScriptEnviroment is destroyed

static RecycleBin *g_Bin=0;

void* VideoFrame::operator new(size_t size) {
  // CriticalSection
  for (LinkedVideoFrame* i = g_Bin->g_VideoFrame_recycle_bin; i; i = i->next)
    if (i->vf.refcount == 0)
      return &i->vf;
  LinkedVideoFrame* result = (LinkedVideoFrame*)::operator new(sizeof(LinkedVideoFrame));
  result->next = g_Bin->g_VideoFrame_recycle_bin;
  g_Bin->g_VideoFrame_recycle_bin = result;
  return &result->vf;
}


VideoFrame::VideoFrame(VideoFrameBuffer* _vfb, int _offset, int _pitch, int _row_size, int _height)
  : refcount(0), vfb(_vfb), offset(_offset), pitch(_pitch), row_size(_row_size), height(_height),
    offsetU(_offset),offsetV(_offset),pitchUV(0)  // PitchUV=0 so this doesn't take up additional space
{
  InterlockedIncrement(&vfb->refcount);
}

VideoFrame::VideoFrame(VideoFrameBuffer* _vfb, int _offset, int _pitch, int _row_size, int _height,
                       int _offsetU, int _offsetV, int _pitchUV)
  : refcount(0), vfb(_vfb), offset(_offset), pitch(_pitch), row_size(_row_size), height(_height),
    offsetU(_offsetU),offsetV(_offsetV),pitchUV(_pitchUV)
{
  InterlockedIncrement(&vfb->refcount);
}

VideoFrame* VideoFrame::Subframe(int rel_offset, int new_pitch, int new_row_size, int new_height) const {
  return new VideoFrame(vfb, offset+rel_offset, new_pitch, new_row_size, new_height);
}


VideoFrame* VideoFrame::Subframe(int rel_offset, int new_pitch, int new_row_size, int new_height,
                                 int rel_offsetU, int rel_offsetV, int new_pitchUV) const {
  return new VideoFrame(vfb, offset+rel_offset, new_pitch, new_row_size, new_height,
                        rel_offsetU+offsetU, rel_offsetV+offsetV, new_pitchUV);
}


VideoFrameBuffer::VideoFrameBuffer() : data(0), data_size(0), sequence_number(0), refcount(0) {}


#ifdef _DEBUG  // Add 16 guard bytes front and back -- cache can check them after every GetFrame() call
VideoFrameBuffer::VideoFrameBuffer(int size) : 
  refcount(0), 
  data((new BYTE[size+32])+16), 
  data_size(data ? size : 0), 
  sequence_number(0) {
  InterlockedIncrement(&sequence_number); 
  int *p=(int *)data;
  p[-4] = 0xDEADBEAF;
  p[-3] = 0xDEADBEAF;
  p[-2] = 0xDEADBEAF;
  p[-1] = 0xDEADBEAF;
  p=(int *)(data+size);
  p[0] = 0xDEADBEAF;
  p[1] = 0xDEADBEAF;
  p[2] = 0xDEADBEAF;
  p[3] = 0xDEADBEAF;
}

VideoFrameBuffer::~VideoFrameBuffer() {
//  _ASSERTE(refcount == 0);
  InterlockedIncrement(&sequence_number); // HACK : Notify any children with a pointer, this buffer has changed!!!
  if (data) delete[] (BYTE*)(data-16);
  (BYTE*)data = 0; // and mark it invalid!!
  (int)data_size = 0;   // and don't forget to set the size to 0 as well!
}

#else

VideoFrameBuffer::VideoFrameBuffer(int size)
 : data(new BYTE[size]), data_size(data ? size : 0), sequence_number(0), refcount(0) { InterlockedIncrement(&sequence_number); }

VideoFrameBuffer::~VideoFrameBuffer() {
//  _ASSERTE(refcount == 0);
  InterlockedIncrement(&sequence_number); // HACK : Notify any children with a pointer, this buffer has changed!!!
  if (data) delete[] data;
// Trick GCC into writing to a const variable.  This behavior is undefined.
// The VFB class is in the public API so we are stuck with this hack.
  BYTE** not_data = const_cast<BYTE**>(&data);
  int* not_data_size = const_cast<int*>(&data_size);
  *not_data = 0;
  *not_data_size = 0;
}
#endif


class LinkedVideoFrameBuffer : public VideoFrameBuffer {
public:
  enum {ident = 0x00AA5500};
  LinkedVideoFrameBuffer *prev, *next;
  bool returned;
  const int signature; // Used by ManageCache to ensure that the VideoFrameBuffer 
                       // it casts is really a LinkedVideoFrameBuffer
  LinkedVideoFrameBuffer(int size) : VideoFrameBuffer(size), returned(true), signature(ident) { next=prev=this; }
  LinkedVideoFrameBuffer() : returned(true), signature(ident) { next=prev=this; }
};


class VarTable {
  VarTable* const dynamic_parent;
  VarTable* const lexical_parent;

  struct Variable {
    Variable* next;
    const char* const name;
    AVSValue val;
    Variable(const char* _name, Variable* _next) : next(_next), name(_name) {}
  };

  Variable variables;   // first entry is "last"

public:
  VarTable(VarTable* _dynamic_parent, VarTable* _lexical_parent)
    : dynamic_parent(_dynamic_parent), lexical_parent(_lexical_parent), variables("last", 0) {}

  ~VarTable() {
    Variable* v = variables.next;
    while (v) {
      Variable* next = v->next;
      delete v;
      v = next;
    }
  }

  VarTable* Pop() {
    VarTable* _dynamic_parent = this->dynamic_parent;
    delete this;
    return _dynamic_parent;
  }

  const AVSValue& Get(const char* name) {
    for (Variable* v = &variables; v; v = v->next)
      if (!strcasecmp(name, v->name))
        return v->val;
    if (lexical_parent)
      return lexical_parent->Get(name);
    else
      throw IScriptEnvironment::NotFound();
  }

  bool Set(const char* name, const AVSValue& val) {
    for (Variable* v = &variables; v; v = v->next)
      if (!strcasecmp(name, v->name)) {
        v->val = val;
        return false;
      }
    variables.next = new Variable(name, variables.next);
    variables.next->val = val;
    return true;
  }
};


class FunctionTable {
  struct LocalFunction : AVSFunction {
    LocalFunction* prev;
  };

  struct Plugin {
    char* name;
    LocalFunction* plugin_functions;
    Plugin* prev;
  };

  LocalFunction* local_functions;
  Plugin* plugins;
  bool prescanning, reloading;

  IScriptEnvironment* const env;

public:

  FunctionTable(IScriptEnvironment* _env) : prescanning(false), reloading(false), env(_env) {
    local_functions = 0;
    plugins = 0;
  }

  ~FunctionTable() {
    while (local_functions) {
      LocalFunction* prev = local_functions->prev;
	  free((void*)local_functions->name);  // Tritical May 2005
	  free((void*)local_functions->param_types);
      delete local_functions;
      local_functions = prev;
    }
    while (plugins) {
      RemovePlugin(plugins);
    }
  }

  void StartPrescanning() { prescanning = true; }
  void StopPrescanning() { prescanning = false; }

  void PrescanPluginStart(const char* name)
  {
    if (!prescanning)
      env->ThrowError("FunctionTable: Not in prescanning state");
    AVXLOG_INFO("Prescanning plugin: %s", name);
    Plugin* p = new Plugin;
    p->name = strdup(name);
    p->plugin_functions = 0;
    p->prev = plugins;
    plugins = p;
  }

  void RemovePlugin(Plugin* p)
  {
    LocalFunction* cur = p->plugin_functions;
    while (cur) {
      LocalFunction* prev = cur->prev;
      free((void*)cur->name);
      free((void*)cur->param_types);
      delete cur;
      cur = prev;
    }
    if (p == plugins) {
      plugins = plugins->prev;
    } else {
      Plugin* pp = plugins;
      while (pp->prev != p) pp = pp->prev;
      pp->prev = p->prev;
    }
    free(p->name);
    delete p;
  }

  static bool IsParameterTypeSpecifier(char c) {
    switch (c) {
        case 'b': case 'i': case 'f': case 's': case 'c': case '.': case 'l':
        return true;
      default:
        return false;
    }
  }

  static bool IsParameterTypeModifier(char c) {
    switch (c) {
      case '+': case '*':
        return true;
      default:
        return false;
    }
  }

  static bool IsValidParameterString(const char* p) {
    int state = 0;
    char c;
    while ((c = *p++) != '\0' && state != -1) {
      switch (state) {
        case 0:
          if (IsParameterTypeSpecifier(c)) {
            state = 1;
          }
          else if (c == '[') {
            state = 2;
          }
          else {
            state = -1;
          }
          break;

        case 1:
          if (IsParameterTypeSpecifier(c)) {
            // do nothing; stay in the current state
          }
          else if (c == '[') {
            state = 2;
          }
          else if (IsParameterTypeModifier(c)) {
            state = 0;
          }
          else {
            state = -1;
          }
          break;

        case 2:
          if (c == ']') {
            state = 3;
          }
          else {
            // do nothing; stay in the current state
          }
          break;

        case 3:
          if (IsParameterTypeSpecifier(c)) {
            state = 1;
          }
          else {
            state = -1;
          }
          break;
      }
    }

    // states 0, 1 are the only ending states we accept
    return state == 0 || state == 1;
  }

  // Update $Plugin! -- Tritical Jan 2006
  void AddFunction(const char* name, const char* params, IScriptEnvironment::ApplyFunc apply, void* user_data) {
    if (prescanning && !plugins)
      env->ThrowError("FunctionTable in prescanning state but no plugin has been set");

    if (!IsValidParameterString(params))
      env->ThrowError("%s has an invalid parameter string (bug in filter)", name);

    bool duse = false;

// Option for Tritcal - Nonstandard, manifestly changes behaviour
#ifdef OPT_TRITICAL_NOOVERLOAD

// Do not allow LoadPlugin or LoadCPlugin to be overloaded
// to access the new function the alternate name must be used
    if      (strcasecmp(name, "LoadPlugin")  == 0) duse = true;
    else if (strcasecmp(name, "LoadCPlugin") == 0) duse = true;
#endif
    const char* alt_name = 0;
    LocalFunction *f = NULL;
    if (!duse) {
      f = new LocalFunction;
      f->name = strdup(name);  // Tritical May 2005
      f->param_types = strdup(params);
      if (!prescanning) {
        f->apply = apply;
        f->user_data = user_data;
        f->prev = local_functions;
        local_functions = f;
      } else {
        AVXLOG_DEBUG("  Function %s (prescan)", name);
        f->prev = plugins->plugin_functions;
        plugins->plugin_functions = f;
      }
    }

    LocalFunction *f2 = NULL;
    if (loadplugin_prefix) {
      AVXLOG_DEBUG("  Plugin name %s", loadplugin_prefix);
      char result[PATH_MAX];
      strcpy(result, loadplugin_prefix);
      strcat(result, "_");
      strcat(result, name);
      f2 = new LocalFunction;
      f2->name = strdup(result);     // needs to copy here since the plugin will be unloaded
      f2->param_types = strdup(params);     // needs to copy here since the plugin will be unloaded
      alt_name = f2->name;
      if (prescanning) {
        f2->prev = plugins->plugin_functions;
        plugins->plugin_functions = f2;
      } else {
        f2->apply = apply;
        f2->user_data = user_data;
        f2->prev = local_functions;
        local_functions = f2;
      }
    }
// *******************************************************************
// *** Make Plugin Functions readable for external apps            ***
// *** Tobias Minich, Mar 2003                                     ***
// BEGIN *************************************************************
    if (prescanning) {
      AVSValue fnplugin;
      char *fnpluginnew;
      try {
        fnplugin = env->GetVar("$PluginFunctions$");
        int string_len = strlen(fnplugin.AsString())+1;

        if (!duse)
          string_len += strlen(name)+1;

        if (alt_name)
          string_len += strlen(alt_name)+1;

        fnpluginnew = new char[string_len];
        strcpy(fnpluginnew, fnplugin.AsString());
        if (!duse) {
          strcat(fnpluginnew, " ");
          strcat(fnpluginnew, name);
        }
        if (alt_name) {
          strcat(fnpluginnew, " ");
          strcat(fnpluginnew, alt_name);
        }
        env->SetGlobalVar("$PluginFunctions$", AVSValue(env->SaveString(fnpluginnew, string_len)));
        delete[] fnpluginnew;

      } catch (...) {
        int string_len = 0;
        if (!duse && alt_name)
        {
          string_len = strlen(name)+strlen(alt_name)+2;
          fnpluginnew = new char[string_len];
          strcpy(fnpluginnew, name);
          strcat(fnpluginnew, " ");
          strcat(fnpluginnew, alt_name);
        }
        else if (!duse)
        {
          string_len = strlen(name)+1;
          fnpluginnew = new char[string_len];
          strcpy(fnpluginnew, name);
        }
        else if (alt_name)
        {
          string_len = strlen(alt_name)+1;
          fnpluginnew = new char[string_len];
          strcpy(fnpluginnew, alt_name);
        }
        env->SetGlobalVar("$PluginFunctions$", AVSValue(env->SaveString(fnpluginnew, string_len)));
        delete[] fnpluginnew;
      }
      char temp[1024] = "$Plugin!";
      if (f) {
        strcat(temp, name);
        strcat(temp, "!Param$");
        env->SetGlobalVar(env->SaveString(temp, 8+strlen(name)+7+1), AVSValue(f->param_types)); // Fizick
      }
      if (f2 && alt_name) {
        strcpy(temp, "$Plugin!");
        strcat(temp, alt_name);
        strcat(temp, "!Param$");
        env->SetGlobalVar(env->SaveString(temp, 8+strlen(alt_name)+7+1), AVSValue(f2->param_types));
      }
    }
// END ***************************************************************

  }

  AVSFunction* Lookup(const char* search_name, const AVSValue* args, int num_args,
                      bool* pstrict, int args_names_count, const char** arg_names) {
    int oanc;
    do {
      for (int strict = 1; strict >= 0; --strict) {
        *pstrict = strict&1;
        // first, look in loaded plugins
        for (LocalFunction* p = local_functions; p; p = p->prev)
          if (!strcasecmp(p->name, search_name) &&
              TypeMatch(p->param_types, args, num_args, strict&1, env) &&
              ArgNameMatch(p->param_types, args_names_count, arg_names))
            return p;
        // now looks in prescanned plugins
        for (Plugin* pp = plugins; pp; pp = pp->prev)
          for (LocalFunction* p = pp->plugin_functions; p; p = p->prev)
            if (!strcasecmp(p->name, search_name) &&
                TypeMatch(p->param_types, args, num_args, strict&1, env) &&
                ArgNameMatch(p->param_types, args_names_count, arg_names)) { 
              AVXLOG_INFO("Loading plugin %s (lookup for function %s)", pp->name, p->name);
              // sets reloading in case the plugin is performing env->FunctionExists() calls
              reloading = true;
              AVSValue temp1(pp->name);
              AVSValue temp2(&temp1, 1);
              LoadPlugin(AVSValue(&temp2, 1), (void*)false, env);
              reloading = false;
              // just in case the function disappeared from the plugin, avoid infinte recursion
              RemovePlugin(pp);
              // restart the search
              return Lookup(search_name, args, num_args, pstrict, args_names_count, arg_names);
            }
        // finally, look for a built-in function
        for (unsigned int i = 0; i < builtInFunctions.size(); ++i)
	{
          for (unsigned int j = 0; j < builtInFunctions[i].size(); j++)
	  {
	    bool bFunctionNameMatch = (0 == strcasecmp(builtInFunctions[i][j].name, search_name));
	    bool bTypeMatch = TypeMatch(builtInFunctions[i][j].param_types, args, num_args, strict&1, env);
	    bool bArgNameMatch = ArgNameMatch(builtInFunctions[i][j].param_types, args_names_count, arg_names);
            if (bFunctionNameMatch && bTypeMatch && bArgNameMatch)
              return &builtInFunctions[i][j];
	  }
	}
      }
      // Try again without arg name matching
      oanc = args_names_count;
      args_names_count = 0;
    } while (oanc);
    return 0;
  }

  bool Exists(const char* search_name) {
    for (LocalFunction* p = local_functions; p; p = p->prev)
      if (!strcasecmp(p->name, search_name))
        return true;
    if (!reloading) {
      for (Plugin* pp = plugins; pp; pp = pp->prev)
        for (LocalFunction* p = pp->plugin_functions; p; p = p->prev)
          if (!strcasecmp(p->name, search_name))
            return true;
    }
    for (unsigned int i = 0; i < builtInFunctions.size(); ++i)
      for (unsigned int j = 0; j < builtInFunctions[i].size(); j++)
        if (!strcasecmp(builtInFunctions[i][j].name, search_name))
          return true;
    return false;
  }

  static bool SingleTypeMatch(char type, const AVSValue& arg, bool strict) {
    switch (type) {
      case '.': return true;
      case 'b': return arg.IsBool();
      case 'i': return arg.IsInt();
      case 'f': return arg.IsFloat() && (!strict || !arg.IsInt());
      case 's': return arg.IsString();
      case 'c': return arg.IsClip();
      default:  return false;
    }
  }

  bool TypeMatch(const char* param_types, const AVSValue* args, int num_args, bool strict, IScriptEnvironment* env) {

    bool optional = false;

    int i = 0;
    while (i < num_args) {

      if (*param_types == '\0') {
        // more args than params
        return false;
      }

      if (*param_types == '[') {
        // named arg: skip over the name
        param_types = strchr(param_types+1, ']');
        if (param_types == NULL) {
          env->ThrowError("TypeMatch: unterminated parameter name (bug in filter)");
        }

        ++param_types;
        optional = true;

        if (*param_types == '\0') {
          env->ThrowError("TypeMatch: no type specified for optional parameter (bug in filter)");
        }
      }

      if (param_types[1] == '*') {
        // skip over initial test of type for '*' (since zero matches is ok)
        ++param_types;
      }

      switch (*param_types) {
        case 'b': case 'i': case 'f': case 's': case 'c':
          if (   (!optional || args[i].Defined())
              && !SingleTypeMatch(*param_types, args[i], strict))
            return false;
          // fall through
        case '.':
          ++param_types;
          ++i;
          break;
        case '+': case '*':
          if (!SingleTypeMatch(param_types[-1], args[i], strict)) {
            // we're done with the + or *
            ++param_types;
          }
          else {
            ++i;
          }
          break;
        default:
          env->ThrowError("TypeMatch: invalid character in parameter list (bug in filter)");
      }
    }

    // We're out of args.  We have a match if one of the following is true:
    // (a) we're out of params.
	// (b) remaining params are named i.e. optional.
    // (c) we're at a '+' or '*' and any remaining params are optional.

    if (*param_types == '+'  || *param_types == '*')
	  param_types += 1;

    if (*param_types == '\0' || *param_types == '[')
	  return true;

	while (param_types[1] == '*') {
	  param_types += 2;
      if (*param_types == '\0' || *param_types == '[')
	    return true;
    }

	return false;
  }

  bool ArgNameMatch(const char* param_types, int args_names_count, const char** arg_names) {

	for (int i=0; i<args_names_count; ++i) {
	  if (arg_names[i]) {
		bool found = false;
		int len = strlen(arg_names[i]);
		for (const char* p = param_types; *p; ++p) {
		  if (*p == '[') {
			p += 1;
			const char* q = strchr(p, ']');
			if (!q) return false;
			if (len == q-p && !strncasecmp(arg_names[i], p, q-p)) {
			  found = true;
			  break;
			}
			p = q+1;
		  }
		}
		if (!found) return false;
	  }
	}
	return true;
  }
};


// This doles out storage space for strings.  No space is ever freed
// until the class instance is destroyed (which happens when a script
// file is closed).
class StringDump {
  enum { BLOCK_SIZE = 32768 };
  char* current_block;
  int block_pos, block_size;

public:
  StringDump() : current_block(0), block_pos(BLOCK_SIZE), block_size(BLOCK_SIZE) {}
  ~StringDump();
  char* SaveString(const char* s, int len = -1);
};

StringDump::~StringDump() {
  AVXLOG_INFO("%s", "StringDump: DeAllocating all stringblocks.");
  char* p = current_block;
  while (p) {
    char* next = *(char**)p;
    delete[] p;
    p = next;
  }
}

char* StringDump::SaveString(const char* s, int len) {
  if (len == -1)
    len = strlen(s);
  if (block_pos+len+1 > block_size) {
    char* new_block = new char[block_size = std::max((unsigned long)block_size, (unsigned long)(len+1+sizeof(char*)))];
    AVXLOG_INFO("s", "StringDump: Allocating new stringblock.");
    *(char**)new_block = current_block;   // beginning of block holds pointer to previous block
    current_block = new_block;
    block_pos = sizeof(char*);
  }
  char* result = current_block+block_pos;
  memcpy(result, s, len);
  result[len] = 0;
  block_pos += (len+sizeof(char*)) & -sizeof(char*); // Keep 32bit aligned
  return result;
}


class AtExiter {
  IScriptEnvironment* const env;
  struct AtExitRec {
    const IScriptEnvironment::ShutdownFunc func;
    void* const user_data;
    AtExitRec* const next;
    AtExitRec(IScriptEnvironment::ShutdownFunc _func, void* _user_data, AtExitRec* _next)
      : func(_func), user_data(_user_data), next(_next) {}
  };
  AtExitRec* atexit_list;
public:
  AtExiter(IScriptEnvironment* _env) : env(_env) {
    atexit_list = 0;
  }
  void Add(IScriptEnvironment::ShutdownFunc f, void* d) {
    atexit_list = new AtExitRec(f, d, atexit_list);
  }
  ~AtExiter() {
#if 1
    while (atexit_list) {
      AtExitRec* next = atexit_list->next;
      atexit_list->func(atexit_list->user_data, env);
      delete atexit_list;
      atexit_list = next;
    }
#endif
  }
};


class ScriptEnvironment : public IScriptEnvironment {
public:
  ScriptEnvironment();
  void __stdcall CheckVersion(int version);
  long __stdcall GetCPUFlags();
  char* __stdcall SaveString(const char* s, int length = -1);
  char* __stdcall Sprintf(const char* fmt, ...);
  char* __stdcall VSprintf(const char* fmt, va_list val);
  void __stdcall ThrowError(const char* fmt, ...);
  void __stdcall AddFunction(const char* name, const char* params, ApplyFunc apply, void* user_data=0);
  void __stdcall InitializeBuiltInFunctionsStorage(void);
  void __stdcall DebugListBuiltInFunctions(void);
  void __stdcall FreeAllBuiltInPlugins(void);
  void __stdcall FreeBuiltInPluginsInfrastructure(unsigned int nFamily, unsigned int nFamilyMember);
  void __stdcall AddBuiltInFunction(unsigned int nFunctionFamilyIndex, const char* name, const char* params, ApplyFunc apply, void* user_data);
  bool __stdcall LoadBuiltInPlugins(void);
  bool __stdcall LoadInternalBuiltinPlugins(void); // temporary use only
  bool __stdcall FunctionExists(const char* name);
  AVSValue __stdcall Invoke(const char* name, const AVSValue args, const char** arg_names=0);
  AVSValue __stdcall GetVar(const char* name);
  bool __stdcall SetVar(const char* name, const AVSValue& val);
  bool __stdcall SetGlobalVar(const char* name, const AVSValue& val);
  void __stdcall PushContext(int level=0);
  void __stdcall PopContext();
  void __stdcall PopContextGlobal();
  PVideoFrame __stdcall NewVideoFrame(const VideoInfo& vi, int align);
  PVideoFrame NewVideoFrame(int row_size, int height, int align);
  PVideoFrame NewPlanarVideoFrame(int width, int height, int align, bool U_first);
  bool __stdcall MakeWritable(PVideoFrame* pvf);
  void __stdcall BitBlt(BYTE* dstp, int dst_pitch, const BYTE* srcp, int src_pitch, int row_size, int height);
  void __stdcall AtExit(IScriptEnvironment::ShutdownFunc function, void* user_data);
  PVideoFrame __stdcall Subframe(PVideoFrame src, int rel_offset, int new_pitch, int new_row_size, int new_height);
  int __stdcall SetMemoryMax(int mem);
  int __stdcall SetWorkingDir(const char * newdir);
  __stdcall ~ScriptEnvironment();
  void* __stdcall ManageCache(int key, void* data);
  bool __stdcall PlanarChromaAlignment(IScriptEnvironment::PlanarChromaAlignmentMode key);
  PVideoFrame __stdcall SubframePlanar(PVideoFrame src, int rel_offset, int new_pitch, int new_row_size, int new_height, int rel_offsetU, int rel_offsetV, int new_pitchUV);

private:
  // Tritical May 2005
  // Note order here!!
  // AtExiter has functions which
  // rely on StringDump elements.
  StringDump string_dump;

  AtExiter at_exit;

  FunctionTable function_table;

  VarTable* global_var_table;
  VarTable* var_table;

  LinkedVideoFrameBuffer video_frame_buffers, lost_video_frame_buffers, *unpromotedvfbs;
  __int64 memory_max, memory_used;

  LinkedVideoFrameBuffer* NewFrameBuffer(int size);

  LinkedVideoFrameBuffer* GetFrameBuffer2(int size);
  VideoFrameBuffer* GetFrameBuffer(int size);
  long CPU_id;

  // helper for Invoke
  int Flatten(const AVSValue& src, AVSValue* dst, int index, int max, const char** arg_names=0);

  IScriptEnvironment* This() { return this; }
  const char* GetPluginDirectory();
  bool PluginsFolderIsNotEmpty();
  bool LoadPluginsMatching(const char* pattern);
  bool IsFileExtension(char* pStrFilename, const char* pStrExtension);
  bool IsPluginNameAcceptable(char* pStrFilename);
  bool LoadAVISynthCustomFunctionScripts();
  void PrescanPlugins();
  void ExportFilters();
  bool PlanarChromaAlignmentState;

  Cache* CacheHead;

  static long refcount; // Global to all ScriptEnvironment objects
};

long ScriptEnvironment::refcount=0;
extern long CPUCheckForExtensions();  // in cpuaccel.cpp

ScriptEnvironment::ScriptEnvironment()
  : at_exit(This()),
    function_table(This()),
    unpromotedvfbs(&video_frame_buffers),
    PlanarChromaAlignmentState(true), // Change to "true" for 2.5.7
    CacheHead(0)
{
    CPU_id = CPUCheckForExtensions();

    if(InterlockedCompareExchange(&refcount, 1, 0) == 0)//tsp June 2005 Initialize Recycle bin
      g_Bin=new RecycleBin();
    else
      InterlockedIncrement(&refcount);

#if 0 // Win32 specific
//    MEMORYSTATUS memstatus;
//    GlobalMemoryStatus(&memstatus);
//    // Minimum 16MB
//    // else physical memory/4
//    // Maximum 0.5GB
//    if (memstatus.dwAvailPhys    > 64*1024*1024)
//      memory_max = (__int64)memstatus.dwAvailPhys >> 2;
//    else
//      memory_max = 16*1024*1024;
#else
    long nPageSize = sysconf(_SC_PAGE_SIZE);
    __int64 nAvailablePhysicalPages;
#ifdef __APPLE__
    vm_statistics64_data_t vmstats;
    mach_msg_type_number_t vmstatsz = HOST_VM_INFO64_COUNT;
    host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info_t)&vmstats, &vmstatsz);
    nAvailablePhysicalPages = vmstats.free_count;
#elif defined(BSD)
    size_t nAvailablePhysicalPagesLen = sizeof(nAvailablePhysicalPages);
    sysctlbyname("vm.stats.vm.v_free_count", &nAvailablePhysicalPages, &nAvailablePhysicalPagesLen, NULL, 0);
#else // Linux
    nAvailablePhysicalPages = sysconf(_SC_AVPHYS_PAGES);
#endif
    memory_max = (__int64)(nPageSize*nAvailablePhysicalPages) >> 2;
#endif
    if (memory_max <= 0 || memory_max > 512*1024*1024) // More than 0.5GB
      memory_max = 512*1024*1024;

    memory_used = (__int64)0;
    global_var_table = new VarTable(0, 0);
    var_table = new VarTable(0, global_var_table);
    global_var_table->Set("true", true);
    global_var_table->Set("false", false);
    global_var_table->Set("yes", true);
    global_var_table->Set("no", false);

	InitializeBuiltInFunctionsStorage();
	LoadInternalBuiltinPlugins(); // <--- this method will eventually go away, it is temporary until we port out all built-in plugins
	LoadBuiltInPlugins();
    PrescanPlugins();
    ExportFilters();
}

void ScriptEnvironment::DebugListBuiltInFunctions(void)
{
	unsigned int nFamilies = builtInFunctions.size();
	unsigned int i, j;
	for(i = 0; i < nFamilies; i++)
	{
		AVXLOG_DEBUG("Family #%02d: ", i);
		AVXLOG_DEBUG("%s", "------------------------------");
		unsigned int nFunctions = builtInFunctions[i].size();
		for(j = 0; j < nFunctions; j++)
		{
			AVXLOG_DEBUG(" Function #%02d: ", j);
			AVXLOG_DEBUG("  name        = %s", builtInFunctions[i][j].name ? builtInFunctions[i][j].name : "NULL");
			AVXLOG_DEBUG("  param_types = %s", builtInFunctions[i][j].param_types ? builtInFunctions[i][j].param_types : "NULL");
			AVXLOG_DEBUG("  apply       = 0x%p", builtInFunctions[i][j].apply);
			AVXLOG_DEBUG("  user_data   = 0x%p", builtInFunctions[i][j].user_data);
		}
		AVXLOG_DEBUG("%s", "------------------------------");
	}
}

void ScriptEnvironment::InitializeBuiltInFunctionsStorage(void)
{
	unsigned int i;

	for(i = 0; i < NUMBER_OF_BUILT_IN_FUNCTIONS_FAMILIES; i++)
	{
		std::vector<AVSFunction> tempFamily;
		builtInFunctions.push_back(tempFamily);
	}

	//DebugListBuiltInFunctions();
}

ScriptEnvironment::~ScriptEnvironment() {
  while (var_table)
    PopContext();
  while (global_var_table)
    PopContextGlobal();
  unpromotedvfbs = &video_frame_buffers;
  LinkedVideoFrameBuffer* i = video_frame_buffers.prev;
  while (i != &video_frame_buffers) {
    LinkedVideoFrameBuffer* prev = i->prev;
    delete i;
    i = prev;
  }
  i = lost_video_frame_buffers.prev;
  while (i != &lost_video_frame_buffers) {
    LinkedVideoFrameBuffer* prev = i->prev;
    delete i;
    i = prev;
  }
  if(!InterlockedDecrement((long*)&refcount)){
	delete g_Bin;//tsp June 2005 Cleans up the heap
	g_Bin=NULL;
  }

  FreeAllBuiltInPlugins();
}

void ScriptEnvironment::FreeAllBuiltInPlugins(void)
{
	for(unsigned int i = 0; i < builtInFunctions.size(); i++)
	{
		for(unsigned int j = 0; j < builtInFunctions[i].size(); j++)
		{
			FreeBuiltInPluginsInfrastructure(i, j);
		}
		builtInFunctions[i].clear();
	}
	builtInFunctions.clear();
}

int ScriptEnvironment::SetMemoryMax(int mem) {
  if (mem > 0) {
    __int64 mem_limit;
    memory_max = mem * (__int64)1048576;                          // mem as megabytes
    if (memory_max < memory_used) memory_max = memory_used; // can't be less than we already have

#if 0 // win32
    MEMORYSTATUS memstatus;
    GlobalMemoryStatus(&memstatus); // Correct call for a 32Bit process. -Ex gives numbers we cannot use!
	if (memstatus.dwAvailVirtual < memstatus.dwAvailPhys) // Check for big memory in Vista64
	  mem_limit = (__int64)memstatus.dwAvailVirtual;
	else
      mem_limit = (__int64)memstatus.dwAvailPhys;
#else
#ifdef __APPLE__
    vm_statistics64_data_t vmstats;
    mach_msg_type_number_t vmstatsz = HOST_VM_INFO64_COUNT;
    host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info_t)&vmstats, &vmstatsz);
    mem_limit = vmstats.free_count;
#elif defined(BSD)
    size_t szAvailPages = sizeof(mem_limit);
    sysctlbyname("vm.stats.vm.v_free_count", &mem_limit, &szAvailPages, NULL, 0);
#else // Linux
    mem_limit = sysconf(_SC_AVPHYS_PAGES);
#endif
    mem_limit *= sysconf(_SC_PAGE_SIZE);
#endif

    mem_limit += memory_used - (__int64)5242880;
    if (memory_max > mem_limit) memory_max = mem_limit;     // can't be more than 5Mb less than total
    if (memory_max < (__int64)4194304) memory_max = (__int64)4194304;	  // can't be less than 4Mb -- Tritical Jan 2006
  }
  return (int)(memory_max/(__int64)1048576);
}

int ScriptEnvironment::SetWorkingDir(const char * newdir) {
  return chdir(newdir) ? 1 : 0; //SetCurrentDirectory(newdir) ? 0 : 1;
}

void ScriptEnvironment::CheckVersion(int version) {
  if (version > AVISYNTH_INTERFACE_VERSION)
    ThrowError("Plugin was designed for a later version of Avisynth (%d)", version);
}


long GetCPUFlags() {
  static long lCPUExtensionsAvailable = CPUCheckForExtensions();
  return lCPUExtensionsAvailable;
}

long ScriptEnvironment::GetCPUFlags() { return CPU_id; }

void ScriptEnvironment::AddFunction(const char* name, const char* params, ApplyFunc apply, void* user_data) {
  function_table.AddFunction(name, params, apply, user_data);
}

void ScriptEnvironment::FreeBuiltInPluginsInfrastructure(unsigned int nFunctionFamilyIndex, unsigned int nFamilyMemberIndex)
{
	if(builtInFunctions[nFunctionFamilyIndex][nFamilyMemberIndex].name)
	{
		delete [] builtInFunctions[nFunctionFamilyIndex][nFamilyMemberIndex].name;
		builtInFunctions[nFunctionFamilyIndex][nFamilyMemberIndex].name = NULL;
	}

	if(builtInFunctions[nFunctionFamilyIndex][nFamilyMemberIndex].param_types)
	{
		delete [] builtInFunctions[nFunctionFamilyIndex][nFamilyMemberIndex].param_types;
		builtInFunctions[nFunctionFamilyIndex][nFamilyMemberIndex].param_types = NULL;
	}
	builtInFunctions[nFunctionFamilyIndex][nFamilyMemberIndex].apply		= NULL;
	builtInFunctions[nFunctionFamilyIndex][nFamilyMemberIndex].user_data	= NULL;
}

void ScriptEnvironment::AddBuiltInFunction(unsigned int nFunctionFamilyIndex, const char* name, const char* params, ApplyFunc apply, void* user_data)
{
	AVSFunction temp;

	unsigned int nBytes = strlen(name) + 1;

	temp.name = new char[nBytes];
	if(temp.name)
	{
		strcpy((char*)temp.name, name);
		((char*)temp.name)[nBytes - 1] = 0;
	}

    nBytes = strlen(params) + 1;
	temp.param_types = new char[nBytes];
	if(temp.param_types)
	{
		strcpy((char*)temp.param_types, params);
		((char*)(temp.param_types))[nBytes - 1] = 0;
	}

	temp.apply		= apply;
	temp.user_data	= user_data;

	builtInFunctions[nFunctionFamilyIndex].push_back(temp);
}

AVSValue ScriptEnvironment::GetVar(const char* name) {
  return var_table->Get(name);
}

bool ScriptEnvironment::SetVar(const char* name, const AVSValue& val) {
  return var_table->Set(name, val);
}

bool ScriptEnvironment::SetGlobalVar(const char* name, const AVSValue& val) {
  return global_var_table->Set(name, val);
}

const char* ScriptEnvironment::GetPluginDirectory()
{
  char* plugin_dir = NULL;
  try {
    plugin_dir = (char*)GetVar("$PluginDir$").AsString();
  }
  catch (...) {

    unsigned long nPluginsPathBytes = sizeof(char) /*slash*/ + sizeof(char) /*NULL*/;      
    char* runTimePath = getenv("AVXSYNTH_RUNTIME_PLUGIN_PATH");
    
    // In case that AVXSYNTH_RUNTIME_PLUGIN_PATH is relative path, this may come handy:
    char runTimeFullPath[PATH_MAX];
    memset(runTimeFullPath, 0, PATH_MAX*sizeof(char));
    char* ret = realpath(runTimePath, runTimeFullPath);
    if(runTimePath && (NULL == ret))
    {
        AVXLOG_WARN("%s", "*******************************************");
        AVXLOG_WARN("AVXSYNTH_RUNTIME_PLUGIN_PATH:\n %s\ndoes not exist (?)", runTimePath);
        AVXLOG_WARN("%s", "*******************************************");
    }
    bool bUseAvxsynthRuntimePluginPath = (runTimePath != NULL) && (NULL != ret) && (0 != strlen(runTimeFullPath));
    // quick fix: if the AVXSYNTH_PLUGIN_PATH is set to "/", empty the string, as one slash will be 
    // added immediately below anyways
    if(0 == strcmp("/", runTimeFullPath))
        memset(runTimeFullPath, 0, PATH_MAX*sizeof(char));
    if ( bUseAvxsynthRuntimePluginPath)
    {
        nPluginsPathBytes += strlen(runTimeFullPath);
    }
    else
    {
        nPluginsPathBytes += strlen(AVXSYNTH_PLUGIN_PATH);
    }
            
    char* strPluginsPath = new char[nPluginsPathBytes];
    memset(strPluginsPath, 0, nPluginsPathBytes*sizeof(char));
    
    if (bUseAvxsynthRuntimePluginPath)
    {
        sprintf(strPluginsPath, "%s/", runTimeFullPath);
    }
    else
    {
        sprintf(strPluginsPath, "%s/", AVXSYNTH_PLUGIN_PATH);        
    }
    
    SetGlobalVar("$PluginDir$", AVSValue(SaveString(strPluginsPath)));
    
    delete [] strPluginsPath;
    strPluginsPath = NULL;
    
    try {
	plugin_dir = (char*)GetVar("$PluginDir$").AsString();
    }
    catch (...)
    {
	return 0;
    }
  }

  AVXLOG_INFO("Plugins Dir: %s", plugin_dir);      
  return plugin_dir;
}

bool ScriptEnvironment::LoadInternalBuiltinPlugins(void)
{
	// 
	// During the course of development, some built-in plugins are still within this project,
	// (i.e. are not yet exported to DLL). This method will populate the storage
	unsigned int nFamilyIndex, nFunctionIndex;
    
    AVSFunction* funcFamilies[] = // pretty much the builtin_functions contents
    {
        NULL, //Audio_filters
        NULL, //Combine_filters_empty_placeholder,
        NULL, //Convert_filters_empty_placeholder,
        NULL, //Convolution_filters_empty_placeholder,
        NULL, //Edit_filters_empty_placeholder,
        NULL, //Field_filters_empty_placeholder,
        NULL, //Focus_filters_empty_placeholder,
        NULL, //Fps_filters_empty_placeholder,
        NULL, //Histogram_filters_empty_placeholder,
        NULL, //Layer_filters_empty_placeholder,
        NULL, //Levels_filters_empty_placeholder,
        NULL, //Misc_filters_empty_placeholder,
        NULL, //Resample_filters_empty_placeholder,
        NULL, //Resize_filters_empty_placeholder,
        Script_functions, 
        NULL, //Source_filters_empty_placeholder, 
        Text_filters,
        NULL, //Transform_filters_empty_placeholder, 
        NULL, //Merge_filters_empty_placeholder, 
        NULL, //Color_filters_empty_placeholder,
        Debug_filters,
        NULL, //Image_filters_empty_placeholder, 
        NULL, //Turn_filters_empty_placeholder,
        Conditional_filters, 
        Conditional_funtions_filters,
        Plugin_functions, 
        CPlugin_filters, 
        Cache_filters,
        NULL, //SSRC_filters_empty_placeholder, 
        NULL, //SuperEq_filters_empty_placeholder, 
        NULL, //Overlay_filters_empty_placeholder,
        NULL, //Greyscale_filters_empty_placeholder, 
        NULL, //Swap_filters_empty_placeholder,
        NULL, //Soundtouch_filters_empty_placeholder
    };

    unsigned int nItems[] = 
    {
        0, // BUILT_IN_AUDIO_FILTERS_NUMBER_OF_ITEMS, //                 (23)
        0, // BUILT_IN_COMBINE_FILTERS_NUMBER_OF_ITEMS, //               (6)
        0, // BUILT_IN_CONVERT_FILTERS_NUMBER_OF_ITEMS, //               (6)
        0, // BUILT_IN_CONVOLUTION_FILTERS_NUMBER_OF_ITEMS, //           (1)
        0, // BUILT_IN_EDIT_FILTERS_NUMBER_OF_ITEMS, //                  (20)
        0, // BUILT_IN_FIELD_FILTERS_NUMBER_OF_ITEMS, //                 (0)
        0, // BUILT_IN_FOCUS_FILTERS_NUMBER_OF_ITEMS, //                 (4)
        0, // BUILT_IN_FPS_FILTERS_NUMBER_OF_ITEMS, //                   (13)
        0, // BUILT_IN_HISTOGRAM_FILTERS_NUMBER_OF_ITEMS, //             (1)
        0, // BUILT_IN_LAYER_FILTERS_NUMBER_OF_ITEMS, //                 (12)
        0, // BUILT_IN_LEVELS_FILTERS_NUMBER_OF_ITEMS, //                (4)
        0, // BUILT_IN_MISC_FILTERS_NUMBER_OF_ITEMS, //                  (3)
        0, // BUILT_IN_RESAMPLE_FILTERS_NUMBER_OF_ITEMS, //              (10)
        0, // BUILT_IN_RESIZE_FILTERS_NUMBER_OF_ITEMS, //                (3)
        BUILT_IN_SCRIPT_FUNCTIONS_NUMBER_OF_ITEMS, //              (79)
        0, // BUILT_IN_SOURCE_FILTERS_NUMBER_OF_ITEMS, //                (14)
        BUILT_IN_TEXT_FILTERS_NUMBER_OF_ITEMS, //                  (6)
        0, // BUILT_IN_TRANSFORM_FILTERS_NUMBER_OF_ITEMS, //             (6)
        0, // BUILT_IN_MERGE_FILTERS_NUMBER_OF_ITEMS, //                 (3)
        0, // BUILT_IN_COLOR_FILTERS_NUMBER_OF_ITEMS, //                 (1)
        BUILT_IN_DEBUG_FILTERS_NUMBER_OF_ITEMS, //                 (2)
        0, // BUILT_IN_IMAGE_FILTERS_NUMBER_OF_ITEMS, //                 (3)
        0, // BUILT_IN_TURN_FILTERS_NUMBER_OF_ITEMS, //                  (3)
        BUILT_IN_CONDITIONAL_FILTERS_NUMBER_OF_ITEMS, //           (8)
        BUILT_IN_CONDITIONAL_FUNCTIONS_FILTERS_NUMBER_OF_ITEMS, // (27)
        BUILT_IN_PLUGIN_FUNCTIONS_NUMBER_OF_ITEMS, //              (1)
        BUILT_IN_CPLUGIN_FILTERS_NUMBER_OF_ITEMS, //               (2)
        BUILT_IN_CACHE_FILTERS_NUMBER_OF_ITEMS, //                 (2)
        0, // BUILT_IN_SSRC_FILTERS_NUMBER_OF_ITEMS, //                  (1)
        0, // BUILT_IN_SUPEREQ_FILTERS_NUMBER_OF_ITEMS, //               (1)
        0, // BUILT_IN_OVERLAY_FILTERS_NUMBER_OF_ITEMS, //               (1)
        0, // BUILT_IN_GRAYSCALE_FILTERS_NUMBER_OF_ITEMS, //             (2)
        0, // BUILT_IN_SWAP_FILTERS_NUMBER_OF_ITEMS, //                  (5)
        0, // BUILT_IN_SOUNDTOUCH_FILTERS_NUMBER_OF_ITEMS, //            (1)
    };
    

    for(nFamilyIndex = 0; nFamilyIndex < NUMBER_OF_BUILT_IN_FUNCTIONS_FAMILIES; nFamilyIndex++)
    {
        if(NULL == funcFamilies[nFamilyIndex])
            continue;
        
        for(nFunctionIndex = 0; nFunctionIndex < nItems[nFamilyIndex]; nFunctionIndex++)
        {
            AddBuiltInFunction(nFamilyIndex, 
                               funcFamilies[nFamilyIndex][nFunctionIndex].name, 
                               funcFamilies[nFamilyIndex][nFunctionIndex].param_types, 
                               funcFamilies[nFamilyIndex][nFunctionIndex].apply, 
                               funcFamilies[nFamilyIndex][nFunctionIndex].user_data);

            //DebugListBuiltInFunctions();
        }
        //DebugListBuiltInFunctions();
    }
    
	DebugListBuiltInFunctions();

	return true;
}

bool ScriptEnvironment::LoadBuiltInPlugins(void)
{
	AVXLOG_INFO("%s", "Loading core built-in plugins");
	AddAviSynthBuiltInFunctions(this);

	DebugListBuiltInFunctions();
	return true;
}

bool ScriptEnvironment::PluginsFolderIsNotEmpty()
{
  const char* plugin_dir = GetPluginDirectory();
  if(NULL == plugin_dir)
  {
    AVXLOG_FATAL("%s", "Undefined plugin directory");
    return false;
  }
 
  unsigned long nFolderPathLength = strlen(plugin_dir);
  DIR* pDir = opendir(plugin_dir);
  if(NULL == pDir)
  {
    AVXLOG_FATAL("%s", "Failed opening plugin directory");
    return false;
  }
    
  bool bFolderNotEmpty = false;
  struct dirent* pItem;
  while(NULL != (pItem = readdir(pDir)))
  {
      unsigned long nFilenameLength = strlen(pItem->d_name);
      if(1 == nFilenameLength || 2 == nFilenameLength)
	  continue; 	// exclude "." and ".." which are mandatory in each folder

      DIR* pIsDir = opendir(pItem->d_name);
      if(pIsDir)
          closedir(pIsDir);
      else
      {
        if(strstr(pItem->d_name, ".so") || strstr(pItem->d_name, ".so."))
        {
            bFolderNotEmpty = true;
            break;
        }
      }
  }
  
  closedir(pDir);
  return bFolderNotEmpty;
}

bool ScriptEnvironment::IsPluginNameAcceptable(char* pStrFilename)
{
    // Linux allows all kinds of strange filenames, opening up the opportunities
    // for people who might name the file something like 'rm -rf /' and toss it
    // to the avxplugins folder. Using such name to construct a string to execute
    // within the shell may cause a lot of trouble. 
    //
    // To prevent the security threat, we will here scrutinize the name of loaded
    // library file against the typical practices of naming Linux libraries
    //
    
    //
    // Extract the pure filename first
    //
    std::string strTest = pStrFilename;
    size_t nLastSlashPos = strTest.find_last_of("/");
    if(std::string::npos != nLastSlashPos)
        strTest = strTest.substr(nLastSlashPos + 1);
        
    //
    // It must have '.so' somewhere in the filename
    //
    size_t nDotSoPosition = strTest.find(".so");
    if(std::string::npos == nDotSoPosition)
    {
        AVXLOG_ERROR("Plugin filename \"%s\" does not have .so extension", pStrFilename);
        return false;
    }
    else
    {
        std::string strAfterDotSo = strTest.substr(nDotSoPosition + strlen(".so"));
        size_t nExtraChars = strAfterDotSo.length();
        for(size_t i = 0; i < nExtraChars; i++)
        {
            char chTest = strAfterDotSo[i];
            if(('.' != chTest) && (false == isdigit(chTest)))
            {
                AVXLOG_ERROR("Plugin filename \"%s\" has non-standard version string (after .so)", pStrFilename);
                return false;
            }   
        }
    }
    
    //
    // The names containing shell special characters will be rejected
    //
    size_t nLength = strTest.length();
    for(size_t i = 0; i < nLength; i++)
    {
        char chTest = strTest[i];
        if((';'  == chTest)  || 
           ('*'  == chTest)  || 
           ('?'  == chTest)  || 
           ('^'  == chTest)  ||
           ('$'  == chTest)  ||
           ('@'  == chTest)  ||
           ('\'' == chTest) ||
           ('\\' == chTest))
        {
            AVXLOG_ERROR("Plugin filename \"%s\" contains unusual characters", pStrFilename);
            return false;
        }
    }
    return true;
}

bool ScriptEnvironment::LoadPluginsMatching(const char* pattern)
{
  const char* plugin_dir = GetPluginDirectory();
  if(NULL == plugin_dir)
  {
    AVXLOG_FATAL("%s", "Undefined plugin directory");
    return false;
  }
 
  unsigned long nFolderPathLength = strlen(plugin_dir);
  DIR* pDir = opendir(plugin_dir);
  if(NULL == pDir)
  {
    AVXLOG_FATAL("%s", "Failed opening plugin directory");
    return false;
  }

  int count = 0;
  struct dirent* pItem;
  while(NULL != (pItem = readdir(pDir)))
  {
      unsigned long nFilenameLength = strlen(pItem->d_name);
      if(1 == nFilenameLength || 2 == nFilenameLength)
	  continue; 	// exclude "." and ".." which are mandatory in each folder

      if(false == IsPluginNameAcceptable(pItem->d_name))
        continue;
      
      unsigned long nPluginPathBytes = 1 + nFolderPathLength + nFilenameLength + 1; // last +1 is for '/' in between 
      
      char* pStrPluginPath = new char[nPluginPathBytes];
      memset(pStrPluginPath, 0, nPluginPathBytes*sizeof(char));
      sprintf(pStrPluginPath, "%s/%s", plugin_dir, pItem->d_name);
      
      ++count;
      if (count > 20) {
        HMODULE* loaded_plugins = (HMODULE*)GetVar("$Plugins$").AsString();
        FreeLibraries(loaded_plugins, this);
        count = 0;
      }
      
      //
      // Examine plugin here
      //
      AVXLOG_INFO("Examining plugin %s", pStrPluginPath);
      
      function_table.PrescanPluginStart(pStrPluginPath);
      
      AVSValue path(pStrPluginPath);
      AVSValue temp(&path, 1);
      LoadPlugin(AVSValue(&temp, 1), (void*)true, this);
      
      delete [] pStrPluginPath;
      pStrPluginPath = NULL;
  }
  
  closedir(pDir);
  return true;
#if 0
  WIN32_FIND_DATA FileData;
  char file[MAX_PATH];
  char* dummy;
//  const char* plugin_dir = GetPluginDirectory();

//  strcpy(file, plugin_dir);
//  strcat(file, "\\");
//  strcat(file, pattern);
  int count = 0;
  HANDLE hFind = FindFirstFile(pattern, &FileData);
  BOOL bContinue = (hFind != INVALID_HANDLE_VALUE);
  while (bContinue) {
    // we have to use full pathnames here
    ++count;
    if (count > 20) {
      HMODULE* loaded_plugins = (HMODULE*)GetVar("$Plugins$").AsString();
      FreeLibraries(loaded_plugins, this);
      count = 0;
    }
    GetFullPathName(FileData.cFileName, MAX_PATH, file, &dummy);
    function_table.PrescanPluginStart(file);
    LoadPlugin(AVSValue(&AVSValue(&AVSValue(file), 1), 1), (void*)true, this);
    bContinue = FindNextFile(hFind, &FileData);
    if (!bContinue) {
      FindClose(hFind);
      return true;
    }
  }
#endif
  return false;
}

bool ScriptEnvironment::IsFileExtension(char* pStrFilename, const char* pStrExtension)
{
    char* pReversedFilename = strdup(pStrFilename);
    pReversedFilename = _strrev(pReversedFilename);
    
    char* pReversedPattern = strdup(pStrExtension);
    pReversedPattern = _strrev(pReversedPattern);
    
    char* pMatchString = strstr(pReversedFilename, pReversedPattern);
    bool bRetValue = (pMatchString == pReversedFilename);
    
    free(pReversedFilename);
    free(pReversedPattern);
    return bRetValue;
}

bool ScriptEnvironment::LoadAVISynthCustomFunctionScripts(void)
{
  const char* plugin_dir = GetPluginDirectory();
  if(NULL == plugin_dir)
  {
    AVXLOG_FATAL("%s", "Undefined plugin directory");
    return false;
  }
 
  unsigned long nFolderPathLength = strlen(plugin_dir);
  DIR* pDir = opendir(plugin_dir);
  if(NULL == pDir)
  {
    AVXLOG_FATAL("%s", "Failed opening plugin directory, Error: %s", strerror(errno));
    return false;
  }

  struct dirent* pItem;
  while(NULL != (pItem = readdir(pDir)))
  {
      unsigned long nFilenameLength = strlen(pItem->d_name);
      if(1 == nFilenameLength || 2 == nFilenameLength)
        continue; 	// exclude "." and ".." which are mandatory in each folder
	  
      if(false == IsFileExtension(pItem->d_name, ".avsi"))
        continue;
      
      //
      // Import function script
      //
      AVXLOG_INFO("Importing script file %s", pItem->d_name);
      
      std::string strFullPluginPath = plugin_dir;
      strFullPluginPath += std::string(pItem->d_name);
      AVSValue name(strFullPluginPath.c_str());
      AVSValue temp(&name, 1);
      Import(AVSValue(&temp, 1), 0, this);
      
  }
  closedir(pDir);
  return true;
}

void ScriptEnvironment::PrescanPlugins()
{
  const char* plugin_dir = GetPluginDirectory();
  if(NULL == plugin_dir)  
  {
    AVXLOG_FATAL("%s", "Undefined plugin directory");
    return;
  }
 
  unsigned long nFolderPathLength = strlen(plugin_dir);
  DIR* pDir = opendir(plugin_dir);
  if(NULL == pDir)
  {
    AVXLOG_FATAL("%s", "Failed opening plugin directory, Error: %d", errno);
    return;
  }
  closedir(pDir);
    
  if(false == PluginsFolderIsNotEmpty())
  {
    AVXLOG_FATAL("%s", "Found empty plugin directory");
    return;
  }

  //
  // Prescan to fill out function_table
  //
  function_table.StartPrescanning();
  if(LoadPluginsMatching(".so"))
  {
    // Unloads all plugins
    HMODULE* loaded_plugins = (HMODULE*)GetVar("$Plugins$").AsString();
    FreeLibraries(loaded_plugins, this);
  }
  function_table.StopPrescanning();
  
  LoadAVISynthCustomFunctionScripts();
  return;
  
/*
  WIN32_FIND_DATA FileData;
  HANDLE hFind = FindFirstFile(plugin_dir, &FileData);
  if (hFind != INVALID_HANDLE_VALUE) {
    FindClose(hFind);
    CWDChanger cwdchange(plugin_dir);
    function_table.StartPrescanning();
    if (LoadPluginsMatching("*.dll") | LoadPluginsMatching("*.vdf")) {  // not || because of shortcut boolean eval.
      // Unloads all plugins
      HMODULE* loaded_plugins = (HMODULE*)GetVar("$Plugins$").AsString();
      FreeLibraries(loaded_plugins, this);
    }
    function_table.StopPrescanning();

    char file[MAX_PATH];
    strcpy(file, plugin_dir);
    strcat(file, "\\*.avsi");
    hFind = FindFirstFile(file, &FileData);
    BOOL bContinue = (hFind != INVALID_HANDLE_VALUE);
    while (bContinue) {
      Import(AVSValue(&AVSValue(&AVSValue(FileData.cFileName), 1), 1), 0, this);
      bContinue = FindNextFile(hFind, &FileData);
      if (!bContinue) FindClose(hFind);
    }  
  }*/
}

void ScriptEnvironment::ExportFilters()
{
  string builtin_names;

  for (unsigned int i = 0; i < builtInFunctions.size(); ++i) {
    for (unsigned int j = 0; j < builtInFunctions[i].size(); j++) {
	  if(NULL == builtInFunctions[i][j].name)
		  break;
      builtin_names += builtInFunctions[i][j].name;
      builtin_names += " ";
      
      string param_id = string("$Plugin!") + builtInFunctions[i][j].name + "!Param$";
      SetGlobalVar( SaveString(param_id.c_str(), param_id.length() + 1), AVSValue(builtInFunctions[i][j].param_types) );
    }
  }

  SetGlobalVar("$InternalFunctions$", AVSValue( SaveString(builtin_names.c_str(), builtin_names.length() + 1) ));
}


PVideoFrame ScriptEnvironment::NewPlanarVideoFrame(int width, int height, int align, bool U_first) {
  int UVpitch, Uoffset, Voffset, pitch;

  if (align<0) {
// Forced alignment - pack Y as specified, pack UV half that
    align = -align;
	pitch = (width+align-1) / align * align;  // Y plane, width = 1 byte per pixel
	UVpitch = (pitch+1)>>1;  // UV plane, width = 1/2 byte per pixel - can't align UV planes seperately.
  } 
  else if (PlanarChromaAlignmentState) {
// Align UV planes, Y will follow
	UVpitch = (((width+1)>>1)+align-1) / align * align;  // UV plane, width = 1/2 byte per pixel
	pitch = UVpitch<<1;  // Y plane, width = 1 byte per pixel
  }
  else {
// Do legacy alignment
	pitch = (width+align-1) / align * align;  // Y plane, width = 1 byte per pixel
	UVpitch = (pitch+1)>>1;  // UV plane, width = 1/2 byte per pixel - can't align UV planes seperately.
  }

  const int size = pitch * height + UVpitch * height;
  const int _align = (align < FRAME_ALIGN) ? FRAME_ALIGN : align;
  VideoFrameBuffer* vfb = GetFrameBuffer(size+(_align*4));
  if (!vfb)
    ThrowError("NewPlanarVideoFrame: Returned 0 image pointer!");
#ifdef _DEBUG
  {
    static const BYTE filler[] = { 0x0A, 0x11, 0x0C, 0xA7, 0xED };
    BYTE* p = vfb->GetWritePtr();
    BYTE* q = p + vfb->GetDataSize()/5*5;
    for (; p<q; p+=5) {
      p[0]=filler[0]; p[1]=filler[1]; p[2]=filler[2]; p[3]=filler[3]; p[4]=filler[4];
    }
  }
#endif
  const int offset = (-(size_t)(vfb->GetWritePtr())) & (FRAME_ALIGN-1);  // align first line offset

  if (U_first) {
    Uoffset = offset + pitch * height;
    Voffset = offset + pitch * height + UVpitch * (height>>1);
  } else {
    Voffset = offset + pitch * height;
    Uoffset = offset + pitch * height + UVpitch * (height>>1);
  }
  return new VideoFrame(vfb, offset, pitch, width, height, Uoffset, Voffset, UVpitch);
}


PVideoFrame ScriptEnvironment::NewVideoFrame(int row_size, int height, int align) {
  const int pitch = (row_size+align-1) / align * align;
  const int size = pitch * height;
  const int _align = (align < FRAME_ALIGN) ? FRAME_ALIGN : align;
  VideoFrameBuffer* vfb = GetFrameBuffer(size+(_align*4));
  if (!vfb)
    ThrowError("NewVideoFrame: Returned 0 image pointer!");
#ifdef _DEBUG
  {
    static const BYTE filler[] = { 0x0A, 0x11, 0x0C, 0xA7, 0xED };
    BYTE* p = vfb->GetWritePtr();
    BYTE* q = p + vfb->GetDataSize()/5*5;
    for (; p<q; p+=5) {
      p[0]=filler[0]; p[1]=filler[1]; p[2]=filler[2]; p[3]=filler[3]; p[4]=filler[4];
    }
  }
#endif
  const int offset = (-(size_t)(vfb->GetWritePtr())) & (FRAME_ALIGN-1);  // align first line offset  (alignment is free here!)
  return new VideoFrame(vfb, offset, pitch, row_size, height);
}


PVideoFrame __stdcall ScriptEnvironment::NewVideoFrame(const VideoInfo& vi, int align) {
  // Check requested pixel_type:
  switch (vi.pixel_type) {
    case VideoInfo::CS_BGR24:
    case VideoInfo::CS_BGR32:
    case VideoInfo::CS_YUY2:
    case VideoInfo::CS_YV12:
    case VideoInfo::CS_I420:
      break;
    default:
      ThrowError("Filter Error: Filter attempted to create VideoFrame with invalid pixel_type.");
  }
  // If align is negative, it will be forced, if not it may be made bigger
  if (vi.IsPlanar()) { // Planar requires different math ;)
    if (align>=0) {
      align = std::max(align,FRAME_ALIGN);
    }
    if ((vi.height&1)||(vi.width&1))
      ThrowError("Filter Error: Attempted to request an YV12 frame that wasn't mod2 in width and height!");
    return ScriptEnvironment::NewPlanarVideoFrame(vi.width, vi.height, align, !vi.IsVPlaneFirst());  // If planar, maybe swap U&V
  } else {
    if ((vi.width&1)&&(vi.IsYUY2()))
      ThrowError("Filter Error: Attempted to request an YUY2 frame that wasn't mod2 in width.");
    if (align<0) {
      align *= -1;
    } else {
      align = std::max(align,FRAME_ALIGN);
    }
    return NewVideoFrame(vi.RowSize(), vi.height, align);
  }
}

bool ScriptEnvironment::MakeWritable(PVideoFrame* pvf) {
  const PVideoFrame& vf = *pvf;
  // If the frame is already writable, do nothing.
  if (vf->IsWritable()) {
    return false;
  }

  // Otherwise, allocate a new frame (using NewVideoFrame) and
  // copy the data into it.  Then modify the passed PVideoFrame
  // to point to the new buffer.
  const int row_size = vf->GetRowSize();
  const int height = vf->GetHeight();
  PVideoFrame dst;

  if (vf->GetPitch(PLANAR_U)) {  // we have no videoinfo, so we assume that it is Planar if it has a U plane.
    dst = NewPlanarVideoFrame(row_size, height, FRAME_ALIGN,false);  // Always V first on internal images
  } else {
    dst = NewVideoFrame(row_size, height, FRAME_ALIGN);
  }
  BitBlt(dst->GetWritePtr(), dst->GetPitch(), vf->GetReadPtr(), vf->GetPitch(), row_size, height);
  // Blit More planes (pitch, rowsize and height should be 0, if none is present)
  BitBlt(dst->GetWritePtr(PLANAR_V), dst->GetPitch(PLANAR_V), vf->GetReadPtr(PLANAR_V), vf->GetPitch(PLANAR_V), vf->GetRowSize(PLANAR_V), vf->GetHeight(PLANAR_V));
  BitBlt(dst->GetWritePtr(PLANAR_U), dst->GetPitch(PLANAR_U), vf->GetReadPtr(PLANAR_U), vf->GetPitch(PLANAR_U), vf->GetRowSize(PLANAR_U), vf->GetHeight(PLANAR_U));

  *pvf = dst;
  return true;
}


void ScriptEnvironment::AtExit(IScriptEnvironment::ShutdownFunc function, void* user_data) {
  at_exit.Add(function, user_data);
}

void ScriptEnvironment::PushContext(int level) {
  var_table = new VarTable(var_table, global_var_table);
}

void ScriptEnvironment::PopContext() {
  var_table = var_table->Pop();
}

void ScriptEnvironment::PopContextGlobal() {
  global_var_table = global_var_table->Pop();
}


PVideoFrame __stdcall ScriptEnvironment::Subframe(PVideoFrame src, int rel_offset, int new_pitch, int new_row_size, int new_height) {
  return src->Subframe(rel_offset, new_pitch, new_row_size, new_height);
}

//tsp June 2005 new function compliments the above function
PVideoFrame __stdcall ScriptEnvironment::SubframePlanar(PVideoFrame src, int rel_offset, int new_pitch, int new_row_size,
                                                        int new_height, int rel_offsetU, int rel_offsetV, int new_pitchUV) {
  return src->Subframe(rel_offset, new_pitch, new_row_size, new_height, rel_offsetU, rel_offsetV, new_pitchUV);
}

void* ScriptEnvironment::ManageCache(int key, void* data){
// An extensible interface for providing system or user access to the
// ScriptEnvironment class without extending the IScriptEnvironment
// definition.

  switch (key)
  {
  // Allow the cache to designate a VideoFrameBuffer as expired thus
  // allowing it to be reused in favour of any of it's older siblings.
  case MC_ReturnVideoFrameBuffer:
  {
	LinkedVideoFrameBuffer *lvfb = (LinkedVideoFrameBuffer*)data;

	// The Cache volunteers VFB's it no longer tracks for reuse. This closes the loop
	// for Memory Management. MC_PromoteVideoFrameBuffer moves VideoFrameBuffer's to
	// the head of the list and here we move unloved VideoFrameBuffer's to the end.

	// Check to make sure the vfb wasn't discarded and is really a LinkedVideoFrameBuffer.
	if ((lvfb->data == 0) || (lvfb->signature != LinkedVideoFrameBuffer::ident)) break;

    // Adjust unpromoted sublist if required
    if (unpromotedvfbs == lvfb) unpromotedvfbs = lvfb->next;

	// Move unloved VideoFrameBuffer's to the end of the video_frame_buffers LRU list.
	Relink(video_frame_buffers.prev, lvfb, &video_frame_buffers);

	// Flag it as returned, i.e. for immediate reuse.
	lvfb->returned = true;

	return (void*)1;
  }
  // Allow the cache to designate a VideoFrameBuffer as being managed thus
  // preventing it being reused as soon as it becomes free.
  case MC_ManageVideoFrameBuffer:
  {
	LinkedVideoFrameBuffer *lvfb = (LinkedVideoFrameBuffer*)data;

	// Check to make sure the vfb wasn't discarded and is really a LinkedVideoFrameBuffer.
	if ((lvfb->data == 0) || (lvfb->signature != LinkedVideoFrameBuffer::ident)) break;

	// Flag it as not returned, i.e. currently managed
	lvfb->returned = false;

	return (void*)1;
  }
  // Allow the cache to designate a VideoFrameBuffer as cacheable thus
  // requesting it be moved to the head of the video_frame_buffers LRU list.
  case MC_PromoteVideoFrameBuffer:
  {
	LinkedVideoFrameBuffer *lvfb = (LinkedVideoFrameBuffer*)data;

	// When a cache instance detects attempts to refetch previously generated frames
	// it starts to promote VFB's to the head of the video_frame_buffers LRU list.
	// Previously all VFB's cycled to the head now only cacheable ones do.

	// Check to make sure the vfb wasn't discarded and is really a LinkedVideoFrameBuffer.
	if ((lvfb->data == 0) || (lvfb->signature != LinkedVideoFrameBuffer::ident)) break;

    // Adjust unpromoted sublist if required
    if (unpromotedvfbs == lvfb) unpromotedvfbs = lvfb->next;

	// Move loved VideoFrameBuffer's to the head of the video_frame_buffers LRU list.
    Relink(&video_frame_buffers, lvfb, video_frame_buffers.next);

	// Flag it as not returned, i.e. currently managed
	lvfb->returned = false;

	return (void*)1;
  }
  // Register Cache instances onto a linked list, so all Cache instances
  // can be poked as a single unit thru the PokeCache interface
  case MC_RegisterCache:
  {
	Cache *cache = (Cache*)data;

	if (CacheHead) CacheHead->priorCache = &(cache->nextCache);
	cache->priorCache = &CacheHead;

	cache->nextCache = CacheHead;
	CacheHead = cache;

	return (void*)1;
  }
  default:
    break;
  }
  return 0;
}


bool ScriptEnvironment::PlanarChromaAlignment(IScriptEnvironment::PlanarChromaAlignmentMode key){
  bool oldPlanarChromaAlignmentState = PlanarChromaAlignmentState;

  switch (key)
  {
  case IScriptEnvironment::PlanarChromaAlignmentOff:
  {
	PlanarChromaAlignmentState = false;
    break;
  }
  case IScriptEnvironment::PlanarChromaAlignmentOn:
  {
	PlanarChromaAlignmentState = true;
    break;
  }
  default:
    break;
  }
  return oldPlanarChromaAlignmentState;
}


LinkedVideoFrameBuffer* ScriptEnvironment::NewFrameBuffer(int size) {
  memory_used += size;
  AVXLOG_DEBUG("Frame buffer memory used: %ld", memory_used);
  return new LinkedVideoFrameBuffer(size);
}


LinkedVideoFrameBuffer* ScriptEnvironment::GetFrameBuffer2(int size) {
  LinkedVideoFrameBuffer *i, *j;

  // Before we allocate a new framebuffer, check our memory usage, and if we
  // are 12.5% or more above allowed usage discard some unreferenced frames.
  if (memory_used >=  memory_max + std::max((__int64)size, (memory_max >> 3)) ) {
    ++g_Mem_stats.CleanUps;
    int freed = 0;
    int freed_count = 0;
    // Deallocate enough unused frames.
    for (i = video_frame_buffers.prev; i != &video_frame_buffers; i = i->prev) {
      if (i->GetRefcount() == 0) {
        if (i->next != i->prev) {
          // Adjust unpromoted sublist if required
          if (unpromotedvfbs == i) unpromotedvfbs = i->next;
          // Store size.
          freed += i->data_size;
          freed_count++;
          // Delete data;
          i->~LinkedVideoFrameBuffer();  // Can't delete me because caches have pointers to me
          // Link onto tail of lost_video_frame_buffers chain.
          j = i;
          i = i -> next; // step back one
          Relink(lost_video_frame_buffers.prev, j, &lost_video_frame_buffers);
          if ((memory_used+size - freed) < memory_max)
            break; // Stop, we are below 100% utilization
        }
        else break;
      }
    }
    AVXLOG_INFO("Freed %d frames, consisting of %d bytes.",freed_count, freed);
    memory_used -= freed;
	g_Mem_stats.Losses += freed_count;
  } 

  // Plan A: When we are below our memory usage :-
  if (memory_used + size < memory_max) {
    //   Part 1: look for a returned free buffer of the same size and reuse it
    for (i = video_frame_buffers.prev; i != &video_frame_buffers; i = i->prev) {
      if (i->returned && (i->GetRefcount() == 0) && (i->GetDataSize() == size)) {
        ++g_Mem_stats.PlanA1;
        return i;
      }
    }
    //   Part 2: else just allocate a new buffer
    ++g_Mem_stats.PlanA2;
    return NewFrameBuffer(size);
  }

  // To avoid Plan D we prod the caches to surrender any VFB's
  // they maybe guarding. We start gently and ask for just the
  // most recently locked VFB from previous cycles, then we ask
  // for the most recently locked VFB, then we ask for all the
  // locked VFB's. Next we start on the CACHE_RANGE protected
  // VFB's, as an offset we promote these.
  // Plan C is not invoked until the Cache's have been poked once.
  for (int c=Cache::PC_Nil; c <= Cache::PC_UnProtectAll; c++) {
    CacheHead->PokeCache(c, size, this);

    // Plan B: Steal the oldest existing free buffer of the same size
    j = 0;
    for (i = video_frame_buffers.prev; i != &video_frame_buffers; i = i->prev) {
      if (i->GetRefcount() == 0) {
        if (i->GetDataSize() == size) {
          ++g_Mem_stats.PlanB;
          InterlockedIncrement((long*)&i->sequence_number);  // Signal to the cache that the vfb has been stolen
          return i;
        }
        if (i->GetDataSize() > size) {
          // Remember the smallest VFB that is bigger than our size
          if ((j == 0) || (i->GetDataSize() < j->GetDataSize())) j = i;
        }
      }
    }

    // Plan C: Steal the oldest, smallest free buffer that is greater in size
    if (j && c > Cache::PC_Nil) {
      ++g_Mem_stats.PlanC;
      InterlockedIncrement((long*)&j->sequence_number);  // Signal to the cache that the vfb has been stolen
      return j;
    }
  }

  // Plan D: Allocate a new buffer, regardless of current memory usage
  ++g_Mem_stats.PlanD;
  return NewFrameBuffer(size);
}

VideoFrameBuffer* ScriptEnvironment::GetFrameBuffer(int size) {
  LinkedVideoFrameBuffer* result = GetFrameBuffer2(size);

  if (!result || !result->data) {
    // Damn! we got a NULL from malloc
    AVXLOG_INFO("GetFrameBuffer failure, size=%d, memory_max=%ld, memory_used=%ld", size, memory_max, memory_used);

    // Put that VFB on the lost souls chain
    if (result) Relink(lost_video_frame_buffers.prev, result, &lost_video_frame_buffers);

	__int64 save_max = memory_max;

    // Set memory_max to 12.5% below memory_used 
    memory_max = std::max((__int64)(4*1024*1024), memory_used - std::max((__int64)size, (memory_used/9)));

    // Retry the request
    result = GetFrameBuffer2(size);

	memory_max = save_max;

    if (!result || !result->data) {
      // Damn!! Damn!! we are really screwed, winge!
      if (result) Relink(lost_video_frame_buffers.prev, result, &lost_video_frame_buffers);

      ThrowError("GetFrameBuffer: Returned a VFB with a 0 data pointer!\n"
                 "size=%d, max=%ld, used=%ld\n"
                 "I think we have run out of memory folks!", size, memory_max, memory_used);
    }
  }

#if 0
# if 0
  // Link onto head of video_frame_buffers chain.
  Relink(&video_frame_buffers, result, video_frame_buffers.next);
# else
  // Link onto tail of video_frame_buffers chain.
  Relink(video_frame_buffers.prev, result, &video_frame_buffers);
# endif
#else
  // Link onto head of unpromoted video_frame_buffers chain.
  Relink(unpromotedvfbs->prev, result, unpromotedvfbs);
  // Adjust unpromoted sublist
  unpromotedvfbs = result;
#endif
  // Flag it as returned, i.e. currently not managed
  result->returned = true;
  return result;
}


int ScriptEnvironment::Flatten(const AVSValue& src, AVSValue* dst, int index, int max, const char** arg_names) {
  if (src.IsArray()) {
    const int array_size = src.ArraySize();
    for (int i=0; i<array_size; ++i) {
      if (!arg_names || arg_names[i] == 0)
        index = Flatten(src[i], dst, index, max);
    }
  } else {
    if (index < max) {
      dst[index++] = src;
    } else {
      ThrowError("Too many arguments passed to function (max. is %d)", max);
    }
  }
  return index;
}


AVSValue ScriptEnvironment::Invoke(const char* name, const AVSValue args, const char** arg_names) {

  int args2_count;
  bool strict;
  AVSFunction *f;
  AVSValue retval;

  const int args_names_count = (arg_names && args.IsArray()) ? args.ArraySize() : 0;

  AVSValue *args1 = new AVSValue[ScriptParser::max_args]; // Save stack space - put on heap!!!

  try {
	// flatten unnamed args
	args2_count = Flatten(args, args1, 0, ScriptParser::max_args, arg_names);

	// find matching function
	f = function_table.Lookup(name, args1, args2_count, &strict, args_names_count, arg_names);
	if (!f)
	  throw NotFound();
  }
  catch (...) {
    delete[] args1;
	throw;
  }

  // collapse the 1024 element array
  AVSValue *args2 = new AVSValue[args2_count];
  for (int i=0; i< args2_count; i++)
    args2[i] = args1[i];
  delete[] args1;

  // combine unnamed args into arrays
  int src_index=0, dst_index=0;
  const char* p = f->param_types;
  const int maxarg3 = std::max(args2_count, (int)strlen(p)); // well it can't be any longer than this.

  AVSValue *args3 = new AVSValue[maxarg3];

  try {
	while (*p) {
	  if (*p == '[') {
		p = strchr(p+1, ']');
		if (!p) break;
		p++;
	  } else if (p[1] == '*' || p[1] == '+') {
		int start = src_index;
		while (src_index < args2_count && FunctionTable::SingleTypeMatch(*p, args2[src_index], strict))
		  src_index++;
		args3[dst_index++] = AVSValue(&args2[start], src_index - start); // can't delete args2 early because of this
		p += 2;
	  } else {
		if (src_index < args2_count)
		  args3[dst_index] = args2[src_index];
		src_index++;
		dst_index++;
		p++;
	  }
	}
	if (src_index < args2_count)
	  ThrowError("Too many arguments to function %s", name);

	const int args3_count = dst_index;

	// copy named args
	for (int i=0; i<args_names_count; ++i) {
	  if (arg_names[i]) {
		int named_arg_index = 0;
		for (const char* p = f->param_types; *p; ++p) {
		  if (*p == '*' || *p == '+') {
			continue;   // without incrementing named_arg_index
		  } else if (*p == '[') {
			p += 1;
			const char* q = strchr(p, ']');
			if (!q) break;
			if (strlen(arg_names[i]) == unsigned(q-p) && !strncasecmp(arg_names[i], p, q-p)) {
			  // we have a match
			  if (args3[named_arg_index].Defined()) {
				ThrowError("Script error: the named argument \"%s\" was passed more than once to %s", arg_names[i], name);
			  } else if (args[i].IsArray()) {
				ThrowError("Script error: can't pass an array as a named argument");
			  } else if (args[i].Defined() && !FunctionTable::SingleTypeMatch(q[1], args[i], false)) {
				ThrowError("Script error: the named argument \"%s\" to %s had the wrong type", arg_names[i], name);
			  } else {
				args3[named_arg_index] = args[i];
				goto success;
			  }
			} else {
			  p = q+1;
			}
		  }
		  named_arg_index++;
		}
		// failure
		ThrowError("Script error: %s does not have a named argument \"%s\"", name, arg_names[i]);
success:;
	  }
	}
	// ... and we're finally ready to make the call
	retval = f->apply(AVSValue(args3, args3_count), f->user_data, this);
  }
  catch (...) {
    delete[] args3;
	delete[] args2;
	throw;
  }
  delete[] args3;
  delete[] args2;

  return retval;
}


bool ScriptEnvironment::FunctionExists(const char* name) {
  return function_table.Exists(name);
}

void BitBlt(BYTE* dstp, int dst_pitch, const BYTE* srcp, int src_pitch, int row_size, int height) {
  if ( (!height)|| (!row_size)) return;
  if (GetCPUFlags() & CPUF_INTEGER_SSE) {
    if (height == 1 || (src_pitch == dst_pitch && dst_pitch == row_size)) {
      memcpy_amd(dstp, srcp, row_size*height);
    } else {
      asm_BitBlt_ISSE(dstp,dst_pitch,srcp,src_pitch,row_size,height);
    }
    return;
  }
  if (height == 1 || (dst_pitch == src_pitch && src_pitch == row_size)) {
    memcpy(dstp, srcp, row_size*height);
  } else {
    for (int y=height; y>0; --y) {
      memcpy(dstp, srcp, row_size);
      dstp += dst_pitch;
      srcp += src_pitch;
    }
  }
}

  /*****************************
  * Assembler bitblit by Steady
   *****************************/


void asm_BitBlt_ISSE(BYTE* dstp, int dst_pitch, const BYTE* srcp, int src_pitch, int row_size, int height) {

  // Warning! : If you modify this routine, check the generated assembler to make sure
  //            the stupid compiler is saving the ebx register in the entry prologue.
  //            And don't just add an extra push/pop ebx pair around the code, try to
  //            convince the compiler to do the right thing, it's not hard, usually a
  //            slight shuffle or a well placed "__asm mov ebx,ebx" does the trick.

  if(row_size==0 || height==0) return; //abort on goofs
  //move backwards for easier looping and to disable hardware prefetch
  const BYTE* srcStart=srcp+src_pitch*(height-1);
  BYTE* dstStart=dstp+dst_pitch*(height-1);

  if(row_size < 64) {
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
    _asm {
      mov   esi,srcStart  //move rows from bottom up
      mov   edi,dstStart
      mov   edx,row_size
      dec   edx
      mov   ebx,height
      align 16
memoptS_rowloop:
      mov   ecx,edx
//      rep movsb
memoptS_byteloop:
      mov   AL,[esi+ecx]
      mov   [edi+ecx],AL
      sub   ecx,1
      jnc   memoptS_byteloop
      sub   esi,src_pitch
      sub   edi,dst_pitch
      dec   ebx
      jne   memoptS_rowloop
    };
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
    return;
  }//end small version

  else if( ((size_t)(dstp) | row_size | src_pitch | dst_pitch) & 7) {//not QW aligned
    //unaligned version makes no assumptions on alignment

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE
    _asm {
//****** initialize
      mov   esi,srcStart  //bottom row
      mov   AL,[esi]
      mov   edi,dstStart
      mov   edx,row_size
      mov   ebx,height

//********** loop starts here ***********

      align 16
memoptU_rowloop:
      mov   ecx,edx     //row_size
      dec   ecx         //offset to last byte in row
      add   ecx,esi     //ecx= ptr last byte in row
      and   ecx,~63     //align to first byte in cache line
memoptU_prefetchloop:
      mov   AX,[ecx]    //tried AL,AX,EAX, AX a tiny bit faster
      sub   ecx,64
      cmp   ecx,esi
      jae   memoptU_prefetchloop

//************ write *************

      movq    mm6,[esi]     //move the first unaligned bytes
      movntq  [edi],mm6
//************************
      mov   eax,edi
      neg   eax
      mov   ecx,eax
      and   eax,63      //eax=bytes from [edi] to start of next 64 byte cache line
      and   ecx,7       //ecx=bytes from [edi] to next QW
      align 16
memoptU_prewrite8loop:        //write out odd QW's so 64 bit write is cache line aligned
      cmp   ecx,eax           //start of cache line ?
      jz    memoptU_pre8done  //if not, write single QW
      movq    mm7,[esi+ecx]
      movntq  [edi+ecx],mm7
      add   ecx,8
      jmp   memoptU_prewrite8loop

      align 16
memoptU_write64loop:
      movntq  [edi+ecx-64],mm0
      movntq  [edi+ecx-56],mm1
      movntq  [edi+ecx-48],mm2
      movntq  [edi+ecx-40],mm3
      movntq  [edi+ecx-32],mm4
      movntq  [edi+ecx-24],mm5
      movntq  [edi+ecx-16],mm6
      movntq  [edi+ecx- 8],mm7
memoptU_pre8done:
      add   ecx,64
      cmp   ecx,edx         //while(offset <= row_size) do {...
      ja    memoptU_done64
      movq    mm0,[esi+ecx-64]
      movq    mm1,[esi+ecx-56]
      movq    mm2,[esi+ecx-48]
      movq    mm3,[esi+ecx-40]
      movq    mm4,[esi+ecx-32]
      movq    mm5,[esi+ecx-24]
      movq    mm6,[esi+ecx-16]
      movq    mm7,[esi+ecx- 8]
      jmp   memoptU_write64loop
memoptU_done64:

      sub     ecx,64    //went to far
      align 16
memoptU_write8loop:
      add     ecx,8           //next QW
      cmp     ecx,edx         //any QW's left in row ?
      ja      memoptU_done8
      movq    mm0,[esi+ecx-8]
      movntq  [edi+ecx-8],mm0
      jmp   memoptU_write8loop
memoptU_done8:

      movq    mm1,[esi+edx-8] //write the last unaligned bytes
      movntq  [edi+edx-8],mm1
      sub   esi,src_pitch
      sub   edi,dst_pitch
      dec   ebx               //row counter (=height at start)
      jne   memoptU_rowloop

      sfence
      emms
    };
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
    return;
  }//end unaligned version

  else {//QW aligned version (fastest)
  //else dstp and row_size QW aligned - hope for the best from srcp
  //QW aligned version should generally be true when copying full rows
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE  
    _asm {
      mov   esi,srcStart  //start of bottom row
      mov   edi,dstStart
      mov   ebx,height
      mov   edx,row_size
      align 16
memoptA_rowloop:
      mov   ecx,edx //row_size
      dec   ecx     //offset to last byte in row

//********forward routine
      add   ecx,esi
      and   ecx,~63   //align prefetch to first byte in cache line(~3-4% faster)
      align 16
memoptA_prefetchloop:
      mov   AX,[ecx]
      sub   ecx,64
      cmp   ecx,esi
      jae   memoptA_prefetchloop

      mov   eax,edi
      xor   ecx,ecx
      neg   eax
      and   eax,63            //eax=bytes from edi to start of cache line
      align 16
memoptA_prewrite8loop:        //write out odd QW's so 64bit write is cache line aligned
      cmp   ecx,eax           //start of cache line ?
      jz    memoptA_pre8done  //if not, write single QW
      movq    mm7,[esi+ecx]
      movntq  [edi+ecx],mm7
      add   ecx,8
      jmp   memoptA_prewrite8loop

      align 16
memoptA_write64loop:
      movntq  [edi+ecx-64],mm0
      movntq  [edi+ecx-56],mm1
      movntq  [edi+ecx-48],mm2
      movntq  [edi+ecx-40],mm3
      movntq  [edi+ecx-32],mm4
      movntq  [edi+ecx-24],mm5
      movntq  [edi+ecx-16],mm6
      movntq  [edi+ecx- 8],mm7
memoptA_pre8done:
      add   ecx,64
      cmp   ecx,edx
      ja    memoptA_done64    //less than 64 bytes left
      movq    mm0,[esi+ecx-64]
      movq    mm1,[esi+ecx-56]
      movq    mm2,[esi+ecx-48]
      movq    mm3,[esi+ecx-40]
      movq    mm4,[esi+ecx-32]
      movq    mm5,[esi+ecx-24]
      movq    mm6,[esi+ecx-16]
      movq    mm7,[esi+ecx- 8]
      jmp   memoptA_write64loop

memoptA_done64:
      sub   ecx,64

      align 16
memoptA_write8loop:           //less than 8 QW's left
      add   ecx,8
      cmp   ecx,edx
      ja    memoptA_done8     //no QW's left
      movq    mm7,[esi+ecx-8]
      movntq  [edi+ecx-8],mm7
      jmp   memoptA_write8loop

memoptA_done8:
      sub   esi,src_pitch
      sub   edi,dst_pitch
      dec   ebx               //row counter (height)
      jne   memoptA_rowloop

      sfence
      emms
    };
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
    return;
  }//end aligned version
}//end BitBlt_memopt()



void ScriptEnvironment::BitBlt(BYTE* dstp, int dst_pitch, const BYTE* srcp, int src_pitch, int row_size, int height) {
  if (height<0)
    ThrowError("Filter Error: Attempting to blit an image with negative height.");
  if (row_size<0)
    ThrowError("Filter Error: Attempting to blit an image with negative row size.");
  avxsynth::BitBlt(dstp, dst_pitch, srcp, src_pitch, row_size, height);
}


char* ScriptEnvironment::SaveString(const char* s, int len) {
  return string_dump.SaveString(s, len);
}


char* ScriptEnvironment::VSprintf(const char* fmt, va_list val) {
  char *buf = NULL;
  int size = 0, count = -1;
  while (count == -1)
  {
    if (buf) delete[] buf;
    size += 4096;
    buf = new char[size];
    count = vsnprintf(buf, size-1, fmt, val);
  }

  char *i = ScriptEnvironment::SaveString(buf);
  delete[] buf;
  return i;
}

char* ScriptEnvironment::Sprintf(const char* fmt, ...) {
  va_list val;
  va_start(val, fmt);
  char* result = ScriptEnvironment::VSprintf(fmt, val);
  va_end(val);
  return result;
}


void ScriptEnvironment::ThrowError(const char* fmt, ...) {
  va_list val;
  va_start(val, fmt);
  char buf[8192];
  vsnprintf(buf,sizeof(buf)-1, fmt, val);
  va_end(val);
  buf[sizeof(buf)-1] = '\0';
  
  AVXLOG_ERROR("%s", buf);
  throw AvisynthError(ScriptEnvironment::SaveString(buf));
}


IScriptEnvironment* __stdcall CreateScriptEnvironment(int version) {
  if (loadplugin_prefix) free((void*)loadplugin_prefix);
  loadplugin_prefix = 0;
  if (version <= AVISYNTH_INTERFACE_VERSION)
    return new ScriptEnvironment;
  else
    return 0;
}

}; // namespace avxsynth
