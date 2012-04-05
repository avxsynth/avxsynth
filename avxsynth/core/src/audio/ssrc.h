/******************************************************
  A fast and high quality sampling rate converter SSRC
                                           written by Naoki Shibata


Homepage : http://shibatch.sourceforge.net/
e-mail   : shibatch@users.sourceforge.net

Some changes are:

Copyright © 2001-2003, Peter Pawlowski
All rights reserved.

Other changes are:

Copyright © 2003, Klaus Post

*******************************************************/


#include "../internal.h"
#include <distrib/include/pfc/pfc.h>

typedef SFLOAT audio_sample;
typedef audio_sample REAL_inout;

#include "math_shared.h"

namespace avxsynth {
	
class Buffer
{
private:
	mem_block_t<REAL_inout> buffer;
	int buf_data;
public:
	_inline Buffer() {buf_data=0;buffer.set_mem_logic(mem_block::ALLOC_FAST_DONTGODOWN);}
	_inline REAL_inout * GetBuffer(int * siz) {*siz=buf_data;return buffer;}
	_inline int Size() {return buf_data;}
	void Read(int size);
	void Write(const REAL_inout * ptr,int size);
};


class Resampler_base
{
public:
	class CONFIG
	{
	public:
		int sfrq,dfrq,nch,dither,pdf,fast;
		_inline CONFIG(int _sfrq,int _dfrq,int _nch,int _dither,int _pdf,int _fast=1)
		{
			sfrq=_sfrq;
			dfrq=_dfrq;
			nch=_nch;
			dither=_dither;
			pdf=_pdf;
			fast=_fast;
		}	
	};

private:
	Buffer in,out;
	void bufloop(int finish);
protected:
	
	Resampler_base(const CONFIG & c);

	void _inline __output(REAL_inout value, int& delay2)
	{
		if(delay2 == 0) {
			out.Write(&value,1);
		} else {
			delay2--;
		}
	};

	virtual unsigned int Resample(REAL_inout * input,unsigned int size,int ending)=0;
	double peak;
	int nch,sfrq,dfrq;
	double gain;
	

	double AA,DF;
	int FFTFIRLEN;

public:
	double GetPeak() {return peak;}//havent tested if this actually still works
	
	void Write(const REAL_inout* input,int size);
	_inline void Finish() {bufloop(1);}
	
	_inline REAL_inout* GetBuffer(int * s) {return out.GetBuffer(s);}
	_inline void Read(unsigned int s) {out.Read(s);}

	unsigned int GetLatency();//returns amount of audio data in in/out buffers in milliseconds

	_inline unsigned int GetDataInInbuf() {return in.Size();}

	virtual ~Resampler_base() {}

	static Resampler_base * Create(CONFIG & c);
};

#define SSRC_create(sfrq,dfrq,nch,dither,pdf,fast) \
	Resampler_base::Create(Resampler_base::CONFIG(sfrq,dfrq,nch,dither,pdf,fast))

/*
USAGE:
Resampler_base::Create() / SSRC_create with your resampling params (see source for exact info what they do)

 loop:
Write() your PCM data to be resampled
GetBuffer() to get pointer to buffer with resampled data and amount of resampled data available (in bytes)
(note: SSRC operates on blocks, don't expect it to return any data right after your first Write() )

Read() to tell the resampler how much data you took from the buffer ( <= size returned by GetBuffer )

you can use GetLatency() to get amount of audio data (in ms) currently being kept in resampler's buffers
(quick hack for determining current output time without extra stupid counters)

Finish() to force-convert remaining PCM data after last Write() (warning: never Write() again after Flush() )

delete when done

*/

}; // namespace avxsynth
