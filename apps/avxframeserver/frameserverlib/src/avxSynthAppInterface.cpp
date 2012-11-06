// Copyright (c) 2012 Anne Aaron, Milan Stevanovic, Pradip Gajjar.  All rights reserved.
// avxanne@gmail.com, avxsynth@gmail.com, pradip.gajjar@gmail.com
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
// import and export plugins, or graphical user interfaces.auto

#include <dlfcn.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "avxlog.h"
#include "avxplugin.h"

#include "frameserverlib.h"

namespace avxsynth
{

#ifdef MODULE_NAME
#undef MODULE_NAME
#endif

#define MODULE_NAME appInterface
#ifdef __APPLE__
  #define AVXSYNTH_LIBRARY "libavxsynth.dylib"
#else
  #define AVXSYNTH_LIBRARY "libavxsynth.so"
#endif
#define MPLAYER "mplayer"

typedef IScriptEnvironment *(*CreateScriptEnvironmentFunc)(int);

struct AvxLibrary {
	void *hAvxsynth;
	IScriptEnvironment *env;
	CreateScriptEnvironmentFunc CreateScriptEnvironment;
};

class AvxContext {
private:
	PClip clip;
	VideoInfo vi;
	const char *scriptName;
	bool launchMPlayer;

	int MPlayerCommandVideo(char *command);
	int MPlayerCommandAudio(char *command);
	int OutputVideo();
	int OutputAudio();
public:
	AvxContext(const char* scriptName, bool launchMPlayer);
	int OpenFile();
	int Output();
};

static AvxLibrary avx_library;

void AvxLibraryClose() {
	if (avx_library.hAvxsynth) {
		if (avx_library.env);
			delete avx_library.env;
		dlclose(avx_library.hAvxsynth);
	}
	avx_library.hAvxsynth = NULL;
	avx_library.env = NULL;
	avx_library.CreateScriptEnvironment = NULL;
}

int AvxLibraryOpen() {
	avx_library.hAvxsynth = dlopen(AVXSYNTH_LIBRARY, RTLD_NOW | RTLD_GLOBAL);
	if (!avx_library.hAvxsynth) {
		AVXLOG_ERROR("Error opening Avxsynth library: (dlopen) %s", dlerror());
		return -1;
	}

	avx_library.CreateScriptEnvironment =
		(CreateScriptEnvironmentFunc)dlsym(avx_library.hAvxsynth, "CreateScriptEnvironment");
	if (!avx_library.CreateScriptEnvironment) {
		AVXLOG_ERROR("Error locating entry point: (dlsym) %s", dlerror());
		goto fail;
	}

	try {
		avx_library.env = avx_library.CreateScriptEnvironment(AVISYNTH_INTERFACE_VERSION);
	} catch (AvisynthError &e) {
		AVXLOG_ERROR("AvisynthError: %s", e.msg);
		goto fail;
	}

	return 0;

fail:
	AvxLibraryClose();
	return -1;
}

AvxContext::AvxContext(const char* scriptName, bool launchMPlayer):
		scriptName(scriptName), launchMPlayer(launchMPlayer) {}

int AvxContext::MPlayerCommandVideo(char *command) {
	char format[sizeof("bgr32")];
	bool flipVertical = false;

	if (vi.IsRGB24()) {
		sprintf(format, "bgr24");
		flipVertical = true;
	} else if (vi.IsRGB32()) {
		sprintf(format, "bgr32");
		flipVertical = true;
	} else if (vi.IsYUY2()) {
		sprintf(format, "yuy2");
	} else if (vi.IsYV12()) {
		sprintf(format, "yv12");
	} else {
		AVXLOG_ERROR("%s", "Unsupported colorspace");
		return -1;
	}

	sprintf(command, MPLAYER " %s -demuxer rawvideo -rawvideo w=%d:h=%d:format=%s - 1> /dev/null",
		flipVertical ? "-flip" : "", vi.width, vi.height, format);
	return 0;
}

int AvxContext::MPlayerCommandAudio(char *command) { // This doesn't seem to work on my MPlayer
	if (vi.sample_type == SAMPLE_FLOAT) {
		AVXLOG_ERROR("%s", "Cannot pipe float audio to mplayer");
		return -1;
	}

	sprintf(command, MPLAYER " -demuxer rawaudio -rawaudio channels=%d:rate=%d:samplesize=%d:format=0 - 1> /dev/null",
			vi.nchannels, vi.audio_samples_per_second, vi.BytesPerChannelSample());
	return 0;
}

int AvxContext::OpenFile() {
	try {
		AVSValue ret = avx_library.env->Invoke("Import", scriptName);
		if (!ret.IsClip()) {
			AVXLOG_ERROR("%s", "Script did not return a clip");
			return -1;
		}
		clip = ret.AsClip();
		vi = clip->GetVideoInfo();
	} catch (AvisynthError &e) {
		AVXLOG_ERROR("AvisynthError: %s", e.msg);
		return -1;
	}

	return 0;
}

int AvxContext::OutputVideo() {
	FILE *sink;
	unsigned char *writeBuffer = NULL;
	sig_t old_sigpipe = signal(SIGPIPE, SIG_IGN);

	if (launchMPlayer) {
		char command[1024];
		if (MPlayerCommandVideo(command))
			return -1;
		AVXLOG_INFO("MPlayer command line: %s", command);

		sink = popen(command, "w");
		if (!sink) {
			AVXLOG_ERROR("%s", "Error starting mplayer");
			return -1;
		}
	} else {
		sink = stdout;
	}

	writeBuffer = (unsigned char *)malloc(vi.RowSize() * vi.height);
	if (!writeBuffer) {
		AVXLOG_ERROR("%s", "Unable to allocate memory");
		goto fail;
	}

	try {
		for (int i = 0; i < vi.num_frames; ++i) {
			if (launchMPlayer && (feof(sink) || ferror(sink))) {
				AVXLOG_ERROR("%s", "mplayer process exited");
				break;
			}
			PVideoFrame frame = clip->GetFrame(i, avx_library.env);
			if (vi.IsPlanar()) { // Check plane count in 2.6.
				int planes[] = {PLANAR_Y, PLANAR_V, PLANAR_U};
				for (int j = 0; j < 3; ++j) {
					int plane = planes[j];
					int src_pitch = frame->GetPitch(plane);
					int row_size = frame->GetRowSize(plane);
					int height = frame->GetHeight(plane);
					const unsigned char *srcp = frame->GetReadPtr(plane);

					avx_library.env->BitBlt(writeBuffer, row_size, srcp, src_pitch, row_size, height);
					fwrite(writeBuffer, 1, row_size * height, sink);
				}
			} else {
				int src_pitch = frame->GetPitch();
				int row_size = frame->GetRowSize();
				int height = frame->GetHeight();
				const unsigned char *srcp = frame->GetReadPtr();

				avx_library.env->BitBlt(writeBuffer, row_size, srcp, src_pitch, row_size, height);
				fwrite(writeBuffer, 1, row_size * height, sink);
			}
		}
	} catch (AvisynthError &e) {
		AVXLOG_ERROR("AvisynthError: %s", e.msg);
		goto fail;
	}

	free(writeBuffer);
	if (launchMPlayer)
		pclose(sink);
	signal(SIGPIPE, old_sigpipe);
	return 0;

fail:
	if (writeBuffer)
		free(writeBuffer);
	if (launchMPlayer)
		pclose(sink);
	signal(SIGPIPE, old_sigpipe);
	return -1;
};

int AvxContext::OutputAudio() {
	FILE *sink;
	void *writeBuffer = NULL;
	sig_t old_sigpipe = signal(SIGPIPE, SIG_IGN);

	if (launchMPlayer) {
		char command[1024];
		if (MPlayerCommandAudio(command))
			return -1;
		AVXLOG_INFO("MPlayer command line: %s", command);

		sink = popen(command, "w");
		if (!sink) {
			AVXLOG_ERROR("%s", "Error starting mplayer");
			return -1;
		}
	} else {
		sink = stdout;
	}

	#define AUDIO_SAMPLES 1000
	try {
		writeBuffer = malloc(vi.BytesPerAudioSample() * AUDIO_SAMPLES);
		if (!writeBuffer) {
			AVXLOG_ERROR("%s", "Unable to allocate memory");
			goto fail;
		}
		for (__int64 i = 0; i < vi.num_audio_samples; i += AUDIO_SAMPLES) {
			if (launchMPlayer && (feof(sink) || ferror(sink))) {
				AVXLOG_ERROR("%s", "mplayer process exited");
				break;
			}
			int read_samples;
			if (vi.num_audio_samples - AUDIO_SAMPLES < i)
				read_samples = vi.num_audio_samples - i;
			else
				read_samples = AUDIO_SAMPLES;
			clip->GetAudio(writeBuffer, i, read_samples, avx_library.env);
			fwrite(writeBuffer, vi.BytesPerAudioSample(), read_samples, sink);
		}
	} catch (AvisynthError &e) {
		AVXLOG_ERROR("AvisynthError: %s", e.msg);
		goto fail;
	}
	#undef AUDIO_SAMPLES

	free(writeBuffer);
	if (launchMPlayer)
		pclose(sink);
	signal(SIGPIPE, old_sigpipe);
	return 0;

fail:
	if (writeBuffer)
		free(writeBuffer);
	if (launchMPlayer)
		pclose(sink);
	signal(SIGPIPE, old_sigpipe);
	return -1;
}

int AvxContext::Output() {
	if (vi.HasVideo()) // An Avisynth clip must have at least one stream.
		return OutputVideo();
	else
		return OutputAudio();
}

int _ProcessScript(const char *scriptName, bool isMPlayerLaunchRequired) {
	AvxContext ctx(scriptName, isMPlayerLaunchRequired);
	if (ctx.OpenFile())
		return -1;
	if (ctx.Output())
		return -1;
	return 0;
}

int ProcessScript(const char *scriptName, bool isMPlayerLaunchRequired) {
	int res;

	if (AvxLibraryOpen())
		return -1;

	// Do the actual processing in another stack frame to avoid scope issues.
	res = _ProcessScript(scriptName, isMPlayerLaunchRequired);
	AvxLibraryClose();
	return res;
}

} // namespace avxsynth
