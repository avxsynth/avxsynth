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

#ifndef __Script_H__
#define __Script_H__

#include "common/include/internal.h"
#include "expression.h"
#include "scriptparser.h"
#include <limits.h>

namespace avxsynth {
	
/********************************************************************
********************************************************************/

typedef char TCHAR;


class ScriptFunction 
/**
  * Executes a script
 **/
{
public:
  ScriptFunction(const PExpression& _body, const char** _param_names, int param_count);
  virtual ~ScriptFunction() 
    { delete[] param_names; }

  static AVSValue Execute(AVSValue args, void* user_data, IScriptEnvironment* env);
  static void Delete(void* self, IScriptEnvironment*);

private:
  const PExpression body;
  const char** param_names;
};







/****  Helper Classes  ****/

class CWDChanger 
/**
  * Class to change the current working directory
 **/
{  
public:
  CWDChanger(const char* new_cwd);  
  virtual ~CWDChanger(void);  

private:
  TCHAR old_working_directory[PATH_MAX];
  bool restore;
};


class DynamicCharBuffer 
/**
  * Simple dynamic character buffer
 **/
{
public:
  DynamicCharBuffer(int size) : p(new char[size]) {}
  operator char*() const { return p; }
  ~DynamicCharBuffer() { delete[] p; }

private:
  char* const p;
};




/****    Helper functions   ****/

AVSValue Assert(AVSValue args, void*, IScriptEnvironment* env);
AVSValue AssertEval(AVSValue args, void*, IScriptEnvironment* env);
AVSValue Eval(AVSValue args, void*, IScriptEnvironment* env);
AVSValue Apply(AVSValue args, void*, IScriptEnvironment* env) ;

AVSValue Import(AVSValue args, void*, IScriptEnvironment* env);

AVSValue SetMemoryMax(AVSValue args, void*, IScriptEnvironment* env);

AVSValue SetWorkingDir(AVSValue args, void*, IScriptEnvironment* env);

/*****   Entry/Factory Methods   ******/

AVSValue Muldiv(AVSValue args, void*, IScriptEnvironment* env);

AVSValue Floor(AVSValue args, void*, IScriptEnvironment* env);
AVSValue Ceil(AVSValue args, void*, IScriptEnvironment* env);
AVSValue Round(AVSValue args, void*, IScriptEnvironment* env);

AVSValue Sin(AVSValue args, void* user_data, IScriptEnvironment* env);
AVSValue Cos(AVSValue args, void* user_data, IScriptEnvironment* env);
AVSValue Pi(AVSValue args, void* user_data, IScriptEnvironment* env);
AVSValue Log(AVSValue args, void* user_data, IScriptEnvironment* env);
AVSValue Exp(AVSValue args, void* user_data, IScriptEnvironment* env);
AVSValue Pow(AVSValue args, void* user_data, IScriptEnvironment* env);
AVSValue Sqrt(AVSValue args, void* user_data, IScriptEnvironment* env);
AVSValue FAbs(AVSValue args, void* user_data, IScriptEnvironment* env);
AVSValue Abs(AVSValue args, void* user_data, IScriptEnvironment* env);


static inline const VideoInfo& VI(const AVSValue& arg);

AVSValue Width(AVSValue args, void*, IScriptEnvironment* env);
AVSValue Height(AVSValue args, void*, IScriptEnvironment* env);
AVSValue FrameCount(AVSValue args, void*, IScriptEnvironment* env);
AVSValue FrameRate(AVSValue args, void*, IScriptEnvironment* env);
AVSValue FrameRateNumerator(AVSValue args, void*, IScriptEnvironment* env);
AVSValue FrameRateDenominator(AVSValue args, void*, IScriptEnvironment* env);
AVSValue AudioRate(AVSValue args, void*, IScriptEnvironment* env);
AVSValue AudioLength(AVSValue args, void*, IScriptEnvironment* env);
AVSValue AudioLengthF(AVSValue args, void*, IScriptEnvironment* env);
AVSValue AudioChannels(AVSValue args, void*, IScriptEnvironment* env);
AVSValue AudioBits(AVSValue args, void*, IScriptEnvironment* env);
AVSValue IsAudioFloat(AVSValue args, void*, IScriptEnvironment* env);
AVSValue IsAudioInt(AVSValue args, void*, IScriptEnvironment* env);

AVSValue IsRGB(AVSValue args, void*, IScriptEnvironment* env);
AVSValue IsYV12(AVSValue args, void*, IScriptEnvironment* env);
AVSValue IsYUY2(AVSValue args, void*, IScriptEnvironment* env);
AVSValue IsYUV(AVSValue args, void*, IScriptEnvironment* env);
AVSValue IsRGB24(AVSValue args, void*, IScriptEnvironment* env);
AVSValue IsRGB32(AVSValue args, void*, IScriptEnvironment* env);
AVSValue IsPlanar(AVSValue args, void*, IScriptEnvironment* env);
AVSValue IsInterleaved(AVSValue args, void*, IScriptEnvironment* env);
AVSValue IsFieldBased(AVSValue args, void*, IScriptEnvironment* env);
AVSValue IsFrameBased(AVSValue args, void*, IScriptEnvironment* env);
AVSValue GetParity(AVSValue args, void*, IScriptEnvironment* env);
AVSValue String(AVSValue args, void*, IScriptEnvironment* env);

AVSValue IsBool(AVSValue args, void*, IScriptEnvironment* env);
AVSValue IsInt(AVSValue args, void*, IScriptEnvironment* env);
AVSValue IsFloat(AVSValue args, void*, IScriptEnvironment* env);
AVSValue IsString(AVSValue args, void*, IScriptEnvironment* env);
AVSValue IsClip(AVSValue args, void*, IScriptEnvironment* env);
AVSValue Defined(AVSValue args, void*, IScriptEnvironment* env);

AVSValue Default(AVSValue args, void*, IScriptEnvironment* env);

AVSValue VersionNumber(AVSValue args, void*, IScriptEnvironment* env);
AVSValue VersionString(AVSValue args, void*, IScriptEnvironment* env); 

AVSValue Int(AVSValue args, void*, IScriptEnvironment* env); 
AVSValue Frac(AVSValue args, void*, IScriptEnvironment* env); 
AVSValue Float(AVSValue args, void*,IScriptEnvironment* env); 
AVSValue Value(AVSValue args, void*, IScriptEnvironment* env);
AVSValue HexValue(AVSValue args, void*, IScriptEnvironment* env);

AVSValue Sign(AVSValue args, void*, IScriptEnvironment* env);

AVSValue UCase(AVSValue args, void*, IScriptEnvironment* env);
AVSValue LCase(AVSValue args, void*, IScriptEnvironment* env);
AVSValue StrLen(AVSValue args, void*, IScriptEnvironment* env);
AVSValue RevStr(AVSValue args, void*, IScriptEnvironment* env);
AVSValue LeftStr(AVSValue args, void*, IScriptEnvironment* env);
AVSValue MidStr(AVSValue args, void*, IScriptEnvironment* env);
AVSValue RightStr(AVSValue args, void*, IScriptEnvironment* env);
AVSValue FindStr(AVSValue args, void*, IScriptEnvironment* env);

AVSValue Rand(AVSValue args, void* user_data, IScriptEnvironment* env);

AVSValue Select(AVSValue args, void*, IScriptEnvironment* env);

AVSValue NOP(AVSValue args, void*, IScriptEnvironment* env);

AVSValue Exist(AVSValue args, void*, IScriptEnvironment* env);

// WE ->
AVSValue AVSChr(AVSValue args, void*, IScriptEnvironment* env);
AVSValue AVSTime(AVSValue args, void*, IScriptEnvironment* env);
AVSValue Spline(AVSValue args, void*, IScriptEnvironment* env);
// WE <-

AVSValue Int(AVSValue args, void*, IScriptEnvironment* env); 
AVSValue Frac(AVSValue args, void*, IScriptEnvironment* env); 
AVSValue Float(AVSValue args, void*,IScriptEnvironment* env); 
AVSValue Value(AVSValue args, void*, IScriptEnvironment* env);
AVSValue HexValue(AVSValue args, void*, IScriptEnvironment* env);

AVSValue HasVideo(AVSValue args, void*, IScriptEnvironment* env);
AVSValue HasAudio(AVSValue args, void*, IScriptEnvironment* env);

AVSValue AvsMin(AVSValue args, void*, IScriptEnvironment* env);
AVSValue AvsMax(AVSValue args, void*, IScriptEnvironment* env);

}; // namespace avxsynth
#endif  // __Script_H__
