
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

#include "conditional.h"
#include "../../core/parser/scriptparser.h"
#include <text-overlay.h>
#include "conditional_reader.h"

namespace avxsynth {
	
AVSFunction Conditional_filters[] = {
  {  "ConditionalFilter","cccsss[show]b", ConditionalFilter::Create },
  {  "ScriptClip", "cs[show]b[after_frame]b", ScriptClip::Create },
  {  "ConditionalReader", "css[show]b", ConditionalReader::Create },
  {  "FrameEvaluate", "cs[show]b[after_frame]b", ScriptClip::Create_eval },
  {  "WriteFile",   "c[filename]ss+[append]b[flush]b", Write::Create },
  {  "WriteFileIf", "c[filename]ss+[append]b[flush]b", Write::Create_If },
  {  "WriteFileStart", "c[filename]ss+[append]b", Write::Create_Start },
  {  "WriteFileEnd", "c[filename]ss+[append]b", Write::Create_End },
  { 0 }
};

#define W_DIVISOR 5  // Width divisor for onscreen messages

/********************************
 * Conditional filter
 *
 * Returns each one frame from two sources,
 * based on an evaluator.
 ********************************/

ConditionalFilter::ConditionalFilter(PClip _child, PClip _source1, PClip _source2, AVSValue  _condition1, AVSValue  _evaluator, AVSValue  _condition2, bool _show, IScriptEnvironment* env) :
  GenericVideoFilter(_child), source1(_source1), source2(_source2),
  eval1(_condition1), eval2(_condition2), show(_show) {
    
    evaluator = NONE;

    if (strcasecmp(_evaluator.AsString(), "equals") == 0 || strcasecmp(_evaluator.AsString(), "=") == 0 || strcasecmp(_evaluator.AsString(), "==") == 0)
      evaluator = EQUALS;
    if (strcasecmp(_evaluator.AsString(), "greaterthan") == 0 || strcasecmp(_evaluator.AsString(), ">") == 0)
      evaluator = GREATERTHAN;
    if (strcasecmp(_evaluator.AsString(), "lessthan") == 0 || strcasecmp(_evaluator.AsString(), "<") == 0)
      evaluator = LESSTHAN;

    if (evaluator == NONE)
      env->ThrowError("ConditionalFilter: Evaluator could not be recognized!");

    VideoInfo vi1 = source1->GetVideoInfo();
    VideoInfo vi2 = source2->GetVideoInfo();

    if (vi1.height != vi2.height)
      env->ThrowError("ConditionalFilter: The two sources must have the same height!");
    if (vi1.width != vi2.width)
      env->ThrowError("ConditionalFilter: The two sources must have the same width!");
    if (!vi1.IsSameColorspace(vi2))
      env->ThrowError("ConditionalFilter: The two sources must be the same colorspace!");

    vi.height = vi1.height;
    vi.width = vi1.width;
    vi.pixel_type = vi1.pixel_type;
    vi.num_frames = std::max(vi1.num_frames,vi2.num_frames);
    vi.num_audio_samples = vi1.num_audio_samples;
    vi.audio_samples_per_second = vi1.audio_samples_per_second;
    vi.image_type = vi1.image_type;
    vi.fps_denominator = vi1.fps_denominator;
    vi.fps_numerator = vi1.fps_numerator;
    vi.nchannels = vi1.nchannels;
    vi.sample_type = vi1.sample_type;
  }

const char* t_TRUE="TRUE"; 
const char* t_FALSE="FALSE";


PVideoFrame __stdcall ConditionalFilter::GetFrame(int n, IScriptEnvironment* env) {

  VideoInfo vi1 = source1->GetVideoInfo();
  VideoInfo vi2 = source2->GetVideoInfo();

  AVSValue prev_last = env->GetVar("last");  // Store previous last
  AVSValue prev_current_frame = GetVar(env, "current_frame");  // Store previous current_frame

  env->SetVar("last",(AVSValue)child);       // Set implicit last
  env->SetVar("current_frame",(AVSValue)n);  // Set frame to be tested by the conditional filters.

  AVSValue e1_result;
  AVSValue e2_result;
  try {
    ScriptParser parser(env, eval1.AsString(), "[Conditional Filter, Expresion 1]");
    PExpression exp = parser.Parse();
    e1_result = exp->Evaluate(env);

    ScriptParser parser2(env, eval2.AsString(), "[Conditional Filter, Expression 2]");
    exp = parser2.Parse();
    e2_result = exp->Evaluate(env);
  } catch (AvisynthError error) {    
    const char* error_msg = error.msg;  

    PVideoFrame dst = source1->GetFrame(n,env);
    env->MakeWritable(&dst);
    ApplyMessage(&dst, vi1, error_msg, vi.width/W_DIVISOR, 0xa0a0a0,0,0 , env );
    env->SetVar("last",prev_last);       // Restore implicit last
    env->SetVar("current_frame",prev_current_frame);       // Restore current_frame
    return dst;
  }

  env->SetVar("last",prev_last);       // Restore implicit last
  env->SetVar("current_frame",prev_current_frame);       // Restore current_frame

  int test_int=false;

  int e1 = 0;
  int e2 = 0;
  float f1 = 0.0f;
  float f2 = 0.0f;
  try {
    if (e1_result.IsInt() || e1_result.IsBool()) {
      test_int = true;
      e1 = e1_result.IsInt() ? e1_result.AsInt() : e1_result.AsBool();
      if (!(e2_result.IsInt() || e2_result.IsBool()))
        env->ThrowError("Conditional filter: Second expression did not return an integer or bool, as first expression.");
      e2 = e2_result.IsInt() ? e2_result.AsInt() : e2_result.AsBool();

    } else if (e1_result.IsFloat()) {
      f1 = e1_result.AsFloat();
      if (!e2_result.IsFloat()) 
        env->ThrowError("Conditional filter: Second expression did not return a float or an integer, as first expression.");
      f2 = e2_result.AsFloat();
    } else {
      env->ThrowError("ConditionalFilter: First condition did not return an int, bool or float!");
    }
  } catch (AvisynthError error) {    
    const char* error_msg = error.msg;  

    PVideoFrame dst = source1->GetFrame(n,env);
    env->MakeWritable(&dst);
    ApplyMessage(&dst, vi1, error_msg, vi.width/W_DIVISOR, 0xa0a0a0,0,0 , env );
    return dst;
  }


  bool state = false;

  if (test_int) {
    if (evaluator&EQUALS) 
      if (e1 == e2) state = true;

    if (evaluator&GREATERTHAN) 
      if (e1 > e2) state = true;

    if (evaluator&LESSTHAN) 
      if (e1 < e2) state = true;
  } else {  // Float compare
    if (evaluator&EQUALS) 
      if (fabs(f1-f2)<0.000001f) state = true;   // Exact equal will sometimes be rounded to wrong values.

    if (evaluator&GREATERTHAN) 
      if (f1 > f2) state = true;

    if (evaluator&LESSTHAN) 
      if (f1 < f2) state = true;    
  }

  if (show) {
      char text[400];
      if (test_int) {
        sprintf(text,
          "Left side Conditional Result:%i\n"
          "Right side Conditional Result:%i\n"
          "Evaluate result: %s\n",
          e1, e2, (state) ? t_TRUE : t_FALSE
        );
      } else {
        sprintf(text,
          "Left side Conditional Result:%7.4f\n"
          "Right side Conditional Result:%7.4f\n"
          "Evaluate result: %s\n", 
          f1, f2, (state) ? t_TRUE : t_FALSE
        );
      }

      PVideoFrame dst = (state) ? source1->GetFrame(std::min(vi1.num_frames-1,n),env) : source2->GetFrame(std::min(vi2.num_frames-1,n),env);
      env->MakeWritable(&dst);
      ApplyMessage(&dst, vi, text, vi.width/4, 0xa0a0a0,0,0 , env );

    return dst;
  }

  if (state) 
    return source1->GetFrame(std::min(vi1.num_frames-1,n),env);
  
  return source2->GetFrame(std::min(vi1.num_frames-1,n),env);
}

void __stdcall ConditionalFilter::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) {
  source1->GetAudio(buf, start, count, env);
}


AVSValue __cdecl ConditionalFilter::Create(AVSValue args, void* user_data, IScriptEnvironment* env)
{
  return new ConditionalFilter(args[0].AsClip(), args[1].AsClip(), args[2].AsClip(), args[3], args[4], args[5], args[6].AsBool(false), env);
}


/**************************
 * ScriptClip.
 *
 * Returns the value of a script evaluated at each frame.
 *
 * Implicit last, and current frame is set on each frame.
 **************************/

ScriptClip::ScriptClip(PClip _child, AVSValue  _script, bool _show, bool _only_eval, bool _eval_after_frame, IScriptEnvironment* env) :
  GenericVideoFilter(_child), script(_script), show(_show), only_eval(_only_eval), eval_after(_eval_after_frame) {

  }

PVideoFrame __stdcall ScriptClip::GetFrame(int n, IScriptEnvironment* env) {
  AVSValue prev_last = env->GetVar("last");  // Store previous last
  AVSValue prev_current_frame = GetVar(env, "current_frame");  // Store previous current_frame

  env->SetVar("last",(AVSValue)child);       // Set explicit last
  env->SetVar("current_frame",(AVSValue)n);  // Set frame to be tested by the conditional filters.

  if (show) {
    PVideoFrame dst = child->GetFrame(n,env);
    env->MakeWritable(&dst);
    ApplyMessage(&dst, vi, script.AsString(), vi.width/6, 0xa0a0a0,0,0 , env );
    env->SetVar("last",prev_last);       // Restore implicit last
    env->SetVar("current_frame",prev_current_frame);       // Restore current_frame
    return dst;
  }

  AVSValue result;
  PVideoFrame eval_return;   // Frame to be returned if script should be evaluated AFTER frame has been fetched. Otherwise not used.

  if (eval_after) eval_return = child->GetFrame(n,env);

  try {
    ScriptParser parser(env, script.AsString(), "[ScriptClip]");
    PExpression exp = parser.Parse();
    result = exp->Evaluate(env);
  } catch (AvisynthError error) {    
    const char* error_msg = error.msg;  

    PVideoFrame dst = child->GetFrame(n,env);
    env->MakeWritable(&dst);
    ApplyMessage(&dst, vi, error_msg, vi.width/W_DIVISOR, 0xa0a0a0,0,0 , env );
    env->SetVar("last",prev_last);       // Restore implicit last
    env->SetVar("current_frame",prev_current_frame);       // Restore current_frame
    return dst;
  }

  env->SetVar("last",prev_last);       // Restore implicit last
  env->SetVar("current_frame",prev_current_frame);       // Restore current_frame

  if (eval_after && only_eval) return eval_return;
  if (only_eval) return child->GetFrame(n,env);
  
  const char* error = NULL;
  VideoInfo vi2;
  if (!result.IsClip()) {
    error = "ScriptClip: Function did not return a video clip!";
  } else {
    vi2 = result.AsClip()->GetVideoInfo();
    if (!vi.IsSameColorspace(vi2)) { 
      error = "ScriptClip: Function did not return a video clip of the same colorspace as the source clip!";
    } else if (vi2.width != vi.width) {
      error = "ScriptClip: Function did not return a video clip with the same width as the source clip!";
    } else if (vi2.height != vi.height) {
      error = "ScriptClip: Function did not return a video clip with the same height as the source clip!";
    }
  }

  if (error != NULL) {
    PVideoFrame dst = child->GetFrame(n,env);
    env->MakeWritable(&dst);
    ApplyMessage(&dst, vi, error, vi.width/W_DIVISOR, 0xa0a0a0,0,0 , env );
    return dst;
  }

  n = std::min(n,vi2.num_frames-1);  // We ignore it if the new clip is not as long as the current one. This can allow the resulting clip to be one frame.

  return result.AsClip()->GetFrame(n,env);
}


AVSValue __cdecl ScriptClip::Create(AVSValue args, void* user_data, IScriptEnvironment* env)
{
  return new ScriptClip(args[0].AsClip(), args[1], args[2].AsBool(false),false, args[3].AsBool(false), env);
}


AVSValue __cdecl ScriptClip::Create_eval(AVSValue args, void* user_data, IScriptEnvironment* env)
{
  return new ScriptClip(args[0].AsClip(), args[1], args[2].AsBool(false),true, args[3].AsBool(false), env);
}

}; // namespace avxsynth
