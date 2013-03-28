
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


#include "internal.h"

namespace avxsynth {
	
class ConditionalFilter : public GenericVideoFilter
/**
  * Conditional 
 **/
{
  enum Eval {
    NONE = 0,
    EQUALS = 1,
    GREATERTHAN = 2,
    LESSTHAN = 4
  };

public:
  ConditionalFilter(PClip _child, PClip _source1, PClip _source2, AVSValue  _condition1, AVSValue  _evaluator, AVSValue  _condition2, bool _show, IScriptEnvironment* env);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env);
  static AVSValue __cdecl Create(AVSValue args, void* user_data, IScriptEnvironment* env);

private:
  PClip source1;
  PClip source2;
  Eval evaluator;
  AVSValue eval1;
  AVSValue eval2;
  bool show;
};

class ScriptClip : public GenericVideoFilter
{
public:
  ScriptClip(PClip _child, AVSValue  _script, bool _show, bool _only_eval, bool _eval_after_frame, IScriptEnvironment* env);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  static AVSValue __cdecl Create(AVSValue args, void* user_data, IScriptEnvironment* env);
  static AVSValue __cdecl Create_eval(AVSValue args, void* user_data, IScriptEnvironment* env);

private:
  AVSValue script;
  bool show;
  bool only_eval;
  bool eval_after;
};

}; // namespace avxsynth
