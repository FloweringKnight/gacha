#ifndef GACHA_GACHA_H
#define GACHA_GACHA_H

#include "list.h"
#include "random.h"

// 抽取结果
typedef struct {
    char* name;                // 菜名
    char* rank;                // 等级
} GachaResult;

// gacha 模块状态
typedef struct {
    GachaList* list;          // gachalist 数据
    RandomGenerator* rng;     // 随机数生成器
    int total_draws;          // 总抽取次数
    int rank_counts[5];       // 各等级抽取次数 [N,R,SR,SSR,UR]
    int balance;              // 抽卡余额（历史总匹配次数）
    int initialized;          // 是否已初始化
} GachaState;

// 核心函数

// 初始化 gacha 模块
GachaState* gacha_init(const char* gachalist_path, int balance);

// 从 gachalist 中随机抽取一个
GachaResult gacha_draw(GachaState* state);

// 批量抽取
GachaResult* gacha_draw_multiple(GachaState* state, int count, int* actual_count);

// 检查余额是否足够
int gacha_check_balance(GachaState* state, int requested_count);

// 确认是否继续抽卡（余额不足时）
int gacha_confirm_continue(int balance);

// 获取抽取统计
void gacha_get_stats(const GachaState* state, int* rank_counts);

// 释放 gacha 状态
void gacha_free(GachaState* state);

// 释放抽取结果
void gacha_result_free(GachaResult* result);

// 输出余额信息
void gacha_output_balance(int balance);

// 输出剩余余额
void gacha_output_remaining_balance(int balance);

// 输出抽取结果
void gacha_output_result(const GachaResult* result);

// 输出统计信息
void gacha_output_stats(const GachaState* state);

#endif // GACHA_GACHA_H
