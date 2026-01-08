#include <stdio.h>
#include <string.h>
#include "src/config.h"

void show_version() {
    printf("%s\n", PROGRAM_VERSION);
}

int main(int argc, char* argv[])
{
    // 检查版本参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            show_version();
            return 0;
        }
    }

    printf("Hello, World!\n");
    return 0;
}