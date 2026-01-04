#ifndef GACHA_RANDOM_H
#define GACHA_RANDOM_H

// 随机生成器状态
typedef struct {
    unsigned int seed;       // 随机种子
    const char* charset;     // 字符集 [a-zA-Z]
    int charset_size;        // 字符集大小
} RandomGenerator;

// 字符集定义
#define CHARSET "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define CHARSET_SIZE 52

// 核心函数

// 初始化随机生成器
RandomGenerator* random_generator_init();

// 生成下一个随机字母
char generate_random_letter(RandomGenerator* rg);

// 释放随机生成器
void random_generator_free(RandomGenerator* rg);

// 延迟函数（跨平台）
void sleep_ms(int milliseconds);

#endif // GACHA_RANDOM_H
