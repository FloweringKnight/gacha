#ifndef GACHA_LIST_H
#define GACHA_LIST_H

#include <stddef.h>

// gachalist 条目结构体
typedef struct {
    char* name;                // 菜名
    char* rank;                // 等级（N/R/SR/SSR/UR）
} GachaItem;

// gachalist 结构体
typedef struct {
    GachaItem* items;          // 菜名数组
    int size;                  // 菜名数量
    char* file_path;           // 文件路径
} GachaList;

// 默认菜名数据结构
typedef struct {
    const char* name;
    const char* rank;
} DefaultGachaItem;

// 核心函数

// 获取 gachalist 文件路径
char* get_gachalist_path();

// 检查 gachalist 是否存在
int gachalist_file_exists();

// 创建默认 gachalist 文件
int create_default_gachalist(const char* path);

// 读取 gachalist 文件
GachaList* read_gachalist(const char* path);

// 获取内置默认 gachalist（600 道菜）
GachaList* get_default_gachalist();

// 释放 gachalist 内存
void free_gachalist(GachaList* list);

// 验证等级格式
int validate_rank(const char* rank);

#endif // GACHA_LIST_H
