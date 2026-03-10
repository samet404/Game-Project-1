#include <stdio.h>
#include <stdlib.h>

int main() {
  int status = system("clang -Wall -Wextra ../src/main.c -o ./out/linux.o");
  if (status < 0) {
    perror("System");
    exit(EXIT_FAILURE);
  }

  printf("\n================\nBUILD COMPLETED!\n================\n");
  return 0;
}
