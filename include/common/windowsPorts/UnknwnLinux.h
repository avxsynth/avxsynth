#ifndef __UNKNWN_LINUX_H__
#define __UNKNWN_LINUX_H__

namespace avxsynth {

/////////////////////////////////
// Copied from Windows Unknwn.h
/////////////////////////////////

typedef struct _IUnknown {
  HRESULT QueryInterface (REFIID riid, LPVOID * ppvObj){ return 0;};
  ULONG AddRef() {return 0L;};
  ULONG Release() { return 0L;};
} IUnknown;

typedef IUnknown* LPUNKNOWN;    


}; // namespace avxsynth

#endif //  __UNKNWN_LINUX_H__
