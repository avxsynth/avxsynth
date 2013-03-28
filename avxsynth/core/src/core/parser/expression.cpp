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

#include "windowsPorts/WinNTLinux.h"
#include "windowsPorts/excptLinux.h"
#include "expression.h"
#include "core/cache.h"
#include "avxlog.h"

namespace avxsynth {

#ifdef MODULE_NAME
#undef MODULE_NAME
#endif

#define MODULE_NAME core::expression


/**** Objects ****/

AVSValue ExpSequence::Evaluate(IScriptEnvironment* env) 
{
    AVSValue last = a->Evaluate(env);
    if (last.IsClip()) env->SetVar("last", last);
    return b->Evaluate(env);
}

/* First cut for breaking out system exceptions from the evil and most
 * unhelpful "Evaluate: Unrecognized exception!".
 *
 * This initial version just decodes the exception code, latter if one
 * is so inclined the info structure could be pulled apart and the
 * state of the machine presented. So far just knowing "Integer Divide
 * by Zero" was happening has been a real boon.
 */
static const char * const StringSystemError(const unsigned code)
{
  switch (code) {
  case STATUS_GUARD_PAGE_VIOLATION:      // 0x80000001
    return "Evaluate: System exception - Guard Page Violation";
  case STATUS_DATATYPE_MISALIGNMENT:     // 0x80000002
    return "Evaluate: System exception - Datatype Misalignment";
  case STATUS_BREAKPOINT:                // 0x80000003
    return "Evaluate: System exception - Breakpoint";
  case STATUS_SINGLE_STEP:               // 0x80000004
    return "Evaluate: System exception - Single Step";
  default:
    break;
  }

  switch (code) {
  case STATUS_ACCESS_VIOLATION:          // 0xc0000005
    return "Evaluate: System exception - Access Violation";
  case STATUS_IN_PAGE_ERROR:             // 0xc0000006
    return "Evaluate: System exception - In Page Error";
  case STATUS_INVALID_HANDLE:            // 0xc0000008
    return "Evaluate: System exception - Invalid Handle";
  case STATUS_NO_MEMORY:                 // 0xc0000017
    return "Evaluate: System exception - No Memory";
  case STATUS_ILLEGAL_INSTRUCTION:       // 0xc000001d
    return "Evaluate: System exception - Illegal Instruction";
  case STATUS_NONCONTINUABLE_EXCEPTION:  // 0xc0000025
    return "Evaluate: System exception - Noncontinuable Exception";
  case STATUS_INVALID_DISPOSITION:       // 0xc0000026
    return "Evaluate: System exception - Invalid Disposition";
  case STATUS_ARRAY_BOUNDS_EXCEEDED:     // 0xc000008c
    return "Evaluate: System exception - Array Bounds Exceeded";
  case STATUS_FLOAT_DENORMAL_OPERAND:    // 0xc000008d
    return "Evaluate: System exception - Float Denormal Operand";
  case STATUS_FLOAT_DIVIDE_BY_ZERO:      // 0xc000008e
    return "Evaluate: System exception - Float Divide by Zero";
  case STATUS_FLOAT_INEXACT_RESULT:      // 0xc000008f
    return "Evaluate: System exception - Float Inexact Result";
  case STATUS_FLOAT_INVALID_OPERATION:   // 0xc0000090
    return "Evaluate: System exception - Float Invalid Operation";
  case STATUS_FLOAT_OVERFLOW:            // 0xc0000091
    return "Evaluate: System exception - Float Overflow";
  case STATUS_FLOAT_STACK_CHECK:         // 0xc0000092
    return "Evaluate: System exception - Float Stack Check";
  case STATUS_FLOAT_UNDERFLOW:           // 0xc0000093
    return "Evaluate: System exception - Float Underflow";
  case STATUS_INTEGER_DIVIDE_BY_ZERO:    // 0xc0000094
    return "Evaluate: System exception - Integer Divide by Zero";
  case STATUS_INTEGER_OVERFLOW:          // 0xc0000095
    return "Evaluate: System exception - Integer Overflow";
  case STATUS_PRIVILEGED_INSTRUCTION:    // 0xc0000096
    return "Evaluate: System exception - Privileged Instruction";
  case STATUS_STACK_OVERFLOW:            // 0xc00000fd
    return "Evaluate: System exception - Stack Overflow";
  default:
    break;
  }

  return 0;
}


/* Damn! This is trickey, exp->Evaluate returns an AVSValue object, so the
 * stupid compiler builds a local one on the stack, calls the constructor,
 * calls Evaluate passing a pointer to the local AVSValue object. Upon return
 * it then copies the local AVSValue to the passed in alias and calls the
 * destructor. Of course there has to be an implicit try/catch around the
 * whole lot just in case so the destructor is guaranteed to be called.
 *
 * All proper C++, yes but it would be just as valid to directly pass the
 * av alias to Evaluate in it's returned object argument, and I wouldn't
 * need all the trickey calls within calls to do this simple job.
 */
void ExpExceptionTranslator::ChainEval(AVSValue &av, IScriptEnvironment* env) 
{
  av = exp->Evaluate(env);
}


#if 0
/* Damn! I can't call Evaluate directly from here because it returns an object
 * I have to call an interlude and pass an alias to the return value through.
 */
void ExpExceptionTranslator::TrapEval(AVSValue &av, unsigned &excode, IScriptEnvironment* env) 
{
// XPsp2 bug with this
  __try {
    ChainEval(av, env);
  }
  __except (excode = _exception_code(), EXCEPTION_CONTINUE_SEARCH ) { }
}

#else
  
/* Damn! XPsp2 barfs at my use of SEH. So do my own exception handling routine.
 * Simple handler snaffles the exception code and stashes it where my extended
 * EXCEPTION_REGISTRATION record retarg pointer point, then just continue the
 * search. 
 */
EXCEPTION_DISPOSITION __cdecl _Exp_except_handler(struct _EXCEPTION_RECORD *ExceptionRecord, void * EstablisherFrame,
												  struct _CONTEXT *ContextRecord, void * DispatcherContext)
{
  struct Est_Frame {  // My extended EXCEPTION_REGISTRATION record
	void	  *prev;
	void	  *handler;
	unsigned  *retarg;	  // pointer where to stash exception code
  };

  if (ExceptionRecord->ExceptionFlags == 0)	  // First pass?
	*(((struct Est_Frame *)EstablisherFrame)->retarg) = ExceptionRecord->ExceptionCode;

  return ExceptionContinueSearch;
}


/* Damn! XPsp2 barfs at my use of SEH, so do my own exception handling
 * Having to do this is pure filth!
 */
void ExpExceptionTranslator::TrapEval(AVSValue &av, unsigned &excode, IScriptEnvironment* env) 
{
#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE 
  DWORD handler = (DWORD)_Exp_except_handler;
  __asm { // Build EXCEPTION_REGISTRATION record:
  push	excode		// Address of return argument
  push	handler	    // Address of handler function
  push	FS:[0]		// Address of previous handler
  mov	FS:[0],esp	// Install new EXCEPTION_REGISTRATION
  }
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
  ChainEval(av, env);

#ifdef ENABLE_INLINE_ASSEMBLY_MMX_SSE 
  __asm { // Remove our EXCEPTION_REGISTRATION record
  mov	eax,[esp]	// Get pointer to previous record
  mov	FS:[0], eax	// Install previous record
  add	esp, 12		// Clean our EXCEPTION_REGISTRATION off stack
  }
#endif // ENABLE_INLINE_ASSEMBLY_MMX_SSE
}
#endif


/* Damn! You can't mix C++ exception handling and SEH in the one routine.
 * Call an interlude. And this is all because C++ doesn't provide any way
 * to expand system exceptions into there true cause or identity.
 */

AVSValue ExpExceptionTranslator::Evaluate(IScriptEnvironment* env) 
{
  unsigned excode=0;
  try {
    AVSValue av;
    TrapEval(av, excode, env);
    return av;
  }
  catch (IScriptEnvironment::NotFound) {
    throw;
  }
  catch (AvisynthError) {
    throw;
  }
  catch (...) {
	if (excode == 0xE06D7363) // C++ Exception, 0xE0000000 | "\0msc"
      env->ThrowError("Evaluate: Unhandled C++ exception!");

	if (excode != 0) {
	  const char * const extext = StringSystemError(excode);
	  if (extext)
		env->ThrowError(extext);
	  else
		env->ThrowError("Evaluate: System exception - 0x%x", excode);
	}
    env->ThrowError("Evaluate: Unrecognized exception!");
  }
  return 0;
}



AVSValue ExpTryCatch::Evaluate(IScriptEnvironment* env) 
{
  AVSValue result;
  try {
    return ExpExceptionTranslator::Evaluate(env);
  }
  catch (AvisynthError ae) {
    env->SetVar(id, ae.msg);
    return catch_block->Evaluate(env);
  }
}


AVSValue ExpLine::Evaluate(IScriptEnvironment* env) 
{
  try {
    return ExpExceptionTranslator::Evaluate(env);
  }
  catch (AvisynthError ae) {
    env->ThrowError("%s\n(%s, line %d)", ae.msg, filename, line);
  }
  return 0;
}


AVSValue ExpConditional::Evaluate(IScriptEnvironment* env) 
{
  AVSValue cond = If->Evaluate(env);
  if (!cond.IsBool())
    env->ThrowError("Evaluate: left of `?' must be boolean (true/false)");
  return (cond.AsBool() ? Then : Else)->Evaluate(env);
}





/**** Operators ****/

AVSValue ExpOr::Evaluate(IScriptEnvironment* env) 
{
  AVSValue x = a->Evaluate(env);
  if (!x.IsBool())
    env->ThrowError("Evaluate: left operand of || must be boolean (true/false)");
  if (x.AsBool())
    return x;
  AVSValue y = b->Evaluate(env);
  if (!y.IsBool())
    env->ThrowError("Evaluate: right operand of || must be boolean (true/false)");
  return y;
}


AVSValue ExpAnd::Evaluate(IScriptEnvironment* env) 
{
  AVSValue x = a->Evaluate(env);
  if (!x.IsBool())
    env->ThrowError("Evaluate: left operand of && must be boolean (true/false)");
  if (!x.AsBool())
    return x;
  AVSValue y = b->Evaluate(env);
  if (!y.IsBool())
    env->ThrowError("Evaluate: right operand of && must be boolean (true/false)");
  return y;
}


AVSValue ExpEqual::Evaluate(IScriptEnvironment* env) 
{
  AVSValue x = a->Evaluate(env);
  AVSValue y = b->Evaluate(env);
  if (x.IsBool() && y.IsBool()) {
    return x.AsBool() == y.AsBool();
  }
  else if (x.IsInt() && y.IsInt()) {
    return x.AsInt() == y.AsInt();
  }
  else if (x.IsFloat() && y.IsFloat()) {
    return x.AsFloat() == y.AsFloat();
  }
  else if (x.IsClip() && y.IsClip()) {
    return x.AsClip() == y.AsClip();
  }
  else if (x.IsString() && y.IsString()) {
    return !strcasecmp(x.AsString(), y.AsString());
  }
  else {
    env->ThrowError("Evaluate: operands of `==' and `!=' must be comparable");
    return 0;
  }
}


AVSValue ExpLess::Evaluate(IScriptEnvironment* env)
{
  AVSValue x = a->Evaluate(env);
  AVSValue y = b->Evaluate(env);
  if (x.IsInt() && y.IsInt()) {
    return x.AsInt() < y.AsInt();
  }
  else if (x.IsFloat() && y.IsFloat()) {
    return x.AsFloat() < y.AsFloat();
  }
  else if (x.IsString() && y.IsString()) {
    return strcasecmp(x.AsString(),y.AsString()) < 0 ? true : false;
  }
  else {
    env->ThrowError("Evaluate: operands of `<' and friends must be string or numeric");
    return 0;
  }
}

AVSValue ExpPlus::Evaluate(IScriptEnvironment* env)
{
  AVSValue x = a->Evaluate(env);
  AVSValue y = b->Evaluate(env);
  if (x.IsClip() && y.IsClip())
    return new_Splice(x.AsClip(), y.AsClip(), false, env);    // UnalignedSplice
  else if (x.IsInt() && y.IsInt())
    return x.AsInt() + y.AsInt();
  else if (x.IsFloat() && y.IsFloat())
    return x.AsFloat() + y.AsFloat();
  else if (x.IsString() && y.IsString())
    return env->Sprintf("%s%s", x.AsString(), y.AsString());
  else {
    env->ThrowError("Evaluate: operands of `+' must both be numbers, strings, or clips");
    return 0;
  }
}


AVSValue ExpDoublePlus::Evaluate(IScriptEnvironment* env)
{
  AVSValue x = a->Evaluate(env);
  AVSValue y = b->Evaluate(env);
  if (x.IsClip() && y.IsClip())
    return new_Splice(x.AsClip(), y.AsClip(), true, env);    // AlignedSplice
  else {
    env->ThrowError("Evaluate: operands of `++' must be clips");
    return 0;
  }
}


AVSValue ExpMinus::Evaluate(IScriptEnvironment* env) 
{
  AVSValue x = a->Evaluate(env);
  AVSValue y = b->Evaluate(env);
  if (x.IsInt() && y.IsInt())
    return x.AsInt() - y.AsInt();
  else if (x.IsFloat() && y.IsFloat())
    return x.AsFloat() - y.AsFloat();
  else {
    env->ThrowError("Evaluate: operands of `-' must be numeric");
    return 0;
  }
}


AVSValue ExpMult::Evaluate(IScriptEnvironment* env) 
{
  AVSValue x = a->Evaluate(env);
  AVSValue y = b->Evaluate(env);
  if (x.IsInt() && y.IsInt())
    return x.AsInt() * y.AsInt();
  else if (x.IsFloat() && y.IsFloat())
    return x.AsFloat() * y.AsFloat();
  else {
    env->ThrowError("Evaluate: operands of `*' must be numeric");
    return 0;
  }
}


AVSValue ExpDiv::Evaluate(IScriptEnvironment* env)
{
  AVSValue x = a->Evaluate(env);
  AVSValue y = b->Evaluate(env);
  if (x.IsInt() && y.IsInt()) {
    if (y.AsInt() == 0)
      env->ThrowError("Evaluate: division by zero");
    return x.AsInt() / y.AsInt();
  }
  else if (x.IsFloat() && y.IsFloat())
    return x.AsFloat() / y.AsFloat();
  else {
    env->ThrowError("Evaluate: operands of `/' must be numeric");
    return 0;
  }
}


AVSValue ExpMod::Evaluate(IScriptEnvironment* env) 
{
  AVSValue x = a->Evaluate(env);
  AVSValue y = b->Evaluate(env);
  if (x.IsInt() && y.IsInt()) {
    if (y.AsInt() == 0)
      env->ThrowError("Evaluate: division by zero");
    return x.AsInt() % y.AsInt();
  }
  else {
    env->ThrowError("Evaluate: operands of `%%' must be integers");
    return 0;
  }
}


AVSValue ExpNegate::Evaluate(IScriptEnvironment* env)
{
  AVSValue x = e->Evaluate(env);
  if (x.IsInt())
    return -x.AsInt();
  else if (x.IsFloat())
    return -x.AsFloat();
  else {
    env->ThrowError("Evaluate: unary minus can only by used with numbers");
    return 0;
  }
}


AVSValue ExpNot::Evaluate(IScriptEnvironment* env)
{
  AVSValue x = e->Evaluate(env);
  if (x.IsBool())
    return !x.AsBool();
  else {
    env->ThrowError("Evaluate: operand of `!' must be boolean (true/false)");
    return 0;
  }
}


AVSValue ExpVariableReference::Evaluate(IScriptEnvironment* env) 
{
  AVSValue result;
  try {
    // first look for a genuine variable
	// No Tritical don't add a cache to this one, it's a Var
    return env->GetVar(name);
  }
  catch (IScriptEnvironment::NotFound) {
    try {
      // next look for a single-arg function taking implicit "last"
      result = env->Invoke(name, env->GetVar("last"));
    }
    catch (IScriptEnvironment::NotFound) {
      try {
        // finally look for an argless function
        result = env->Invoke(name, AVSValue(0,0));
      }
      catch (IScriptEnvironment::NotFound) {
        env->ThrowError("I don't know what \"%s\" means", name);
        return 0;
      }
    }
  }
  // Add cache to Bracketless call of argless function
  if (result.IsClip()) { // Tritical Jan 2006
    return Cache::Create_Cache(result, 0, env);
  }
  return result;
}


AVSValue ExpAssignment::Evaluate(IScriptEnvironment* env)
{
  env->SetVar(lhs, rhs->Evaluate(env));
  return AVSValue();
}


AVSValue ExpGlobalAssignment::Evaluate(IScriptEnvironment* env) 
{
  env->SetGlobalVar(lhs, rhs->Evaluate(env));
  return AVSValue();
}


ExpFunctionCall::ExpFunctionCall( const char* _name, PExpression* _arg_exprs,
                   const char** _arg_expr_names, int _arg_expr_count, bool _oop_notation )
  : name(_name), arg_expr_count(_arg_expr_count), oop_notation(_oop_notation)
{
  arg_exprs = new PExpression[arg_expr_count];
  // arg_expr_names has an extra elt at the beginning, for implicit "last"
  arg_expr_names = new const char*[arg_expr_count+1];
  arg_expr_names[0] = 0;
  for (int i=0; i<arg_expr_count; ++i) {
    arg_exprs[i] = _arg_exprs[i];
    arg_expr_names[i+1] = _arg_expr_names[i];
  }
}


AVSValue ExpFunctionCall::Call(IScriptEnvironment* env) 
{
  AVSValue result;
  AVSValue *args = new AVSValue[arg_expr_count+1];
  
  AVXLOG_INFO("Call: %s", name);
  
  try {
    for (int a=0; a<arg_expr_count; ++a)
      args[a+1] = arg_exprs[a]->Evaluate(env);
  }
  catch (...)
  {
    delete[] args;
    throw;
  }
  // first try without implicit "last"
  try {
    result = env->Invoke(name, AVSValue(args+1, arg_expr_count), arg_expr_names+1);
    delete[] args;
    return result;
  }
  catch (IScriptEnvironment::NotFound) {
    // if that fails, try with implicit "last" (except when OOP notation was used)
    if (!oop_notation) {
      try {
        args[0] = env->GetVar("last");
        result = env->Invoke(name, AVSValue(args, arg_expr_count+1), arg_expr_names);
        delete[] args;
        return result;
      }
      catch (IScriptEnvironment::NotFound) { /* see below */ }
      catch (...)
      {
        delete[] args;
        throw;
      }
    }
  }
  catch (...)
  {
    delete[] args;
    throw;
  }
  delete[] args;
  env->ThrowError(env->FunctionExists(name) ? "Script error: Invalid arguments to function \"%s\""
    : "Script error: there is no function named \"%s\"", name);
  return 0;
}


ExpFunctionCall::~ExpFunctionCall(void)
{
  delete[] arg_exprs;
  delete[] arg_expr_names;
}


AVSValue ExpFunctionCall::Evaluate(IScriptEnvironment* env)
{
  AVSValue result = Call(env);

  if (result.IsClip()) {
    return Cache::Create_Cache(result, 0, env);
  }

  return result;
}

}; // namespace avxsynth
