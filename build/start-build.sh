clang -Wall -Wextra -I./build.h build.c run-build.c -o run-build.o && ./run-build.o "$@" && rm -rf ./run-build.o
