/*
  ConditionalReader  (c) 2004 by Klaus Post

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  The author can be contacted at:
  sh0dan[at]stofanet.dk
*/

#include "stdafx.h"
#include "conditional_reader.h"
#include "avxlog.h"
#include <text-overlay.h>
#include <errno.h>

namespace avxsynth {
	
#ifdef MODULE_NAME
#undef MODULE_NAME
#endif

#define MODULE_NAME core::conditional_reader

ConditionalReader::ConditionalReader(PClip _child, const char* filename, const char _varname[], bool _show, IScriptEnvironment* env) :
  GenericVideoFilter(_child), show(_show), variableName(_varname)
{
  FILE * f;
  char *line;
  int lines;

  if ((f = fopen(filename, "rb")) == NULL)
    env->ThrowError("ConditionalReader: Could not open file '%s'.", filename);

  lines = 0;
  mode = MODE_UNKNOWN;

  while ((line = readline(f)) != NULL) {
    char *ptr;
    int fields;

    lines++;

    /* We skip spaces */
    ptr = skipspaces(line);

    /* Skip coment lines or empty lines */
    if(iscomment(ptr) || *ptr == '\0') {
      free(line);
      continue;
    }

    if (mode == MODE_UNKNOWN) {
      // We have not recieved a mode - We expect type.
      char keyword [1024];
      char type [1024];
      fields = sscanf(ptr,"%1023s %1023s", keyword, type);
      if (fields) {
        if (!strcasecmp((const char*)keyword, "type")) {
          if (!strcasecmp((const char*)type, "int")) {
            mode = MODE_INT;
            intVal = new int[vi.num_frames];
          } else if (!strcasecmp((const char*)type, "float")) {
            mode = MODE_FLOAT;
            floatVal = new float[vi.num_frames];
          } else if (!strcasecmp((const char*)type, "bool")) {
            mode = MODE_BOOL;
            boolVal = new bool[vi.num_frames];
          } else {
            ThrowLine("ConditionalReader: Unknown 'type' specified in line %d", lines, env);
          }// end if compare type
        }// end if compare keyword
      }// end if fields

    } else { // We have a defined mode and allocated the values.

      char keyword [1024];
      char type [1024];
      fields = sscanf(ptr,"%1023s %1023s", keyword, type);

      if (!strcasecmp((const char*)keyword, "default")) {
        AVSValue def = ConvertType((const char*)type, lines, env);
        SetRange(0, vi.num_frames-1, def);
        free(line);
        continue;
      } // end if "default"

      if (ptr[0] == 'R' || ptr[0] == 'r') {  // Range
        ptr++;
        ptr = skipspaces(ptr);
        int start;
        int stop;
        char value [64];
        fields = sscanf(ptr, "%d %d %63s", &start, &stop, value);

        if (fields != 3) 
          ThrowLine("ConditionalReader: Could not read range in line %d", lines, env);
        if (start > stop)
          ThrowLine("ConditionalReader: The start frame is after the end frame in line %d", lines, env);

        AVSValue set = ConvertType((const char*)value, lines, env);
        SetRange(start, stop, set);
      } else if (ptr[0] == 'I' || ptr[0] == 'i') {  // Interpolate
        if (mode == MODE_BOOL)
          ThrowLine("ConditionalReader: Cannot interpolate booleans in line %d", lines, env);

        ptr++;
        ptr = skipspaces(ptr);
        int start;
        int stop;
        char start_value [64];
        char stop_value [64];
        fields = sscanf(ptr, "%d %d %63s %63s", &start, &stop, start_value, stop_value);

        if (fields != 4) 
          ThrowLine("ConditionalReader: Could not read interpolation range in line %d", lines, env);
        if (start > stop)
          ThrowLine("ConditionalReader: The start frame is after the end frame in line %d", lines, env);

        AVSValue set_start = ConvertType((const char*)start_value, lines, env);
        AVSValue set_stop = ConvertType((const char*)stop_value, lines, env);

        int range = stop-start;
		double diff = set_stop.AsFloat() - set_start.AsFloat();
        for (int i = 0; i<=range; i++) {
          double where = (double)(i)/(double)range;
          double n = where * diff + set_start.AsFloat();
          SetFrame(i+start, (mode == MODE_FLOAT)
                  ? AVSValue(n)
                  : AVSValue((int) n));
        }
      } else {
        char value [64];
        int cframe;
        fields = sscanf(ptr, "%d %63s", &cframe, value);
        if (fields == 2) {
          AVSValue set = ConvertType((const char*)value, lines, env);
          SetFrame(cframe, set);
        } else {
          AVXLOG_INFO("ConditionalReader: Ignored line %d.\n", lines);
        }
      }
    
    } // End we have defined type
    free(line);
  }// end while still some file left to read.

  /* We are done with the file */
  fclose(f);

  if (mode == MODE_UNKNOWN)
    env->ThrowError("ConditionalReader: Mode was not defined!");

}



// Converts from the char array given to the type specified.

AVSValue ConditionalReader::ConvertType(const char* content, int line, IScriptEnvironment* env)
{
  if (mode == MODE_UNKNOWN)
    ThrowLine("ConditionalReader: Type has not been defined. Line %d", line, env);

  int fields;
  switch (mode) {
    case MODE_INT:
      int ival;
      fields = sscanf(content, "%d", &ival);
      if (!fields)
        ThrowLine("ConditionalReader: Could not find an expected integer at line %d!", line, env);

      return AVSValue(ival);

    case MODE_FLOAT:
      float fval;
      fields = sscanf(content, "%e", &fval);
      if (!fields)
        ThrowLine("ConditionalReader: Could not find an expected float at line %d!", line, env);
      return AVSValue(fval);

    case MODE_BOOL:
      char bval [8];
      fields = sscanf(content, "%7s", bval);
      if (!strcasecmp((const char*)bval, "true")) {
        return AVSValue(true);
      }
      else if (!strcasecmp((const char*)bval, "t")) {
        return AVSValue(true);
      }
      else if (!strcasecmp((const char*)bval, "yes")) {
        return AVSValue(true);
      }
      else if (!strcasecmp((const char*)bval, "false")) {
        return AVSValue(false);
      }
      else if (!strcasecmp((const char*)bval, "f")) {
        return AVSValue(false);
      }
      else if (!strcasecmp((const char*)bval, "no")) {
        return AVSValue(false);
      } 
      ThrowLine("ConditionalReader: Boolean value was not true or false in line %d", line, env);
  }
  return AVSValue(0);
}


// Sets range with both start and stopframe inclusive.

void ConditionalReader::SetRange(int start_frame, int stop_frame, AVSValue v) {
  int i;
  start_frame = std::max(std::min(start_frame, vi.num_frames-1), 0);
  stop_frame = std::max(std::min(stop_frame, vi.num_frames-1), 0);
  int p;
  float q;
  bool r;

  switch (mode) {
    case MODE_INT:
      p = v.AsInt();
      for (i = start_frame; i <= stop_frame; i++) {
        intVal[i] = p;
      }
      break;
    case MODE_FLOAT:
      q = (float)v.AsFloat();
      for (i = start_frame; i <= stop_frame; i++) {
        floatVal[i] = q;
      }
      break;
    case MODE_BOOL:
      r = v.AsBool();
      for (i = start_frame; i <= stop_frame; i++) {
        boolVal[i] = r;
      }
      break;
  }
}

// Sets the value of one frame.

void ConditionalReader::SetFrame(int framenumber, AVSValue v) {

  if (framenumber < 0 || framenumber > vi.num_frames-1 )
    return;

  switch (mode) {
    case MODE_INT:
      intVal[framenumber] = v.AsInt();
      break;
    case MODE_FLOAT:
      floatVal[framenumber] = (float)v.AsFloat();
      break;
    case MODE_BOOL:
      boolVal[framenumber] = v.AsBool();
      break;
  }
}

// Get the value of a frame.
AVSValue ConditionalReader::GetFrameValue(int framenumber) {
  framenumber = std::max(std::min(framenumber, vi.num_frames-1), 0);

  switch (mode) {
    case MODE_INT:
      return AVSValue(intVal[framenumber]);
      break;
    case MODE_FLOAT:
      return AVSValue(floatVal[framenumber]);
      break;
    case MODE_BOOL:
      return AVSValue(boolVal[framenumber]);
      break;
  }
  return AVSValue(0);
}

// Destructor
ConditionalReader::~ConditionalReader(void) 
{
  switch (mode) {
    case MODE_INT:
      delete[] intVal;
      break;
    case MODE_FLOAT:
      delete[] floatVal;
      break;
    case MODE_BOOL:
      delete[] boolVal;
      break;
  }
}


void ConditionalReader::ThrowLine(const char* err, int line, IScriptEnvironment* env) {
  env->ThrowError(err, line);
}


PVideoFrame __stdcall ConditionalReader::GetFrame(int n, IScriptEnvironment* env)
{
  AVSValue v = GetFrameValue(n);
  env->SetGlobalVar(variableName, v);

  PVideoFrame src = child->GetFrame(n,env);

  if (show) {
    AVSValue v2 = env->Invoke("String", v);
    env->MakeWritable(&src);
    ApplyMessage(&src, vi, v2.AsString(""), vi.width/2, 0xa0a0a0,0,0 , env );
  }
  return src;
}




AVSValue __cdecl ConditionalReader::Create(AVSValue args, void* user_data, IScriptEnvironment* env)
{
  return new ConditionalReader(args[0].AsClip(), args[1].AsString(""), args[2].AsString("Conditional") , args[3].AsBool(false), env);
}


// Write ------------------------------------------------

Write::Write (PClip _child, const char _filename[], AVSValue args, int _linecheck, bool _append, bool _flush, IScriptEnvironment* env):
	GenericVideoFilter(_child), linecheck(_linecheck), flush(_flush), append(_append)
{
    memset(filename, 0, PATH_MAX*sizeof(char));
	char* retValue = realpath(_filename, filename);	// may check for NULL return value
    if((NULL == retValue) && (0 == strlen(filename)))
    {
        env->ThrowError("Write::Write() failed determining realpath, error = %s\n", strerror(errno));
    }
	arrsize = std::min(args.ArraySize(), maxWriteArgs);
	int i;
	
	for (i=0; i<maxWriteArgs; i++) strcpy(arglist[i].string, "");

	for (i=0; i<arrsize; i++) {
		strncpy(arglist[i].expression, args[i].AsString(""), 254);
	}

	if (append) {
		strcpy(mode, "a+t");
	} else {
		strcpy(mode, "w+t");
	}

	fout = fopen(filename, mode);	//append or purge file
	if (!fout) env->ThrowError("Write: File '%s' cannot be opened.", filename);
	
	if (flush) fclose(fout);	//will be reopened in FileOut

	strcpy(mode, "a+t");	// in GetFrame always appending

	if (linecheck == -1) {	//write at start
		env->SetVar("current_frame",-1);
		Write::DoEval(env);
		Write::FileOut(env);
	}
	if (linecheck == -2) {	//write at end, evaluate right now
		env->SetVar("current_frame",-2);
		Write::DoEval(env);
	}
}

PVideoFrame __stdcall Write::GetFrame(int n, IScriptEnvironment* env) {

//changed to call write AFTER the child->GetFrame


	PVideoFrame tmpframe = child->GetFrame(n, env);

	if (linecheck<0) return tmpframe;	//do nothing here when writing only start or end

	AVSValue prev_last = env->GetVar("last");  // Store previous last
	AVSValue prev_current_frame = GetVar(env, "current_frame");  // Store previous current_frame

	env->SetVar("last",(AVSValue)child);       // Set implicit last (to avoid recursive stack calls?)
	env->SetVar("current_frame",n);
	
	if (Write::DoEval(env)) {
		Write::FileOut(env);
	}

	env->SetVar("last",prev_last);       // Restore implicit last
	env->SetVar("current_frame",prev_current_frame);       // Restore current_frame

	return tmpframe;

};

Write::~Write(void) {
	if (append) {
		strcpy(mode, "a+t");
	} else {
		strcpy(mode, "w+t");
	}

	if (linecheck == -2) {	//write at end
		Write::FileOut(0);
	}
	if (!flush) fclose(fout);
};

void Write::FileOut(IScriptEnvironment* env) {
	int i;
	if (flush) {
		fout = fopen(filename, mode);
		if (!fout) {
			if (env) env->ThrowError("Write: File '%s' cannot be opened.", filename);
			return;
		}
	}
	for (i= ( (linecheck==1) ? 1 : 0) ; i<maxWriteArgs; i++ ) {
		fprintf(fout, "%s", arglist[i].string );
	}
	fprintf(fout, "\n");
	if (flush) {
		fclose(fout);
	}
}

bool Write::DoEval( IScriptEnvironment* env) {
	bool keep_this_line = true;
	int i;
	AVSValue expr;
	AVSValue result;

	for (i=0; i<arrsize; i++) {
		expr = arglist[i].expression;
		
		if ( (linecheck==1) && (i==0)) {
			try {
				result = env->Invoke("Eval",expr);
				if (!result.AsBool(true)) {
					keep_this_line = false;
					break;
				}
			} catch (AvisynthError) {
//				env->ThrowError("Write: Can't eval linecheck expression!"); // results in KEEPING the line
			}
		} else {
			try {
				result = env->Invoke("Eval",expr);
				result = env->Invoke("string",result);	//convert all results to a string
				strncpy(arglist[i].string, result.AsString(""),254);
			} catch (AvisynthError error) {
				strncpy(arglist[i].string, error.msg, 254);
			}
		}
	}
	return keep_this_line;
}

AVSValue __cdecl Write::Create(AVSValue args, void* user_data, IScriptEnvironment* env)
{
	return new Write(args[0].AsClip(), args[1].AsString(""), args[2], 0, args[3].AsBool(true),args[4].AsBool(true), env);
}

AVSValue __cdecl Write::Create_If(AVSValue args, void* user_data, IScriptEnvironment* env)
{
	return new Write(args[0].AsClip(), args[1].AsString(""), args[2], 1, args[3].AsBool(true),args[4].AsBool(true), env);
}

AVSValue __cdecl Write::Create_Start(AVSValue args, void* user_data, IScriptEnvironment* env)
{
	return new Write(args[0].AsClip(), args[1].AsString(""), args[2], -1, args[3].AsBool(false), true, env);
}

AVSValue __cdecl Write::Create_End(AVSValue args, void* user_data, IScriptEnvironment* env)
{
	return new Write(args[0].AsClip(), args[1].AsString(""), args[2], -2, args[3].AsBool(true), true, env);
}

}; // namespace avxsynth
