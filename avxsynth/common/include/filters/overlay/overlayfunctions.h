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

#ifndef __Overlay_funcs_h
#define __Overlay_funcs_h

#include "avxsynth/common/include/internal.h"
#include "imghelpers.h"
#include "blend_asm.h"

namespace avxsynth {
	
class OverlayFunction {
public:
  OverlayFunction() {
  }
  void setOpacity(int _opacity) { opacity = max(0,min(_opacity,256)); inv_opacity = 256-opacity; }
  void setEnv(IScriptEnvironment *_env) { env = _env;}
  virtual void BlendImage(Image444* base, Image444* overlay) = 0;
  virtual void BlendImageMask(Image444* base, Image444* overlay, Image444* mask) = 0;
protected:
  int opacity;
  int inv_opacity;
  IScriptEnvironment *env;
};

class OL_BlendImage : public OverlayFunction {
  void BlendImage(Image444* base, Image444* overlay);
  void BlendImageMask(Image444* base, Image444* overlay, Image444* mask);
private:
};

class OL_AddImage : public OverlayFunction {
  void BlendImage(Image444* base, Image444* overlay);
  void BlendImageMask(Image444* base, Image444* overlay, Image444* mask);
};

class OL_SubtractImage : public OverlayFunction {
  void BlendImage(Image444* base, Image444* overlay);
  void BlendImageMask(Image444* base, Image444* overlay, Image444* mask);
};

class OL_MultiplyImage : public OverlayFunction {
  void BlendImage(Image444* base, Image444* overlay);
  void BlendImageMask(Image444* base, Image444* overlay, Image444* mask);
};

class OL_BlendLumaImage : public OverlayFunction {
  void BlendImage(Image444* base, Image444* overlay);
  void BlendImageMask(Image444* base, Image444* overlay, Image444* mask);
private:
};

class OL_BlendChromaImage : public OverlayFunction {
  void BlendImage(Image444* base, Image444* overlay);
  void BlendImageMask(Image444* base, Image444* overlay, Image444* mask);
private:
};

class OL_LightenImage : public OverlayFunction {
  void BlendImage(Image444* base, Image444* overlay);
  void BlendImageMask(Image444* base, Image444* overlay, Image444* mask);
};

class OL_DarkenImage : public OverlayFunction {
  void BlendImage(Image444* base, Image444* overlay);
  void BlendImageMask(Image444* base, Image444* overlay, Image444* mask);
};

class OL_SoftLightImage : public OverlayFunction {
  void BlendImage(Image444* base, Image444* overlay);
  void BlendImageMask(Image444* base, Image444* overlay, Image444* mask);
};

class OL_HardLightImage : public OverlayFunction {
  void BlendImage(Image444* base, Image444* overlay);
  void BlendImageMask(Image444* base, Image444* overlay, Image444* mask);
};

class OL_DifferenceImage : public OverlayFunction {
  void BlendImage(Image444* base, Image444* overlay);
  void BlendImageMask(Image444* base, Image444* overlay, Image444* mask);
};

class OL_ExclusionImage : public OverlayFunction {
  void BlendImage(Image444* base, Image444* overlay);
  void BlendImageMask(Image444* base, Image444* overlay, Image444* mask);
};

}; // namespace avxsynth

#endif  // Overlay_funcs_h
