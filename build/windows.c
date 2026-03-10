#include <stdio.h>
#include <stdlib.h>

int main() {
  int status = system("x86_64-w64-mingw32-gcc -Wall -Wextra ../src/main.c -o ./out/win.exe");
  if (status < 0) {
    perror("System");
    exit(EXIT_FAILURE);
  }

  printf("\n================\nBUILD COMPLETED!\n================\n");
  return 0;
}
