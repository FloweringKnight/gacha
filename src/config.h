#ifndef GACHA_CONFIG_H
#define GACHA_CONFIG_H

#include <stddef.h>

// 配置结构体
typedef struct {
    int letters_per_second;    // 每秒生成字母数
    char** dictionary;          // 字典单词数组
    int dictionary_size;        // 字典单词数量
    int history_total_count;    // 历史总匹配次数
} GachaConfig;

// 默认配置宏
#define DEFAULT_LETTERS_PER_SECOND 2
#define DEFAULT_DICTIONARY_SIZE 2
#define DEFAULT_HISTORY_TOTAL_COUNT 0

// 核心函数

// 获取配置文件路径
char* get_config_path();

// 检查配置文件是否存在
int config_file_exists(const char* path);

// 创建默认配置文件
int create_default_config(const char* path);

// 解析配置文件
GachaConfig* parse_config(const char* path);

// 获取默认配置
GachaConfig* get_default_config();

// 保存配置文件
int save_config(const char* path, GachaConfig* config);

// 释放配置内存
void free_config(GachaConfig* config);

#endif // GACHA_CONFIG_H
