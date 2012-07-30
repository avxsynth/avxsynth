#include <avxsynth_c.h>
#include <stdio.h>

int main() {
	AVS_ScriptEnvironment *env;
	const int EXPECTED_WIDTH = 640;
	const int EXPECTED_HEIGHT = 480;

	fprintf(stderr, "Creating script environment...\n");
	env = avs_create_script_environment(2);

	fprintf(stderr, "Trying to invoke the BlankClip() filter...\n");
	AVS_Value val;
	val = avs_invoke(env, "BlankClip", avs_void, NULL);
	if (avs_is_error(val)) {
		fprintf(stderr, "oh noes: %s\n", avs_as_error(val));
		avs_delete_script_environment(env);
		return 1;
	}
	else { fprintf(stderr, "...it worked\n"); }

	fprintf(stderr, "Trying to get clip dimensions... ");
	int width, height;
	AVS_Clip *clip;
	const AVS_VideoInfo *vi;
	if (avs_is_clip(val)) {
		clip = avs_take_clip(val, env);
		avs_release_value(val);
		vi = avs_get_video_info(clip);
		width = vi->width;
		height = vi->height;
		fprintf(stderr, "%i x %i\n", width, height);
	}
	else {
		fprintf(stderr, "Return value was not a clip\n");
		avs_delete_script_environment(env);
		return 1;
	}

	if ((width != EXPECTED_WIDTH) || (height != EXPECTED_HEIGHT)) {
		fprintf(stderr, "Did not get expected height %i x %i...\n", EXPECTED_WIDTH, EXPECTED_HEIGHT);
		avs_delete_script_environment(env);
		return 1;
	}

	fprintf(stderr, "Deleting script environment...\n");
	avs_delete_script_environment(env);

	fprintf(stderr, "Done\n");
	return 0;
}
