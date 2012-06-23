#include "../../../avxsynth/core/src/core/avxsynth_c.h"
#include <stdio.h>

int main() {
	printf("1\n");
	AVS_ScriptEnvironment *env = avs_create_script_environment(2);
	printf("2\n");
	avs_delete_script_environment(env);
	printf("3\n");
	return 0;
}
