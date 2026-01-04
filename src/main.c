#include "config.h"
#include "random.h"
#include "matcher.h"
#include "output.h"
#include "gacha.h"
#include "list.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 全局运行标志
volatile sig_atomic_t running = 1;

// 信号处理函数
void signal_handler(int sig) {
    (void)sig;  // 未使用的参数
    running = 0;
}

// 设置信号处理器
void setup_signal_handler() {
    signal(SIGINT, signal_handler);
#ifdef SIGTERM
    signal(SIGTERM, signal_handler);
#endif
}

// 初始化配置文件
void init_config_files() {
    char* config_path = get_config_path();
    char* gachalist_path = get_gachalist_path();

    // 检查并创建 gacha.conf
    if (!config_file_exists(config_path)) {
        printf("配置文件不存在，创建默认配置文件: %s\n", config_path);
        if (create_default_config(config_path) != 0) {
            fprintf(stderr, "警告：无法创建默认配置文件\n");
        }
    }

    // 检查并创建 gachalist
    if (!gachalist_file_exists()) {
        printf("gachalist 文件不存在，创建默认文件: %s\n", gachalist_path);
        if (create_default_gachalist(gachalist_path) != 0) {
            fprintf(stderr, "警告：无法创建默认 gachalist 文件\n");
        }
    }

    free(config_path);
    free(gachalist_path);
}

// 解析抽取次数
int parse_draw_count(const char* str) {
    int count = 0;
    if (sscanf(str, "%d", &count) != 1) {
        return -1;
    }
    return count;
}

// 打印用法
void print_usage() {
    printf("用法：gacha [选项]\n");
    printf("选项：\n");
    printf("  -c              chaos 模式，启动随机字母生成与单词匹配\n");
    printf("  -g [数字]       gacha 模式，从 gachalist 随机抽取内容\n");
    printf("  -h, --help      显示帮助信息\n");
}

// 打印帮助信息
void print_help() {
    printf("gacha - 随机字母生成与单词匹配命令行工具 v2.0\n\n");
    printf("用法：\n");
    printf("  gacha -c              启动 chaos 模式\n");
    printf("  gacha -g [数字]       启动 gacha 模式（默认抽取 1 次）\n");
    printf("  gacha -h              显示帮助信息\n\n");
    printf("chaos 模式：\n");
    printf("  随机生成字母并匹配字典单词\n");
    printf("  每次匹配成功增加 1 次历史总匹配次数\n");
    printf("  历史总匹配次数用于 gacha 模式的抽卡\n\n");
    printf("gacha 模式：\n");
    printf("  从 gachalist 随机抽取菜名\n");
    printf("  每次抽卡消耗 1 次历史总匹配次数\n");
    printf("  当历史总匹配次数为 0 时无法抽卡\n");
    printf("  若请求次数 > 余额，可确认使用剩余次数\n\n");
    printf("示例：\n");
    printf("  gacha -c              启动 chaos 模式\n");
    printf("  gacha -g              抽取 1 次\n");
    printf("  gacha -g 10           抽取 10 次\n\n");
    printf("配置文件位置：\n");
    printf("  Windows: %%APPDATA%%\\gacha\\gacha.conf\n");
    printf("  Linux/macOS: ~/.config/gacha/gacha.conf\n\n");
    printf("gachalist 文件位置：\n");
    printf("  与 gacha.conf 存放在同一目录\n");
}

// 运行 chaos 模式
int run_chaos_mode() {
    // 1. 加载配置
    char* config_path = get_config_path();
    if (config_path == NULL) {
        fprintf(stderr, "错误: 无法获取配置文件路径\n");
        return 1;
    }

    GachaConfig* config = NULL;

    // 检查配置文件是否存在
    if (!config_file_exists(config_path)) {
        // 创建默认配置文件
        printf("配置文件不存在，创建默认配置文件: %s\n", config_path);
        if (create_default_config(config_path) != 0) {
            fprintf(stderr, "警告: 无法创建默认配置文件，使用内置默认配置\n");
            config = get_default_config();
        } else {
            // 解析刚创建的配置文件
            config = parse_config(config_path);
        }
    } else {
        // 解析现有配置文件
        config = parse_config(config_path);
    }

    if (config == NULL) {
        fprintf(stderr, "错误: 无法加载配置\n");
        free(config_path);
        return 1;
    }

    // 2. 初始化各模块
    RandomGenerator* rg = random_generator_init();
    if (rg == NULL) {
        fprintf(stderr, "错误: 无法初始化随机生成器\n");
        free_config(config);
        free(config_path);
        return 1;
    }

    MatcherState* ms = matcher_init(config->dictionary, config->dictionary_size);
    if (ms == NULL) {
        fprintf(stderr, "错误: 无法初始化匹配器\n");
        random_generator_free(rg);
        free_config(config);
        free(config_path);
        return 1;
    }

    OutputState* os = output_init();
    if (os == NULL) {
        fprintf(stderr, "错误: 无法初始化输出模块\n");
        matcher_free(ms);
        random_generator_free(rg);
        free_config(config);
        free(config_path);
        return 1;
    }

    // 3. 设置信号处理
    setup_signal_handler();

    // 4. 主循环
    int delay = 1000 / config->letters_per_second;  // 毫秒

    printf("开始随机生成 (每秒 %d 个字母)\n", config->letters_per_second);
    printf("字典包含 %d 个单词，按 Ctrl+C 停止\n\n", config->dictionary_size);
    fflush(stdout);

    while (running && !matcher_should_end(ms)) {
        // 生成字母
        char letter = generate_random_letter(rg);

        // 输出字母
        output_letter(os, letter);

        // 处理匹配
        char matched_word[256] = {0};
        if (matcher_process_letter(ms, letter, matched_word)) {
            // 匹配成功，输出换行和单词
            output_newline();
            output_matched_word(os, matched_word);
        }

        // 延迟
        sleep_ms(delay);
    }

    // 5. 输出最终统计
    int current_run_count = matcher_get_total_count(ms);
    output_final_count(current_run_count);

    // 6. 更新并保存历史总匹配次数
    config->history_total_count += current_run_count;
    output_history_total_count(config->history_total_count);

    if (save_config(config_path, config) != 0) {
        fprintf(stderr, "警告: 无法保存配置文件\n");
    }

    // 7. 清理资源
    output_free(os);
    matcher_free(ms);
    random_generator_free(rg);
    free_config(config);
    free(config_path);

    return 0;
}

// 运行 gacha 模式
int run_gacha_mode(int draw_count) {
    // 1. 加载配置文件获取历史总匹配次数
    char* config_path = get_config_path();
    GachaConfig* config = parse_config(config_path);
    if (config == NULL) {
        fprintf(stderr, "错误: 无法加载配置文件\n");
        free(config_path);
        return 1;
    }

    int balance = config->history_total_count;

    // 2. 检查余额
    if (balance == 0) {
        printf("剩余抽卡次数为 0\n");
        free_config(config);
        free(config_path);
        return 0;
    }

    // 3. 加载 gachalist
    char* gachalist_path = get_gachalist_path();
    GachaList* list = read_gachalist(gachalist_path);

    if (list == NULL || list->size == 0) {
        fprintf(stderr, "错误: gachalist 为空或无法读取，使用内置默认列表\n");
        list = get_default_gachalist();
    }

    if (list == NULL || list->size == 0) {
        fprintf(stderr, "错误: 无法加载 gachalist\n");
        free_config(config);
        free(config_path);
        free(gachalist_path);
        return 1;
    }

    // 4. 初始化 gacha 模块
    GachaState* state = gacha_init(gachalist_path, balance);
    if (state == NULL) {
        fprintf(stderr, "错误: 无法初始化 gacha 模块\n");
        free_gachalist(list);
        free_config(config);
        free(config_path);
        free(gachalist_path);
        return 1;
    }

    // 5. 检查余额是否足够
    int actual_draw_count = draw_count;
    if (!gacha_check_balance(state, draw_count)) {
        // 余额不足，确认是否继续
        if (!gacha_confirm_continue(balance)) {
            // 用户取消
            gacha_free(state);
            free_gachalist(list);
            free_config(config);
            free(config_path);
            free(gachalist_path);
            return 0;
        }
        actual_draw_count = balance;
    }

    // 6. 显示当前余额
    gacha_output_balance(balance);

    // 7. 执行抽取
    int actual_count = 0;
    GachaResult* results = gacha_draw_multiple(state, actual_draw_count, &actual_count);

    // 8. 输出结果
    for (int i = 0; i < actual_count; i++) {
        gacha_output_result(&results[i]);
    }

    // 9. 显示剩余余额
    int remaining_balance = balance - actual_count;
    gacha_output_remaining_balance(remaining_balance);

    // 10. 输出统计
    gacha_output_stats(state);

    // 11. 更新配置文件中的历史总匹配次数
    config->history_total_count = remaining_balance;
    if (save_config(config_path, config) != 0) {
        fprintf(stderr, "警告: 无法保存配置文件\n");
    }

    // 12. 清理资源
    for (int i = 0; i < actual_count; i++) {
        free(results[i].name);
        free(results[i].rank);
    }
    free(results);
    gacha_free(state);
    free_gachalist(list);
    free_config(config);
    free(config_path);
    free(gachalist_path);

    return 0;
}

int main(int argc, char* argv[]) {
    // 1. 初始化配置文件
    init_config_files();

    // 2. 解析命令行参数
    if (argc < 2) {
        print_usage();
        return 1;
    }

    if (strcmp(argv[1], "-c") == 0) {
        // Chaos 模式（第一版功能）
        return run_chaos_mode();
    } else if (strcmp(argv[1], "-g") == 0) {
        // Gacha 模式（第二版功能）
        int draw_count = 1;  // 默认值
        if (argc > 2) {
            draw_count = parse_draw_count(argv[2]);
            if (draw_count <= 0) {
                fprintf(stderr, "错误: 参数必须是正整数\n");
                print_usage();
                return 1;
            }
        }
        return run_gacha_mode(draw_count);
    } else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        // 帮助信息
        print_help();
        return 0;
    } else {
        fprintf(stderr, "错误: 未知参数 %s\n", argv[1]);
        print_usage();
        return 1;
    }
}
