#include "random.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

// 初始化随机生成器
RandomGenerator* random_generator_init() {
    RandomGenerator* rg = (RandomGenerator*)malloc(sizeof(RandomGenerator));
    if (rg == NULL) {
        return NULL;
    }

    rg->seed = (unsigned int)time(NULL);
    rg->charset = CHARSET;
    rg->charset_size = CHARSET_SIZE;

    // 初始化标准随机数生成器
    srand(rg->seed);

    return rg;
}

// 生成下一个随机字母
char generate_random_letter(RandomGenerator* rg) {
    if (rg == NULL) {
        return 'a';
    }

    int index = rand() % rg->charset_size;
    return rg->charset[index];
}

// 释放随机生成器
void random_generator_free(RandomGenerator* rg) {
    if (rg != NULL) {
        free(rg);
    }
}

// 延迟函数（跨平台）
#ifdef _WIN32
void sleep_ms(int milliseconds) {
    Sleep(milliseconds);
}
#else
void sleep_ms(int milliseconds) {
    usleep(milliseconds * 1000);
}
#endif
