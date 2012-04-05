// Avisynth v1.0 beta.  Copyright 2000 Ben Rudiak-Gould.
// http://www.math.berkeley.edu/~benrg/avisynth.html

#ifndef f_ERROR_H
#define f_ERROR_H

#include "../internal.h"

namespace avxsynth {
	
static inline AvisynthError MyMemoryError() {
  return AvisynthError("Out of memory");
}

// the code for these is in AVIReadHandler.cpp

AvisynthError MyError(const char* fmt, ...);
AvisynthError MyWin32Error(const char *format, DWORD err, ...);

}; // namespace avxsynth

#endif
