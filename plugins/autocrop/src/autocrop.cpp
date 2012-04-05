/*

	Copyright (c) 2002 Glenn Bussell.  All rights reserved.
    glenn@videofringe.com

This file is subject to the terms of the GNU General Public License as
published by the Free Software Foundation.  A copy of this license is
included with this software distribution in the file COPYING.  If you
do not have a copy, you may obtain a copy by writing to the Free
Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

This software is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details

INTRODUCTION

AutoCrop is a simple AVISynth Filter that automatically crops the black borders from a clip. 
It operates in either preview mode where it overlays the recommended cropping information on the existing 
clip, or cropping mode where it really crops the clip. It can also ensure width and height are multiples 
of specified numbers so the cropped clip can be passed to the video compressor of your choice without problems.

This is my first AviSynth filter, so there may be some problems. Feedback about problems and suggestions for 
improvements are welcome.

The latest version of this filter is available at

http://www.videofringe.com/autocrop

*/


// Version 1.2 Released 02/01/2005



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <math.h>
#include <algorithm>

#include "avxplugin.h"
#include "info.h"
#include "infoYV12.h"
//#define DEBUG 1



int debugLog(char *str);
int autocropLog(char *str);
char* strpad(char *str,int len);

/*

My first time using YUV color so I need this. Taken from a Doom9 forum post by Donald Graft 

Y1 U1+2 Y2 V1+2 Y3 U3+4 Y4 U3+4 ... 

... where U1+2 means it applies to pixels 1 and 2. Thus, Y is fully sampled but U and V are subsampled. 



code:
--------------------------------------------------------------------------------

RGB to YUV Conversion
Y  =      (0.257 * R) + (0.504 * G) + (0.098 * B) + 16
Cr = V =  (0.439 * R) - (0.368 * G) - (0.071 * B) + 128
Cb = U = -(0.148 * R) - (0.291 * G) + (0.439 * B) + 128

YUV to RGB Conversion
B = 1.164(Y - 16)                   + 2.018(U - 128)
G = 1.164(Y - 16) - 0.813(V - 128) - 0.391(U - 128)
R = 1.164(Y - 16) + 1.596(V - 128)

*/

int Round(double num)
{
  if (num > 0)
  {
    num += 0.5;
    return floor(num);
  }
  else
  {
    num -= 0.5;
    return ceil(num);
  }
} 

class AutoCrop : public GenericVideoFilter {
		int top,bottom,left,right,width,height,nw,nh,sstart,send;
		int xMultipleOf,yMultipleOf;
		int leftAdd,rightAdd,topAdd,bottomAdd;
		int mode,threshold,samples;
		float aspect;
		char *cropString;
		char buf[1024];
public:
    AutoCrop(PClip _child, int _mode, int _wMultOf, int _hMultOf, int _leftAdd, int _topAdd, int _rightAdd, int _bottomAdd, int _threshold, int _samples, int _samplesstart, int _samplesend, float _aspect, IScriptEnvironment *env) : GenericVideoFilter(_child) {
			xMultipleOf = std::min(_wMultOf,vi.width);
			yMultipleOf = std::min(_hMultOf,vi.height);
			if (vi.IsYUY2()) {
				leftAdd = (_leftAdd + _leftAdd%2)*2;
				rightAdd = (_rightAdd + _rightAdd%2)*2;
			} else {
				leftAdd= _leftAdd;
				rightAdd = _rightAdd;
			}
			topAdd = _topAdd;
			bottomAdd = _bottomAdd;
			mode = _mode;
			threshold = _threshold;
			samples = _samples;
			sstart = _samplesstart;
			send = _samplesend;
			aspect = _aspect;
			if (aspect==-1)
				aspect = (float)vi.width/(float)vi.height;
#ifdef DEBUG
			sprintf(buf,"Aspect Ratio is %f",aspect);
			debugLog(buf);
#endif

			if (xMultipleOf==0)
				env->ThrowError("wMultOf must be non zero.");
			if (yMultipleOf==0)
				env->ThrowError("hMultOf must be non zero");

			if (vi.IsYUY2())
			{
				if (xMultipleOf%2!=0)
					env->ThrowError("wMultOf must be a multiple of 2 for YUY2.");
			}
			else if (vi.IsYV12()) {
				if (xMultipleOf%2!=0)
					env->ThrowError("wMultOf must be a multiple of 2 for YV12.");
			} else {
				env->ThrowError("AutoCrop currently supports YUY2 and YV12 color formats only.");
			}
			if (yMultipleOf%2!=0)
				env->ThrowError("hMultOf must be a multiple of 2.");


			SampleFrames(env);			
			if (mode==0 || mode==3) {
				if (vi.IsYUY2())
					vi.width = (right-left)/2+1;
				else
					vi.width = (right-left)+1;
				vi.height = bottom-top+1;
			}
			if (mode==2 || mode==3) {
				if (vi.IsYUY2())
					sprintf(buf,"Crop(%d,%d,%d,%d)",left/2,top,((right-left)/2)+1,(bottom-top)+1);
				else
					sprintf(buf,"Crop(%d,%d,%d,%d)",left,top,(right-left)+1,(bottom-top)+1);
				autocropLog(buf);
			}
		}
    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
		int __stdcall SampleFrames(IScriptEnvironment* env);
		int __stdcall FindBordersYUY2(IScriptEnvironment* env,int frameno);
		int __stdcall FindBordersYV12(IScriptEnvironment* env,int frameno);
};


int __stdcall AutoCrop::FindBordersYV12(IScriptEnvironment* env,int frameno)
{
		int found,lum,avglum;
    PVideoFrame eval;
		DWORD start, finish;
		double duration;
		char buf[1024];

#ifdef DEBUG
		start = timeGetTime();
#endif
		eval = child->GetFrame(frameno, env);  // Fetch frame
#ifdef DEBUG
		finish = timeGetTime();
		duration = (double)(finish - start);
		sprintf(buf,"Frame %d Load %f",frameno, duration );
		debugLog(buf);
		start = timeGetTime();
#endif



		const unsigned char *evalp;

		evalp = eval->GetReadPtr(PLANAR_Y);

		// Find the top
		found = 0;
		for (int ctop = 0; ctop < eval->GetHeight()-1; ctop++) {
			lum=0;
			int x = 0;
			for (x = 0; x < eval->GetRowSize(PLANAR_Y); x++) {	  // Sum the luminance for that line
				lum+=evalp[x];					
			}
			avglum = lum/x;
			if (avglum>=threshold) { 		// If the average luminance if > threshold we've found the top
				found++;
				if (found==3) {
					if (ctop-2<top)
						top = ctop-2;
					break;
				}
			} else
				found = 0;
			evalp += eval->GetPitch(PLANAR_Y);
		}

		// Find the bottom
		found=0;
		for (int cbottom = eval->GetHeight()-1; cbottom>=0; cbottom--) {
			lum=0;
			evalp = eval->GetReadPtr(PLANAR_Y) + eval->GetPitch(PLANAR_Y)*cbottom;	
			int x = 0;
			for (x = 0; x < eval->GetRowSize(PLANAR_Y); x++) // Sum the luminance for that line
				lum+=evalp[x];					
			avglum = lum/x;
			if (avglum>=threshold) { // If the average luminance if > threshold we've found the top
				found++;
				if (found==3) {
					if (cbottom+2>bottom)							
					bottom = cbottom+2;							
					break;
				}
			} else 
				found = 0;
		}



		// Find the Left
		found = 0;				
		for (int cleft = 0; cleft < eval->GetRowSize(PLANAR_Y); cleft++) {
			evalp = eval->GetReadPtr(PLANAR_Y) + cleft;					
			lum=0;
			for (int y = 0; y < eval->GetHeight(PLANAR_Y) ; y++) {	  // Sum the luminance for that line
				lum+=evalp[y*eval->GetPitch(PLANAR_Y)];					
			}
			avglum = lum/eval->GetHeight();
			if (avglum>=threshold) { 		// If the average luminance if > threshold we've found the top
				found++;
				if (found==3) {
					if (cleft-2<left)
						left = cleft-2;
					break;
				}
			} else
				found = 0;
		}


		// Find the Right
		found = 0;				
		for (int cright = eval->GetRowSize(PLANAR_Y)-1; cright >=0 ; cright--) {
			evalp = eval->GetReadPtr(PLANAR_Y) + cright;					
			lum=0;
			for (int y = 0; y < eval->GetHeight(); y++) {	  // Sum the luminance for that line
				lum+=evalp[y*eval->GetPitch(PLANAR_Y)];					
			}

			avglum = lum/eval->GetHeight();
			if (avglum>=threshold) { 		// If the average luminance if > threshold we've found the right
				found++;
				if (found==3) {
					if (cright+2>right)
						right = cright+2;
					break;
				}
			} else
				found = 0;
		}

//		sprintf(buf,"Crop(%d,%d,%d,%d)",left,top,right,bottom);
//		env->ThrowError(buf);


	#ifdef DEBUG
		finish = timeGetTime();
		duration = (double)(finish - start);
		sprintf(buf,"Processing %f", duration );
		debugLog(buf);
	#endif

		return 1;
}



int __stdcall AutoCrop::FindBordersYUY2(IScriptEnvironment* env,int frameno)
{
		int found,lum,avglum;
		PVideoFrame eval;
		eval = child->GetFrame(frameno, env);  // Fetch frame
		const unsigned char *evalp;

		evalp = eval->GetReadPtr();

		// Find the top
		found = 0;
		for (int ctop = 0; ctop < eval->GetHeight()-1; ctop++) {
			lum=0;
			for (int x = 0; x < eval->GetRowSize(); x+=2) {	  // Sum the luminance for that line
				lum+=evalp[x];					
			}
			avglum = lum/(eval->GetRowSize()/2);
			if (avglum>=threshold) { 		// If the average luminance if > threshold we've found the top
				found++;
				if (found==3) {
					if (ctop-2<top)
						top = ctop-2;
					break;
				}
			} else
				found = 0;
			evalp += eval->GetPitch();
		}

		// Find the bottom
		found=0;
		for (int cbottom = eval->GetHeight()-1; cbottom>=0; cbottom--) {
			lum=0;
			evalp = eval->GetReadPtr() + eval->GetPitch()*cbottom;					
			for (int x = 0; x < eval->GetRowSize(); x+=2) // Sum the luminance for that line
				lum+=evalp[x];					
			avglum = lum/(eval->GetRowSize()/2);
			if (avglum>=threshold) { // If the average luminance if > threshold we've found the top
				found++;
				if (found==3) {
					if (cbottom+2>bottom)							
					bottom = cbottom+2;							
					break;
				}
			} else 
				found = 0;
		}

		// Find the Left
		found = 0;				
		for (int cleft = 0; cleft < eval->GetRowSize()-2; cleft+=2) {
			evalp = eval->GetReadPtr() + cleft;					
			lum=0;
			for (int y = 0; y < eval->GetHeight() ; y++) {	  // Sum the luminance for that line
				lum+=evalp[y*eval->GetPitch()];					
			}
			avglum = lum/eval->GetHeight();
			if (avglum>=threshold) { 		// If the average luminance if > threshold we've found the top
				found++;
				if (found==3) {
					if (cleft-4<left)
						left = cleft-4;
					break;
				}
			} else
				found = 0;
		}

		// Find the Right
		found = 0;				
		for (int cright = eval->GetRowSize()-2; cright >=0 ; cright-=2) {
			evalp = eval->GetReadPtr() + cright;					
			lum=0;
			for (int y = 0; y < eval->GetHeight(); y++) {	  // Sum the luminance for that line
				lum+=evalp[y*eval->GetPitch()];					
			}

			avglum = lum/eval->GetHeight();
			if (avglum>=threshold) { 		// If the average luminance if > threshold we've found the top
				found++;
				if (found==3) {
					if (cright+4>right)
						right = cright+4;
					break;
				}
			} else
				found = 0;
		}
	#ifdef DEBUG
		sprintf(buf,"Right is %d",right);
		debugLog(buf);
	#endif
		return 1;
}



int __stdcall AutoCrop::SampleFrames(IScriptEnvironment* env)
{
    PVideoFrame eval;
		const int frames = vi.num_frames;
		const int framesToSample = samples;
		int remainder,a1,a2,tmp;


		if (vi.IsYUY2())
			tmp=2;
		else
			tmp=1;

		// Sample the required number of frames within the range specified
		if (sstart==0&&send==-1)
			sstart = frames/framesToSample;
		if (send==-1)
			send=frames-1;
		if (sstart>send) {
			tmp = sstart;
			sstart = send;
			send = tmp;
		}
		sstart = std::max(0,sstart);
		send = std:: min(frames,send-1);

		int interval = (send-sstart)/(framesToSample);

	#ifdef DEBUG
		sprintf(buf,"Int %d Start %d End %d Samples %d",interval,sstart,send,framesToSample);
		debugLog(buf);
	#endif

		// Setup some sensible defaults
		eval = child->GetFrame(1, env);  // Fetch frame
		top = eval->GetHeight()-1;
		bottom = 0;
		left=eval->GetRowSize()-1;
		right=0;

		for (int cnt=0; cnt<framesToSample ; cnt++) {
			if (vi.IsYUY2())
				FindBordersYUY2(env,sstart+cnt*interval);
			else
				FindBordersYV12(env,sstart+cnt*interval);

		}	

		// Add any extra cropping user asks for
		left = std::max(0,left+leftAdd);
		right = std::min(eval->GetRowSize(),right-rightAdd);
		top = std::max(0,top+topAdd);
		bottom = std::min(eval->GetHeight(),bottom-bottomAdd);

		width = ((right-left)/tmp)+1;
		height = (bottom-top+1);

		// Enforce aspect ratio if required
		if (aspect>0) {
			if ((float)width/aspect>height) {
				nw = Round((float)height * aspect);
				remainder = width - nw;
				a1 = (remainder/2);
				a2 = remainder - a1;
				left+=(a1*tmp);
				right-=(a2*tmp);
				width = nw;
			} else {
				nh = Round((float)width / aspect);
				remainder = height - nh;
				a1 = remainder/2;
				a2 = remainder - a1;
				top+=a1;
				bottom-=a2;
				height = nh;
			}
		}

		if (width<xMultipleOf) {
				remainder = xMultipleOf - width;
				a1 = (remainder/2);
				a2 = remainder - a1;
				left-=(a1*tmp);
				right+=(a2*tmp);
				width = xMultipleOf;
				if (left<0) {
					right-=left;
					left=0;
				}
				if (right>eval->GetRowSize()-tmp) {
					left-=(right-eval->GetRowSize()+tmp);
					right=eval->GetRowSize()-tmp;
				}
		} else {
			// Ensure new width is a multiple of the specified X
			remainder = (((right-left)/tmp)+1)%xMultipleOf;
			if (remainder) {
				a1 = (remainder/2);
				a2 = remainder - a1;
				left+=(a1*tmp);
				right-=(a2*tmp);
			}
		}
		// Ensure new height is a multiple of the specified Y
		if (height<yMultipleOf) {
				remainder = yMultipleOf - height;
				a1 = (remainder/2);
				a2 = remainder - a1;
				top-=a1;
				bottom+=a2;
				height = yMultipleOf;
				if (top<0) {
					bottom-=top;
					top=0;
				}
				if (bottom>eval->GetHeight()-1) {
						top-=bottom-eval->GetHeight()+1;
						bottom = eval->GetHeight()-1;
				}
		} else {
			remainder = (bottom-top+1)%yMultipleOf;
			if (remainder) {
				a1 = remainder/2;
				a2 = remainder - a1;
				top+=a1;
				bottom-=a2;
			}
		}

#ifdef DEBUG
		sprintf(buf,"2. L %d R %d W %d",left,right,width);
		debugLog(buf);
#endif

		// Ensure we the left point is an even no
		// This makes the crop easier
		if (vi.IsYUY2()) {
			if (left%4) {
				left+=2;
				right+=2;
			}
		}	else {
			// Ensure Valid Cropping points for YV12
			if (left%2) {
				left+=1;
				right+=1;
			}
			if (top%2) {
				top+=1;
				bottom+=1;
			}
		}
		return 1;
}


PVideoFrame __stdcall AutoCrop::GetFrame(int n, IScriptEnvironment* env) {

		char buf[2048];
    PVideoFrame frame = child->GetFrame(n, env);
		unsigned char* framep;
    int pitch,row_size;
    const int height = frame->GetHeight();
    const int frames = vi.num_frames;

		int x,y;


		// Write Cropping Information only, don't crop
		if (mode==1) {
			env->MakeWritable(&frame);
			if (vi.IsYUY2()) {
				pitch = frame->GetPitch();
				row_size = frame->GetRowSize();

				// Top
				framep = frame->GetWritePtr();
				framep += pitch*top;
				for (x = 0; x < row_size; x+=2)
						framep[x] = 255;
				// Bottom
				framep = frame->GetWritePtr();
				framep += pitch*bottom;
				for (x = 0; x < row_size; x+=2)
						framep[x] = 255;
				// Left
				framep = frame->GetWritePtr()+left;
				for (y=0 ; y<height ; y++) {
					framep[y*frame->GetPitch()]=255;
				}
				// Right
				framep = frame->GetWritePtr()+right;
				for (y=0 ; y<height ; y++) {
					framep[y*frame->GetPitch()]=255;
				}

				sprintf(buf,"Left %d Top %d",left/2,top);
				DrawStringYUY2(frame,left/2+10,top+10,strpad(buf,23));
				sprintf(buf,"Right %d Bottom %d ",right/2,bottom);
				DrawStringYUY2(frame,left/2+10,top+30,strpad(buf,23));
				sprintf(buf,"Width %d Height %d ",((right-left)/2)+1,(bottom-top)+1);
				DrawStringYUY2(frame,left/2+10,top+50,strpad(buf,23));
				sprintf(buf,"Crop(%d,%d,%d,%d)",left/2,top,((right-left)/2)+1,(bottom-top)+1);
				DrawStringYUY2(frame,left/2+10,top+70,strpad(buf,23));
			} else {
				sprintf(buf,"Pitch Y %d Pitch U %d Pitch V %d",frame->GetPitch(PLANAR_Y),frame->GetPitch(PLANAR_U),frame->GetPitch(PLANAR_V));
				debugLog(buf);
				sprintf(buf,"RowSize Y %d RowSize U %d RowSize V %d",frame->GetRowSize(PLANAR_Y),frame->GetRowSize(PLANAR_U),frame->GetRowSize(PLANAR_V));
				debugLog(buf);
				pitch = frame->GetPitch(PLANAR_Y);
				row_size = frame->GetRowSize(PLANAR_Y);
				// Top
				framep = frame->GetWritePtr(PLANAR_Y);
				framep += pitch*top;
				for (x = 0; x < row_size; x++)
						framep[x] = 255;

				// Bottom
				framep = frame->GetWritePtr(PLANAR_Y);
				framep += pitch*bottom;
				for (x = 0; x < row_size; x++)
						framep[x] = 255;

				// Left
				framep = frame->GetWritePtr()+left;
				for (y=0 ; y<height ; y++) {
					framep[y*pitch]=255;
				}

				// Right
				framep = frame->GetWritePtr()+right;
				for (y=0 ; y<height ; y++) {
					framep[y*pitch]=255;
				}

				sprintf(buf,"Left %d Top %d",left,top);
				DrawString(frame,left+10,top+10,strpad(buf,23));
				sprintf(buf,"Right %d Bottom %d ",right,bottom);
				DrawString(frame,left+10,top+30,strpad(buf,23));
				sprintf(buf,"Width %d Height %d ",((right-left))+1,(bottom-top)+1);
				DrawString(frame,left+10,top+50,strpad(buf,23));
				sprintf(buf,"Crop(%d,%d,%d,%d)",left,top,((right-left))+1,(bottom-top)+1);
				DrawString(frame,left+10,top+70,strpad(buf,23));
			}

			return frame;
		} else if (mode==0 || mode==3){
					// Crop
			if (vi.IsYUY2()) {
				return env->Subframe(frame,top * frame->GetPitch() + left, frame->GetPitch(), right-left+2, bottom-top+1);
			} 	else {

				PVideoFrame dst = env->NewVideoFrame(vi);
				// Y 
				env->BitBlt(dst->GetWritePtr(PLANAR_Y), dst->GetPitch(PLANAR_Y), frame->GetReadPtr(PLANAR_Y)+left+frame->GetPitch(PLANAR_Y)*top, frame->GetPitch(PLANAR_Y), (right-left+1),bottom-top+1);			
				// U
				env->BitBlt(dst->GetWritePtr(PLANAR_U), dst->GetPitch(PLANAR_U), frame->GetReadPtr(PLANAR_U)+left/2+frame->GetPitch(PLANAR_U)*top/2, frame->GetPitch(PLANAR_U), (right-left+1)/2,(bottom-top+1)/2);			
				// V 
				env->BitBlt(dst->GetWritePtr(PLANAR_V), dst->GetPitch(PLANAR_V), frame->GetReadPtr(PLANAR_V)+left/2+frame->GetPitch(PLANAR_V)*top/2, frame->GetPitch(PLANAR_V), (right-left+1)/2,(bottom-top+1)/2);			
				return dst;
//			return env->Subframe(frame,top * frame->GetPitch() + left, frame->GetPitch(), right-left+1, bottom-top+1, (top/2) * frame->GetPitch(PLANAR_U) + (left/2), (top/2) * frame->GetPitch(PLANAR_V) + (left/2), frame->GetPitch(PLANAR_U));
			}
		}
		return frame;
}


AVSValue __cdecl Create_AutoCrop(AVSValue args, void* user_data, IScriptEnvironment* env) {
    return new AutoCrop(args[0].AsClip(),
			args[1].AsInt(1),
			args[2].AsInt(4), 
			args[3].AsInt(2),
			args[4].AsInt(0),
			args[5].AsInt(0),
			args[6].AsInt(0),
			args[7].AsInt(0),
			args[8].AsInt(40),
			args[9].AsInt(5),
			args[10].AsInt(0),
			args[11].AsInt(-1),
			args[12].AsFloat(0),
			env);

}


extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env) {
    env->AddFunction("AutoCrop", "c[mode]i[wMultOf]i[hMultOf]i[leftAdd]i[topAdd]i[rightAdd]i[bottomAdd]i[threshold]i[samples]i[samplestartframe]i[sampleendframe]i[aspect]f", Create_AutoCrop, 0);
    return "`AutoCrop' Automatic Cropping plugin";
}


int debugLog(char *str)
{
#ifdef DEBUG
	char buf[4096];
	FILE *fp=NULL;
	sprintf(buf,"%s\n",str);
	fp = fopen("debug.log","a");
	fwrite(buf,strlen(buf),1,fp);
	fclose(fp);
#endif
	return 1;
}

int autocropLog(char *str)
{

	char buf[4096];
	FILE *fp=NULL;
	sprintf(buf,"%s\n",str);
	fp = fopen("AutoCrop.log","w");
	if (fp) {
		fwrite(buf,strlen(buf),1,fp);
		fclose(fp);
	} 
	return 1;
}


char* strpad(char *str,int len)
{
	int cnt=strlen(str);
	if (cnt>=len)
		return str;
	for (;cnt<len;cnt++)
		str[cnt]=' ';
	str[cnt]=0;
	return str;
}