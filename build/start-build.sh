PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
clang -Wall -Wextra  \
-I$PROJECT_DIR/build/build.h \
$PROJECT_DIR/build/build.c  \
$PROJECT_DIR/build/run-build.c \
-o $PROJECT_DIR/build/run-build.o && $PROJECT_DIR/build/run-build.o ${PROJECT_DIR} ${@:1} && rm -rf $PROJECT_DIR/build/run-build.o
