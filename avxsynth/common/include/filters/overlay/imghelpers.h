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

// Overlay (c) 2003, 2004 by Klaus Post

#ifndef __Overlay_helpers_h
#define __Overlay_helpers_h

#include "avxlog.h"

namespace avxsynth {

#ifdef MODULE_NAME
#undef MODULE_NAME
#endif

#define MODULE_NAME core::imghelpers
	
class Image444 {
private:
  BYTE* Y_plane;
  BYTE* U_plane;
  BYTE* V_plane;

  BYTE* fake_Y_plane;
  BYTE* fake_U_plane;
  BYTE* fake_V_plane;

  int fake_w;
  int fake_h;
  const int _w;
  const int _h;

  bool return_original;


public:
  int pitch;

  Image444() : _w(0), _h(0) {}

  Image444(Image444* img) : _w(img->w()), _h(img->h()), pitch(img->pitch) {
    Y_plane = img->GetPtr(PLANAR_Y);
    U_plane = img->GetPtr(PLANAR_U);
    V_plane = img->GetPtr(PLANAR_V);
    ResetFake();
  }

  Image444(int _inw, int _inh) : _w(_inw), _h(_inh) {
    pitch = (_w+15)&(~15);
    Y_plane = (BYTE*)_aligned_malloc(pitch*_h, 64); 
    U_plane = (BYTE*)_aligned_malloc(pitch*_h, 64); 
    V_plane = (BYTE*)_aligned_malloc(pitch*_h, 64); 
    ResetFake();
  }

  Image444(BYTE* Y, BYTE* U, BYTE* V, int _inw, int _inh, int _pitch) : _w(_inw), _h(_inh) {
    if (!(_w && _h)) {
      AVXLOG_INFO("%s", Image444: Height or Width is 0");
    }
    Y_plane = Y;
    U_plane = U;
    V_plane = V;
    pitch = _pitch;
    ResetFake();
  }

  void free_chroma() {
    _aligned_free(U_plane);
    _aligned_free(V_plane);
  }

  void free_luma() {
    _aligned_free(Y_plane);
  }

  void free() {
    if (!(_w && _h)) {
      AVXLOG_INFO("%s", "Image444: Height or Width is 0");
    }
    free_luma();
    free_chroma();
  }

  __inline int w() { return (return_original) ? _w : fake_w; }
  __inline int h() { return (return_original) ? _h : fake_h; }

  BYTE* GetPtr(int plane) {
    if (!(_w && _h)) {
      AVXLOG_INFO("%s", "Image444: Height or Width is 0");
    }
    switch (plane) {
      case PLANAR_Y:
        return (return_original) ? Y_plane : fake_Y_plane;
      case PLANAR_U:
        return (return_original) ? U_plane : fake_U_plane;
      case PLANAR_V:
        return (return_original) ? V_plane : fake_V_plane;
    }
    return Y_plane;
  }

  void SetPtr(BYTE* ptr, int plane) {
    if (!(_w && _h)) {
      AVXLOG_INFO("%s", "Image444: Height or Width is 0");
    }
    switch (plane) {
      case PLANAR_Y:
        fake_Y_plane = Y_plane = ptr;
        break;
      case PLANAR_U:
        fake_Y_plane = U_plane = ptr;
        break;
      case PLANAR_V:
        fake_Y_plane = V_plane = ptr;
        break;
    }
  }

  void SubFrame(int x, int y, int new_w, int new_h) {
    new_w = min(new_w, w()-x);
    new_h = min(new_h, h()-y);

    fake_Y_plane = GetPtr(PLANAR_Y) + x + (y*pitch);
    fake_U_plane = GetPtr(PLANAR_U) + x + (y*pitch);
    fake_V_plane = GetPtr(PLANAR_V) + x + (y*pitch);
    
    fake_w = new_w;
    fake_h = new_h;
  }

  bool IsSizeZero() {
    if (w()<=0) return true;
    if (h()<=0) return true;
    if (!(pitch && Y_plane && V_plane && U_plane)) return true;
    return false;
  }

  void ReturnOriginal(bool shouldI) {
    return_original = shouldI;
  }

  void ResetFake() {
    return_original = true;
    fake_Y_plane = Y_plane;
    fake_U_plane = U_plane;
    fake_V_plane = V_plane;
    fake_w = _w;
    fake_h = _h;
  }

};

}; // namespace avxsynth

#endif
