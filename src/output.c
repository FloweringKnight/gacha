#include "output.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
#endif

// 初始化输出
OutputState* output_init() {
    OutputState* os = (OutputState*)malloc(sizeof(OutputState));
    if (os == NULL) {
        return NULL;
    }

    os->bold_enabled = 1;  // 默认启用加粗

    // 初始化终端
    enable_terminal_bold();

    return os;
}

// 输出字母
void output_letter(OutputState* os, char letter) {
    (void)os;  // 未使用的参数
    putchar(letter);
    fflush(stdout);
}

// 输出匹配的单词（加粗）
void output_matched_word(OutputState* os, const char* word) {
    if (word == NULL) {
        return;
    }

    if (os && os->bold_enabled) {
        printf("%s%s%s", ANSI_BOLD, word, ANSI_RESET);
    } else {
        printf("%s", word);
    }
    fflush(stdout);
}

// 输出换行
void output_newline() {
    putchar('\n');
    fflush(stdout);
}

// 输出最终统计
void output_final_count(int count) {
    printf("\n匹配总数: %d\n", count);
    fflush(stdout);
}

// 输出历史总匹配次数
void output_history_total_count(int history_count) {
    printf("历史总匹配次数: %d\n", history_count);
    fflush(stdout);
}

// 启用终端加粗（跨平台）
void enable_terminal_bold() {
#ifdef _WIN32
    // 启用 Windows 虚拟终端以支持 ANSI 转义码
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#else
    // Linux/macOS 默认支持 ANSI 转义码
    // 无需特殊处理
#endif
}

// 重置终端样式
void reset_terminal_style() {
    printf("%s", ANSI_RESET);
    fflush(stdout);
}

// 释放输出状态
void output_free(OutputState* os) {
    if (os != NULL) {
        // 重置终端样式
        reset_terminal_style();
        free(os);
    }
}
