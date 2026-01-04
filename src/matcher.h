#ifndef GACHA_MATCHER_H
#define GACHA_MATCHER_H

#include <stddef.h>

// 匹配状态
typedef struct {
    char* buffer;            // 缓冲区（存储最近生成的字母）
    int buffer_size;         // 缓冲区大小
    int buffer_pos;          // 当前位置
    int buffer_len;          // 当前缓冲区中有效数据长度

    char** dictionary;       // 字典
    int dictionary_size;     // 字典大小

    int* match_counts;       // 各单词匹配次数
    int total_count;         // 总匹配次数

    int max_match_count;     // 最大匹配次数（结束条件）

    int min_search_index;    // 最小搜索索引（避免重复匹配旧位置）
} MatcherState;

// 默认配置
#define BUFFER_SIZE 256      // 缓冲区大小
#define MAX_MATCH_COUNT 3    // 最大匹配次数

// 核心函数

// 初始化匹配器
MatcherState* matcher_init(char** dictionary, int dictionary_size);

// 处理新生成的字母
int matcher_process_letter(MatcherState* ms, char letter, char* matched_word);

// 检查是否应该结束
int matcher_should_end(const MatcherState* ms);

// 获取总匹配次数
int matcher_get_total_count(const MatcherState* ms);

// 获取历史总匹配次数（本次运行之前）
int matcher_get_history_total_count(const MatcherState* ms);

// 释放匹配器
void matcher_free(MatcherState* ms);

#endif // GACHA_MATCHER_H
