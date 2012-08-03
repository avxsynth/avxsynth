#include <stdio.h>
#include <avxsynth_c.h>

// The purpose of this program is to check consistency
// between the size of AVS_Value (C) and AVSValue (C++)

int main() {
AVS_ScriptEnvironment* env;
AVS_Value args[3] = {
	avs_void,
	avs_new_value_string("Rec601"),
	avs_new_value_bool(0)};
AVS_Value ret;

env = avs_create_script_environment(3);
args[0] = avs_invoke(env, "blankclip", avs_void, NULL);
ret = avs_invoke(env, "converttorgb24", avs_new_value_array(args, 3), NULL);
if ( avs_is_error(ret) ) {
	printf("%s\n", "error");
	return -1;
}

avs_delete_script_environment(env);

return 0;
}
