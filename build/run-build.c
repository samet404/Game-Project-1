#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "./build.h"
#include <sys/time.h>
#include <linux/limits.h>

#define LINUX_COMPILER "clang"
#define WIN_COMPILER "x86_64-w64-mingw32-gcc"

// ======================

int main(int argc, char **argv) {
  // ===============================
  // PARSE THE ARGUMENTS
  // ==============================
  
  char target = -1;
  char* absPath;
  bool debug = false;
  
  if (argc < 2) {
    printf("ERROR: Couldn't find working directory!");
    exit(EXIT_FAILURE);
  }
  
  if (argc < 3) {
    printf("Please provide target: --win or --linux");
    exit(EXIT_FAILURE);
  }
  
  absPath = argv[1];
  printf("WORKING DIRECTORY: %s\n", absPath);

  // ignore the args belongs to index 0, 1 which are comes from start-build.sh
  for (unsigned char i = 2; i < argc; ++i) {
    if (!strcmp(argv[i], "--linux")) {
      target = 0;
      continue;
    }

    if (!strcmp(argv[i], "--win")) {
      target = 1;
      continue;
    }
    
    if (!strcmp(argv[i], "--debug")) {
      debug = true;
      printf("[DEBUG MODE ENABLED]\n");
      continue;
    }
    // TODO: add --help flag
  }

  if (target == -1) {
    printf("NO TARGET SPECIFIED\nUSE: --linux or --win");
    exit(EXIT_FAILURE);
  }


  // ========================
  // MAIN PROGRAM
  // ========================
  struct timeval startTime;
  gettimeofday(&startTime, NULL);

  { // SCOPE OF ALL BUILD STAGE  
    printf("[BUILD STARTED]\n");

      printf("a");
    // ======================
    // WINDOWS
    // ======================
    if (target == 1) {
      {
        char *name = "main";
        int status = 0;

        char *cmd_str = NULL;
        cmd_append(&cmd_str, WIN_COMPILER);
        cmd_append(&cmd_str, "%s/src/main.c -o %s/build/out/windows/game.exe", absPath, absPath);
        // common flags
        cmd_append(&cmd_str, "-v -Wall -Wextra");
        // enable/disable debug output, decide optimization level
        cmd_append(&cmd_str, "%s", debug ? "-O0 -g" : "-O3");
        // link with raylib
        cmd_append(&cmd_str, "-L%s/deps/raylib-win -I%s/deps/raylib-win/include/ -lraylib.dll", absPath, absPath);
        // required libraries
        cmd_append(&cmd_str, "-lm -lwinmm -lgdi32 -lopengl32");
            
        cmd(name, &status, cmd_str);
        cmd_infoOnError(name, status);
        cmd_exitOnError(name, status);
      }

      {
        char *name = "copy_dll";
        int status = 0;

        char *cmd_str = NULL;
        cmd_append(&cmd_str, "cp %s/deps/raylib-win/libraylib.dll %s/build/out/windows/libraylib.dll", absPath, absPath);
        
        cmd(name, &status, cmd_str);
        cmd_infoOnError(name, status);
        cmd_exitOnError(name, status);
      }
    }

    // ======================
    // LINUX
    // ======================
    else if (target == 0) {
      {
        int status = 0;
        char *name = "delete_prev_build";
        char *cmd_str = NULL;
        cmd_append(&cmd_str, "rm -rf %s/build/out/linux/linux.out" , absPath);

        cmd(name, &status, cmd_str);
        
        cmd_infoOnError(name, status);
        cmd_exitOnError(name, status);
      }

      {
        char *name = "main";
        int status = 0;
        
        char *cmd_str = NULL;
        cmd_append(&cmd_str, LINUX_COMPILER);
        cmd_append(&cmd_str, "-v -Wall -Wextra");
        // enable/disable debug output, decide optimization level
        cmd_append(&cmd_str, "%s", debug ? "-O0 -g" : "-O3");
        cmd_append(&cmd_str, "-lm");
        // Link against raylib library
        cmd_append(&cmd_str, "-lraylib -L%s/deps/raylib-linux -Wl,-rpath=%s/deps/raylib-linux -I%s/deps/raylib-linux/include/", absPath, absPath, absPath);
        // input/output
        cmd_append(&cmd_str, "%s/src/main.c -o %s/build/out/linux/linux.out", absPath, absPath);

        cmd(name, &status, cmd_str);
        
        cmd_infoOnError(name, status);
        cmd_exitOnError(name, status);
      }
    }
  }

  { // Print total build time
    struct timeval endTime;
    gettimeofday(&endTime, NULL);
    
    double elapsedTime;

    // Seconds to milliseconds
    elapsedTime = (endTime.tv_sec - startTime.tv_sec) * 1000.0;
    // Microseconds to milliseconds
    elapsedTime += (endTime.tv_usec - startTime.tv_usec) / 1000.0;

    printf("[TOTAL BUILD TIME] %f ms.\n", elapsedTime);
  }
  
  printf("\n================\nBUILD COMPLETED!\n================\n");
  return 0;
}
