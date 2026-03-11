#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "./build.h"
#include <sys/time.h>


#define LINUX_COMMON_FLAGS "-Wall -Wextra"
#define WIN_COMMON_FLAGS "-Wall -Wextra"

#define LINUX_COMPILER "clang"
#define WIN_COMPILER "x86_64-w64-mingw32-gcc"

#define SRC_DIR "../src/"

enum Target {
  WIN,
  LINUX
};
char targets[][8] = {"windows", "linux"};

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Please provide target: --win or --linux");
    exit(EXIT_FAILURE);
  }

  enum Target target;
  char *compiler;
  char *commonFlags;

  if (!strcmp(argv[1], "--linux")) {
    target = LINUX;
    compiler = LINUX_COMPILER;
    commonFlags = LINUX_COMMON_FLAGS;
  } else if (!strcmp(argv[1], "--win")) {
    target = WIN;
    compiler = WIN_COMPILER;
    commonFlags = WIN_COMMON_FLAGS;
  } else {
    printf("INVALID TARGET SPECIFIED. ONLY USE: --linux or --win");
    exit(EXIT_FAILURE);
  }

  printf("TARGET SELECTED: %s\n", targets[target]);

  // =========================

  struct timeval startTime;
  gettimeofday(&startTime, NULL);

  {
    char *name = "main";
    int status = 0;
    if (target == WIN) cmd(name, &status, WIN_COMPILER" "WIN_COMMON_FLAGS" "SRC_DIR"main.c -o ./out/win.exe");
    else if (target == LINUX) cmd(name, &status, LINUX_COMPILER" "LINUX_COMMON_FLAGS" "SRC_DIR"/main.c -o ./out/linux.o");

    cmd_infoOnError(name, status);
    cmd_exitOnError(name, status);
  }
  

  struct timeval endTime;
  gettimeofday(&endTime, NULL);
  
  __time_t microSdiff = endTime.tv_usec - startTime.tv_usec;

  printf("TOTAL BUILD TIME: %ld second || %ld millisecond || %ld microsecond", endTime.tv_sec - startTime.tv_sec, microSdiff / 1000, microSdiff);
  printf("\n================\nBUILD COMPLETED!\n================\n");
  return 0;
}
