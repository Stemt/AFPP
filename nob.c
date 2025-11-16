#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD_DIR "build/"

int main(int argc, char** argv){
  NOB_GO_REBUILD_URSELF(argc, argv);

  Nob_Cmd cmd = {0};
  nob_cc(&cmd);
  nob_cc_flags(&cmd);
  nob_cmd_append(&cmd, "-g");
  nob_cc_inputs(&cmd, "./afpp.c");
  nob_cc_output(&cmd, "./afpp");

  if(!nob_cmd_run(&cmd)) return 1;

  const char* source = "example.c";

  nob_cmd_append(&cmd, "./afpp");
  nob_cmd_append(&cmd, "-d", BUILD_DIR);
  nob_cmd_append(&cmd, source);
  
  if(!nob_cmd_run(&cmd)) return 1;

  nob_cc(&cmd);
  nob_cc_flags(&cmd);
  nob_cc_inputs(&cmd, nob_temp_sprintf("./"BUILD_DIR"%s.afpp.c", source));
  nob_cc_output(&cmd, "./app");
  
  if(!nob_cmd_run(&cmd)) return 1;

  return 0;

}
