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

#include <internal.h>
#include <stdio.h>
#include <limits.h>

namespace avxsynth {

enum {
  MODE_UNKNOWN = -1,
  MODE_INT = 1,
  MODE_FLOAT = 2,
  MODE_BOOL = 3
};

class ConditionalReader : public GenericVideoFilter
{
private:
  const bool show;
  int* intVal;
  bool* boolVal;
  float* floatVal;
  const char* variableName;
  int mode;

  AVSValue ConvertType(const char* content, int line, IScriptEnvironment* env);
  void SetRange(int start_frame, int stop_frame, AVSValue v);
  void SetFrame(int framenumber, AVSValue v);
  void ThrowLine(const char* err, int line, IScriptEnvironment* env);
  AVSValue GetFrameValue(int framenumber);

public:
  ConditionalReader(PClip _child, const char* filename, const char* _varname, bool _show, IScriptEnvironment* env);
  ~ConditionalReader(void);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
  static AVSValue __cdecl Create(AVSValue args, void* user_data, IScriptEnvironment* env);
};


// Helper function - exception protected wrapper

inline AVSValue GetVar(IScriptEnvironment* env, const char* name) {
  try {
    return env->GetVar(name);
  }
  catch (IScriptEnvironment::NotFound) {}

  return AVSValue();
}


/*****************************************************************************
 *  Helper code from XviD (http://www.xvid.org)
 *
 *  Copyright (C)      2002 Foxer <email?>
 *                     2002 Dirk Knop <dknop@gwdg.de>
 *                2002-2003 Edouard Gomez <ed.gomez@free.fr>
 *                     2003 Pete Ross <pross@xvid.org>
 *****************************************************************************/

/* Default buffer size for reading lines */
#define BUF_SZ   1024


/* This function returns an allocated string containing a complete line read
 * from the file starting at the current position */
static char *
readline(FILE *f)
{
	char *buffer = NULL;
	int buffer_size = 0;
	int pos = 0;

	do {
		int c;

		/* Read a character from the stream */
		c = fgetc(f);

		/* Is that EOF or new line ? */
		if(c == EOF || c == '\n')
			break;

		/* Do we have to update buffer ? */
		if(pos >= buffer_size - 1) {
			buffer_size += BUF_SZ;
			buffer = (char*)realloc(buffer, buffer_size);
			if (buffer == NULL)
				return(NULL);
		}

		buffer[pos] = c;
		pos++;
	} while(1);

	/* Read \n or EOF */
	if (buffer == NULL) {
		/* EOF, so we reached the end of the file, return NULL */
		if(feof(f))
			return(NULL);

		/* Just an empty line with just a newline, allocate a 1 byte buffer to
		 * store a zero length string */
		buffer = (char*)malloc(1);
		if(buffer == NULL)
			return(NULL);
	}

	/* Zero terminated string */
	buffer[pos] = '\0';

	return(buffer);
}

/* This function returns a pointer to the first non space char in the given
 * string */
static char *
skipspaces(char *string)
{
	const char spaces[] =
		{
			' ','\t','\0'
		};
	const char *spacechar = spaces;

	if (string == NULL) return(NULL);

	while (*string != '\0') {
		/* Test against space chars */
		while (*spacechar != '\0') {
			if (*string == *spacechar) {
				string++;
				spacechar = spaces;
				break;
			}
			spacechar++;
		}

		/* No space char */
		if (*spacechar == '\0') return(string);
	}

	return(string);
}

/* This function returns a boolean that tells if the string is only a
 * comment */
static int
iscomment(char *string)
{
	const char comments[] =
		{
			'#',';', '%', '\0'
		};
	const char *cmtchar = comments;
	int iscomment = 0;

	if (string == NULL) return(1);

	string = skipspaces(string);

	while(*cmtchar != '\0') {
		if(*string == *cmtchar) {
			iscomment = 1;
			break;
		}
		cmtchar++;
	}

	return(iscomment);
}

/* ------------------------------------------------------------------------------
** Write function to evaluate expressions per frame and write the results to file
** Ernst Peché, 2004
*/

#define maxWriteArgs 16

class Write : public GenericVideoFilter
{
private:
	FILE * fout;
	int linecheck;	// 0=write each line, 1=write only if first expression == true, -1 = write at start, -2 = write at end
	bool flush;
	bool append;

	char filename[PATH_MAX];
	char mode[10];	//file open mode
	int arrsize;
	struct exp_res {
		char expression[255];
		char string[255];
	} arglist[maxWriteArgs];	//max 16 args

public:
    Write(PClip _child, const char _filename[], AVSValue args, int _linecheck, bool _flush, bool _append, IScriptEnvironment* env);
	~Write(void);
	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
	static AVSValue __cdecl Create(AVSValue args, void* user_data, IScriptEnvironment* env);
	static AVSValue __cdecl Create_If(AVSValue args, void* user_data, IScriptEnvironment* env);
	static AVSValue __cdecl Create_Start(AVSValue args, void* user_data, IScriptEnvironment* env);
	static AVSValue __cdecl Create_End(AVSValue args, void* user_data, IScriptEnvironment* env);
	bool DoEval(IScriptEnvironment* env);
	void FileOut(IScriptEnvironment* env);
};

}; // namespace avxsynth
