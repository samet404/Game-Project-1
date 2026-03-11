#include <stdbool.h>
#include "./build.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

void printCmdTime(char *name, struct timeval startTime) {
  struct timeval endTime;
  gettimeofday(&endTime, NULL);
  
  __time_t microSDiff = endTime.tv_usec - startTime.tv_usec;
    printf("\"%s\" COMMAND TOOK: %ld second || %ld millisecond || %ld microsecond\n", name,  endTime.tv_sec - startTime.tv_sec, microSDiff / 1000, microSDiff);
}

void cmd(char *name, int *status, char *cmd) {
  struct timeval startTime;
  gettimeofday(&startTime, NULL);
  FILE *fp;
  char *result;

  fp = popen(cmd, "r");
  if (fp == NULL) {
    fprintf(stderr, "cmd(): Couldn't open FILE '%s': %s\n", name, strerror(errno));
    printCmdTime(name, startTime);
    exit(EXIT_FAILURE);
  }

  while (fscanf(fp, NULL, &result) == 1) {
    printf("%s", result);
  }

  int closeStatus = pclose(fp);
  if (closeStatus == -1) {
    fprintf(stderr, "cmd(): pclose error '%s': %s\n", name, strerror(errno));
    printCmdTime(name, startTime);
    exit(EXIT_FAILURE);
  }
 
  printf("%s status: %d\n", name, closeStatus);
  printCmdTime(name, startTime);
  *status = closeStatus;
}

void cmd_infoOnError(char *name, int status) {
  if (status > 0) {
    printf("cmd_ioe - %s - status: %d\n", name, status);
    if (WIFEXITED(status)) {
      printf("cmd_ioe - %s -  process terminated normally\n", name);
      int exitStatus = WEXITSTATUS(status);
      printf("cmd_ioe - %s - exit status: %d\n", name, exitStatus);
    }

    if (WIFSIGNALED(status)) {
      printf("cmd_ioe - %s - process was terminated by signal\n", name);
      int exitSignal = WTERMSIG(status);
      printf("cmd_ioe - %s -  signal: %d\n", name, exitSignal);
    }
    
    if (WIFSTOPPED(status)) {
      printf("cmd_ioe - %s - process was stopped by delivery of a signal \n", name);
      int stopSig = WSTOPSIG(status);
      printf("cmd_ioe - %s - signal: %d\n", name, stopSig);
    }
  }
}

void cmd_exitOnError(char *name, int status) {
  if (status > 0) {
    printf("BUILD FAILED!! %s was unsuccessfull", name);
    exit(EXIT_FAILURE);
  }
}

bool cmd_isFailed(int status) {
  if (status > 0) return true;
  else return false;
}
