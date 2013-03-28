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



// Avisynth filter: general convolution 3d
// by Richard Berg (avisynth-dev@richardberg.net)
// adapted from General Convolution 3D for VDub by Gunnar Thalin (guth@home.se)

#ifndef __GeneralConvolution_H__
#define __GeneralConvolution_H__

#include "internal.h"

namespace avxsynth {
	
/*****************************************
****** General Convolution 2D filter *****
*****************************************/


class GeneralConvolution : public GenericVideoFilter 
/** This class exposes a video filter that applies general convolutions -- up to a 5x5
  * kernel -- to a clip.  Smaller (3x3) kernels have their own code path.  SIMD support forthcoming.
 **/
{
public:
    GeneralConvolution(PClip _child, double _divisor, int _nBias, const char * _matrix, bool _autoscale, IScriptEnvironment* _env);
    virtual ~GeneralConvolution(void);
    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
    static AVSValue __cdecl Create(AVSValue args, void* user_data, IScriptEnvironment* env);

protected:
    void setMatrix(const char * _matrix, IScriptEnvironment* env);
    void initBuffers(IScriptEnvironment* env);

private:      
    double divisor;
    int nBias;
    unsigned int nSize;
    bool autoscale;

    // some buffers
    BYTE *pbyA, *pbyR, *pbyG, *pbyB;
    
           
    // Messy way of storing matrix, but avoids performance penalties of indirection    
    int i00;
    int i10;
    int i20;
    int i30;
    int i40;
    int i01;
    int i11;
    int i21;
    int i31;
    int i41;
    int i02;
    int i12;
    int i22;
    int i32;
    int i42;
    int i03;
    int i13;
    int i23;
    int i33;
    int i43;
    int i04;
    int i14;
    int i24;
    int i34;
    int i44;
};

}; // namespace avxsynth

#endif  // __GeneralConvolution_H__
