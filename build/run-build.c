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

enum Target {
  WIN,
  LINUX
};
char targets[][8] = {"windows", "linux"};

// ======================

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("ERROR: Couldn't find working directory!");
    exit(EXIT_FAILURE);
  }
  char* absPath = argv[1];
  printf("WORKING DIRECTORY: %s\n", absPath);

  if (argc < 3) {
    printf("Please provide target: --win or --linux");
    exit(EXIT_FAILURE);
  }

  enum Target target;

  if (!strcmp(argv[2], "--linux")) {
    target = LINUX;
  } else if (!strcmp(argv[2], "--win")) {
    target = WIN;
  } else {
    printf("INVALID TARGET SPECIFIED %s\n ONLY USE: --linux or --win", argv[2]);
    exit(EXIT_FAILURE);
  }

  printf("TARGET SELECTED: %s\n", targets[target]);

  // =========================

  struct timeval startTime;
  gettimeofday(&startTime, NULL);

  {
    if (target == WIN) {
      {
        char *name = "main";
        int status = 0;

        char *command;
        asprintf(&command, "%s %s/src/main.c -o %s/build/out/windows/game.exe -v -L%s/deps/raylib-win -I%s/deps/raylib-win/include/ -lraylib.dll -lm -lwinmm -lgdi32 -lopengl32 -Wall -Wextra", WIN_COMPILER, absPath, absPath, absPath, absPath);
        
        cmd(name, &status, command);
        cmd_infoOnError(name, status);
        cmd_exitOnError(name, status);
      }

      {
        char *name = "copy_dll";
        int status = 0;

        char *command;
        asprintf(&command, "cp %s/deps/raylib-win/libraylib.dll %s/build/out/windows/libraylib.dll", absPath, absPath);
        
        cmd(name, &status, command);
        cmd_infoOnError(name, status);
        cmd_exitOnError(name, status);
      }
    } else if (target == LINUX) {
      {
        int status = 0;
        char *name = "delete_prev_build";
        char *command;
        asprintf(&command, "rm -rf %s/build/out/linux/linux.out" , absPath);

        cmd(name, &status, command);
        
        cmd_infoOnError(name, status);
        cmd_exitOnError(name, status);
      }

      {
        char *name = "main";
        int status = 0;
        
        char *command;
        asprintf(&command, "%s -lraylib -lm -v -L%s/deps/raylib-linux -Wl,-rpath=%s/deps/raylib-linux -I%s/deps/raylib-linux/include/ -Wall -Wextra %s/src/main.c -o %s/build/out/linux/linux.out" , LINUX_COMPILER, absPath, absPath, absPath, absPath, absPath);

        cmd(name, &status, command);
        
        cmd_infoOnError(name, status);
        cmd_exitOnError(name, status);
      }
    }
  }
  

  struct timeval endTime;
  gettimeofday(&endTime, NULL);
  
  __time_t microSdiff = endTime.tv_usec - startTime.tv_usec;

  printf("TOTAL BUILD TIME: %ld second || %ld millisecond || %ld microsecond", endTime.tv_sec - startTime.tv_sec, microSdiff / 1000, microSdiff);
  printf("\n================\nBUILD COMPLETED!\n================\n");
  return 0;
}
