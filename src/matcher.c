#include "matcher.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 初始化匹配器
MatcherState* matcher_init(char** dictionary, int dictionary_size) {
    if (dictionary == NULL || dictionary_size <= 0) {
        return NULL;
    }

    MatcherState* ms = (MatcherState*)malloc(sizeof(MatcherState));
    if (ms == NULL) {
        return NULL;
    }

    // 初始化缓冲区
    ms->buffer_size = BUFFER_SIZE;
    ms->buffer = (char*)malloc(ms->buffer_size * sizeof(char));
    if (ms->buffer == NULL) {
        free(ms);
        return NULL;
    }

    // 初始化缓冲区状态
    ms->buffer_pos = 0;
    ms->buffer_len = 0;

    // 保存字典
    ms->dictionary = dictionary;
    ms->dictionary_size = dictionary_size;

    // 初始化匹配计数
    ms->match_counts = (int*)calloc(dictionary_size, sizeof(int));
    if (ms->match_counts == NULL) {
        free(ms->buffer);
        free(ms);
        return NULL;
    }

    ms->total_count = 0;
    ms->max_match_count = MAX_MATCH_COUNT;
    ms->min_search_index = 0;  // 从缓冲区开头搜索

    return ms;
}

// 处理新生成的字母
int matcher_process_letter(MatcherState* ms, char letter, char* matched_word) {
    if (ms == NULL || matched_word == NULL) {
        return 0;
    }

    // 1. 将字母添加到缓冲区
    ms->buffer[ms->buffer_pos] = letter;
    ms->buffer_pos = (ms->buffer_pos + 1) % ms->buffer_size;

    // 更新有效数据长度
    if (ms->buffer_len < ms->buffer_size) {
        ms->buffer_len++;
    }

    // 2. 在缓冲区中搜索字典单词
    for (int i = 0; i < ms->dictionary_size; i++) {
        const char* word = ms->dictionary[i];
        if (word == NULL) {
            continue;
        }

        int word_len = strlen(word);
        if (word_len == 0 || word_len > ms->buffer_len) {
            continue;
        }

        // 从 min_search_index 开始搜索，避免重复匹配旧位置
        for (int j = ms->min_search_index; j <= ms->buffer_len - word_len; j++) {
            int match = 1;

            // 检查单词是否匹配
            for (int k = 0; k < word_len; k++) {
                // 计算缓冲区中的位置
                int buffer_index = (ms->buffer_pos - ms->buffer_len + j + k + ms->buffer_size) % ms->buffer_size;

                if (ms->buffer[buffer_index] != word[k]) {
                    match = 0;
                    break;
                }
            }

            // 如果匹配成功
            if (match) {
                // 更新最小搜索索引，跳过已匹配的位置
                ms->min_search_index = j + word_len;
                // 不要重置 min_search_index，保持它继续增长


                ms->match_counts[i]++;
                ms->total_count++;
                strcpy(matched_word, word);
                return 1;  // 匹配成功
            }
        }
    }

    return 0;  // 未匹配
}

// 检查是否应该结束
int matcher_should_end(const MatcherState* ms) {
    if (ms == NULL) {
        return 0;
    }

    // 检查是否有单词匹配次数达到阈值
    for (int i = 0; i < ms->dictionary_size; i++) {
        if (ms->match_counts[i] >= ms->max_match_count) {
            return 1;
        }
    }

    return 0;
}

// 获取总匹配次数
int matcher_get_total_count(const MatcherState* ms) {
    if (ms == NULL) {
        return 0;
    }
    return ms->total_count;
}

// 释放匹配器
void matcher_free(MatcherState* ms) {
    if (ms == NULL) {
        return;
    }

    if (ms->buffer != NULL) {
        free(ms->buffer);
    }

    if (ms->match_counts != NULL) {
        free(ms->match_counts);
    }

    free(ms);
}
