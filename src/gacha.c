#include "gacha.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 等级到索引的映射
static int rank_to_index(const char* rank) {
    if (strcmp(rank, "N") == 0) return 0;
    if (strcmp(rank, "R") == 0) return 1;
    if (strcmp(rank, "SR") == 0) return 2;
    if (strcmp(rank, "SSR") == 0) return 3;
    if (strcmp(rank, "UR") == 0) return 4;
    return -1;
}

// 初始化 gacha 模块
GachaState* gacha_init(const char* gachalist_path, int balance) {
    if (gachalist_path == NULL) {
        return NULL;
    }

    GachaState* state = (GachaState*)malloc(sizeof(GachaState));
    if (state == NULL) {
        return NULL;
    }

    // 初始化随机数生成器
    state->rng = random_generator_init();
    if (state->rng == NULL) {
        free(state);
        return NULL;
    }

    // 读取 gachalist
    state->list = read_gachalist(gachalist_path);
    if (state->list == NULL || state->list->size == 0) {
        state->list = get_default_gachalist();
    }

    if (state->list == NULL || state->list->size == 0) {
        random_generator_free(state->rng);
        free(state);
        return NULL;
    }

    // 初始化状态
    state->total_draws = 0;
    memset(state->rank_counts, 0, sizeof(state->rank_counts));
    state->balance = balance;
    state->initialized = 1;

    return state;
}

// 从 gachalist 中随机抽取一个
GachaResult gacha_draw(GachaState* state) {
    GachaResult result = { NULL, NULL };

    if (state == NULL || !state->initialized || state->list == NULL || state->list->size == 0) {
        return result;
    }

    // 检查余额
    if (state->balance <= 0) {
        return result;
    }

    // 生成随机索引
    int index = rand() % state->list->size;

    // 创建抽取结果
    result.name = strdup(state->list->items[index].name);
    result.rank = strdup(state->list->items[index].rank);

    // 更新统计
    state->total_draws++;
    state->balance--;

    // 更新等级计数
    int rank_index = rank_to_index(result.rank);
    if (rank_index >= 0 && rank_index < 5) {
        state->rank_counts[rank_index]++;
    }

    return result;
}

// 批量抽取
GachaResult* gacha_draw_multiple(GachaState* state, int count, int* actual_count) {
    if (state == NULL || !state->initialized || actual_count == NULL) {
        return NULL;
    }

    // 验证参数
    if (count < 1) count = 1;
    if (count > 1000) count = 1000;

    // 分配结果数组
    GachaResult* results = (GachaResult*)malloc(sizeof(GachaResult) * count);
    if (results == NULL) {
        return NULL;
    }

    // 执行抽取
    int drawn = 0;
    for (int i = 0; i < count; i++) {
        GachaResult result = gacha_draw(state);
        if (result.name != NULL) {
            results[i] = result;
            drawn++;
        } else {
            // 余额不足，停止抽取
            break;
        }
    }

    *actual_count = drawn;
    return results;
}

// 检查余额是否足够
int gacha_check_balance(GachaState* state, int requested_count) {
    if (state == NULL) {
        return 0;
    }
    return state->balance >= requested_count;
}

// 确认是否继续抽卡（余额不足时）
int gacha_confirm_continue(int balance) {
    printf("当前余额仅能进行 【%d】 次抽卡，是否继续？（y/n）", balance);

    char input[10];
    if (fgets(input, sizeof(input), stdin) == NULL) {
        return 0;  // 读取失败，不继续
    }

    // 移除换行符
    input[strcspn(input, "\r\n")] = '\0';

    return (strcmp(input, "y") == 0 || strcmp(input, "Y") == 0);
}

// 获取抽取统计
void gacha_get_stats(const GachaState* state, int* rank_counts) {
    if (state == NULL || rank_counts == NULL) {
        return;
    }

    memcpy(rank_counts, state->rank_counts, sizeof(state->rank_counts));
}

// 释放 gacha 状态
void gacha_free(GachaState* state) {
    if (state == NULL) {
        return;
    }

    if (state->rng != NULL) {
        random_generator_free(state->rng);
    }

    if (state->list != NULL) {
        free_gachalist(state->list);
    }

    free(state);
}

// 释放抽取结果
void gacha_result_free(GachaResult* result) {
    if (result == NULL) {
        return;
    }

    if (result->name != NULL) {
        free(result->name);
    }

    if (result->rank != NULL) {
        free(result->rank);
    }

    result->name = NULL;
    result->rank = NULL;
}

// 输出余额信息
void gacha_output_balance(int balance) {
    printf("当前抽卡余额：%d 次\n", balance);
}

// 输出剩余余额
void gacha_output_remaining_balance(int balance) {
    printf("剩余抽卡余额：%d 次\n", balance);
}

// 输出抽取结果
void gacha_output_result(const GachaResult* result) {
    if (result == NULL || result->name == NULL) {
        return;
    }

    if (result->rank != NULL) {
        printf("【%s】%s\n", result->rank, result->name);
    } else {
        printf("%s\n", result->name);
    }
}

// 输出统计信息
void gacha_output_stats(const GachaState* state) {
    if (state == NULL) {
        return;
    }

    const char* rank_names[] = { "N", "R", "SR", "SSR", "UR" };

    printf("\n本次抽取 %d 次：\n", state->total_draws);
    for (int i = 0; i < 5; i++) {
        printf("【%s】%d 次\n", rank_names[i], state->rank_counts[i]);
    }
}
