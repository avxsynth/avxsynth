#ifndef __SOURCE_PRIVATE_H__
#define __SOURCE_PRIVATE_H__

#define PI 3.1415926535897932384626433832795
#include <ctime>

namespace avxsynth {
	
AVSValue __cdecl Create_SegmentedSource(AVSValue args, void* use_directshow, IScriptEnvironment* env);
AVSValue __cdecl Create_BlankClip(AVSValue args, void*, IScriptEnvironment* env);
AVSValue __cdecl Create_Version(AVSValue args, void*, IScriptEnvironment* env);
AVSValue __cdecl Create_MessageClip(AVSValue args, void*, IScriptEnvironment* env);

class ColorBars : public IClip {
  VideoInfo vi;
  PVideoFrame frame;
  SFLOAT *audio;
  unsigned nsamples;

  enum { Hz = 440 } ;

public:
  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);

  ColorBars(int w, int h, const char* pixel_type, IScriptEnvironment* env);
  ~ColorBars();
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env) { return frame; };
  bool __stdcall GetParity(int n) { return false; }
  const VideoInfo& __stdcall GetVideoInfo() { return vi; }
  void __stdcall SetCacheHints(int cachehints,size_t frame_range) { };
  void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env);
};

/**********************************************************
 *                         TONE                           *
 **********************************************************/
class SampleGenerator {
public:
  SampleGenerator() {}
  virtual SFLOAT getValueAt(double where) {return 0.0f;}
};

class SineGenerator : public SampleGenerator {
public:
  SineGenerator() {}
  SFLOAT getValueAt(double where) {return sinf(PI * where* 2.0);}
};


class NoiseGenerator : public SampleGenerator {
public:
  NoiseGenerator() {
    srand( (unsigned)time( NULL ) );
  }

  SFLOAT getValueAt(double where) {return (float) rand()*(2.0f/RAND_MAX) -1.0f;}
};

class SquareGenerator : public SampleGenerator {
public:
  SquareGenerator() {}

  SFLOAT getValueAt(double where) {
    if (where<=0.5) {
      return 1.0f;
    } else {
      return -1.0f;
    }
  }
};

class TriangleGenerator : public SampleGenerator {
public:
  TriangleGenerator() {}

  SFLOAT getValueAt(double where) {
    if (where<=0.25) {
      return (where*4.0);
    } else if (where<=0.75) {
      return ((-4.0*(where-0.50)));
    } else {
      return ((4.0*(where-1.00)));
    }
  }
};

class SawtoothGenerator : public SampleGenerator {
public:
  SawtoothGenerator() {}

  SFLOAT getValueAt(double where) {
    return 2.0*(where-0.5);
  }
};

class Tone : public IClip {
  VideoInfo vi;
  SampleGenerator *s;
  const double freq;            // Frequency in Hz
  const double samplerate;      // Samples per second
  const int ch;                 // Number of channels
  const double add_per_sample;  // How much should we add per sample in seconds
  const float level;

public:
  static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env);

  Tone(float _length, double _freq, int _samplerate, int _ch, const char* _type, float _level, IScriptEnvironment* env);
  ~Tone();
  
  void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env);
  PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env) { return NULL; }
  const VideoInfo& __stdcall GetVideoInfo() { return vi; }
  bool __stdcall GetParity(int n) { return false; }
  void __stdcall SetCacheHints(int cachehints,size_t frame_range) { };
};

}; // namespace avxsynth

#endif // __SOURCE_PRIVATE_H__
