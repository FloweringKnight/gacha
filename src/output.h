#ifndef GACHA_OUTPUT_H
#define GACHA_OUTPUT_H

// 输出状态
typedef struct {
    int bold_enabled;        // 是否启用加粗
} OutputState;

// ANSI 转义码
#define ANSI_BOLD "\033[1m"
#define ANSI_RESET "\033[0m"

// 核心函数

// 初始化输出
OutputState* output_init();

// 输出字母
void output_letter(OutputState* os, char letter);

// 输出匹配的单词（加粗）
void output_matched_word(OutputState* os, const char* word);

// 输出换行
void output_newline();

// 输出最终统计
void output_final_count(int count);

// 输出历史总匹配次数
void output_history_total_count(int history_count);

// 启用终端加粗（跨平台）
void enable_terminal_bold();

// 重置终端样式
void reset_terminal_style();

// 释放输出状态
void output_free(OutputState* os);

#endif // GACHA_OUTPUT_H
