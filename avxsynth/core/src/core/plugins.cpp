// Avisynth v4.0  Copyright 2002 Ben Rudiak-Gould et al.
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
#include "windowsPorts.h"
#include "Error.h"    // which includes "internal.h"
#include <stdarg.h>
#include <dlfcn.h>
#include "avxlog.h"
#include <limits.h>
#include <iostream>
//#include <fstream>
#include <unistd.h> // gcc 4.7 needs it in order to have the definition of getcwd() (issue #49)

namespace avxsynth {

#ifdef MODULE_NAME
#undef MODULE_NAME
#endif 

#define MODULE_NAME core::plugins

const char* loadplugin_prefix = NULL;

/********************************************************************
* Native plugin support
********************************************************************/

enum { max_plugins=50 };

void FreeLibraries(void* loaded_plugins, IScriptEnvironment* env) {
  for (int i=0; i<max_plugins; ++i) {
    HMODULE plugin = ((HMODULE*)loaded_plugins)[i];
    if (plugin)
      dlclose(plugin);
    else
      break;
  }
  memset(loaded_plugins, 0, max_plugins*sizeof(HMODULE));
}

static bool IdentifiedLibAvxsynthDuplicate(const char* filename)
{
    bool bIdentified = false;
	HMODULE hmod = dlopen(filename, RTLD_LOCAL | RTLD_NOW | RTLD_FIRST);
	if(hmod) {
		if(dlsym(hmod, "add_built_in_functions_Audio_filters")) {
			dlclose(hmod);
            bIdentified = true;
		}
		dlclose(hmod);
	}
    return bIdentified;
}

static bool MyLoadLibrary(const char* filename, HMODULE* hmod, bool quiet, IScriptEnvironment* env) {
    
  if(IdentifiedLibAvxsynthDuplicate(filename))
  {
    AVXLOG_ERROR("plugin \"%s\" identified as duplicate of libavxsynth.so, prevented its loading", filename);
    return false;
  }
  
  HMODULE* loaded_plugins;
  try {
    loaded_plugins = (HMODULE*)env->GetVar("$Plugins$").AsString();
  }  // Tritical May 2005
  catch (IScriptEnvironment::NotFound) {
    HMODULE plugins[max_plugins]; // buffer to clone on stack

    memset(plugins, 0, max_plugins*sizeof(HMODULE));
    // Cheat and copy into SaveString buffer
    env->SetGlobalVar("$Plugins$", env->SaveString((const char*)plugins, max_plugins*sizeof(HMODULE)));
    try {
        loaded_plugins = (HMODULE*)env->GetVar("$Plugins$").AsString();
    }
    catch(IScriptEnvironment::NotFound) {
      if (!quiet)
        env->ThrowError("LoadPlugin: unable to get plugin list $Plugins$, loading \"%s\"", filename);
      return false;
    }
    // Register FreeLibraries(loaded_plugins) to be run at script close
    env->AtExit(FreeLibraries, loaded_plugins);
  }
  *hmod = dlopen(filename, RTLD_NOW | RTLD_GLOBAL);
  if (!*hmod)
  {
    // one more thing to try: implicit on Windows but not implicit on Linux is that the library loading
    // path may include current working directory
    char cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);
    std::string strTryingCurrentPath = std::string(cwd) + std::string("/") + std::string(filename);
    *hmod = dlopen(strTryingCurrentPath.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if(!*hmod)
    {
        AVXLOG_ERROR("Failed loading %s, error = %s", filename, dlerror());
        if (quiet)
            return false;
        else
            env->ThrowError("LoadPlugin: unable to load \"%s\"", filename);
    }
  }
  // see if we've loaded this already, and add it to the list if not
  for (int j=0; j<max_plugins; ++j) {
    if (loaded_plugins[j] == *hmod) {
      dlclose(*hmod);
      return false;
    }
    if (loaded_plugins[j] == 0) {
      char result[PATH_MAX] = "\0";
      char* t_string = _strrev(strdup(filename));
      int len = strlen(filename);
      int pos = len-strcspn(t_string, ".");
      int pos2 = len-strcspn(t_string, "/");
      free(t_string);  // Tritical May 2005
      strncat(result, filename+pos2, pos-pos2-1);
      if (loadplugin_prefix) free((void*)loadplugin_prefix);  // Tritical May 2005
      loadplugin_prefix = strdup(result);
      loaded_plugins[j] = *hmod;
      return true;
    }
  }
  dlclose(*hmod);  // Tritical Jan 2006
  if (!quiet)
    env->ThrowError("LoadPlugin: too many plugins loaded already (max. %d)", max_plugins);
  return false;
}


AVSValue LoadPlugin(AVSValue args, void* user_data, IScriptEnvironment* env) {
  bool quiet = (user_data != 0);
  args = args[0];
  const char* result=0;
  for (int i=0; i<args.ArraySize(); ++i) {
    HMODULE plugin;
    const char* plugin_name = args[i].AsString();
    if (MyLoadLibrary(plugin_name, &plugin, quiet, env)) {
      typedef const char* (__stdcall *AvisynthPluginInitFunc)(IScriptEnvironment* env);
      AvisynthPluginInitFunc AvisynthPluginInit = (AvisynthPluginInitFunc)dlsym(plugin, "AvisynthPluginInit2");
      if (!AvisynthPluginInit) {
        AvisynthPluginInit = (AvisynthPluginInitFunc)dlsym(plugin, "_AvisynthPluginInit2@4");
/*
        if (!AvisynthPluginInit) {  // Attempt C-plugin
          AvisynthPluginInit = (AvisynthPluginInitFunc)dlsym(plugin, "avisynth_c_plugin_init");
          if (AvisynthPluginInit) {
            dlclose(plugin);
            return env->Invoke("LoadCPlugin", args);
          }
        }
*/
        if (!AvisynthPluginInit) {  // Older version
          dlclose(plugin);
          if (quiet) {
            // remove the last handle from the list
            HMODULE* loaded_plugins = (HMODULE*)env->GetVar("$Plugins$").AsString();
            int j=0;
            while (loaded_plugins[j+1]) j++;
            loaded_plugins[j] = 0;
          } else {
            env->ThrowError("Plugin %s is not an AviSynth 2.5 plugin.",plugin_name);
          }
        } else {
          result = AvisynthPluginInit(env);
        }
      } else {
        result = AvisynthPluginInit(env);
      }
    }
  }
  if (loadplugin_prefix) free((void*)loadplugin_prefix);  // Tritical May 2005
  loadplugin_prefix = 0;
  return result ? AVSValue(result) : AVSValue();
}



AVSFunction Plugin_functions[] = {
  { "LoadPlugin", "s+", LoadPlugin, (void*)false },
  { 0 }
};

}; // namespace avxsynth
