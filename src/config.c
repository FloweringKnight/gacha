#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
    #include <direct.h>
    #include <windows.h>
    #define mkdir_(_path) _mkdir(_path)
#else
    #include <unistd.h>
    #define mkdir_(_path) mkdir(_path, 0755)
#endif

// 解析行类型
typedef enum {
    LINE_TYPE_COMMENT,      // # 注释
    LINE_TYPE_SECTION,      // ## 章节
    LINE_TYPE_LIST_ITEM,    // - 列表项
    LINE_TYPE_EMPTY,        // 空行
    LINE_TYPE_UNKNOWN       // 未知
} LineType;

// 配置节类型
typedef enum {
    SECTION_NONE,
    SECTION_LETTERS_PER_SECOND,
    SECTION_DICTIONARY,
    SECTION_HISTORY_STATS
} SectionType;

// 展开用户目录（处理 ~）
static char* expand_home(const char* path) {
    if (path == NULL) {
        return NULL;
    }

    // 如果路径不以 ~ 开头，直接复制
    if (path[0] != '~') {
        return strdup(path);
    }

    // 获取 HOME 环境变量
    char* home = getenv("HOME");
#ifdef _WIN32
    if (home == NULL) {
        home = getenv("USERPROFILE");
    }
#endif

    if (home == NULL) {
        return NULL;
    }

    // 替换 ~ 为 HOME 路径
    size_t len = strlen(home) + strlen(path);
    char* expanded = malloc(len);
    if (expanded == NULL) {
        return NULL;
    }
    sprintf(expanded, "%s%s", home, path + 1);

    return expanded;
}

// 获取配置文件路径
char* get_config_path() {
    char* config_dir = NULL;
    char* config_path = NULL;

#ifdef _WIN32
    char* appdata = getenv("APPDATA");
    if (appdata) {
        config_dir = malloc(strlen(appdata) + strlen("\\gacha") + 1);
        if (config_dir) {
            sprintf(config_dir, "%s\\gacha", appdata);
        }
    }
#else
    char* home = getenv("HOME");
    if (home) {
        config_dir = malloc(strlen(home) + strlen("/.config/gacha") + 1);
        if (config_dir) {
            sprintf(config_dir, "%s/.config/gacha", home);
        }
    }
#endif

    if (config_dir == NULL) {
        return NULL;
    }

    // 创建配置目录
    mkdir_(config_dir);

    // 构建配置文件路径
    config_path = malloc(strlen(config_dir) + strlen("/gacha.conf") + 1);
    if (config_path) {
        sprintf(config_path, "%s/gacha.conf", config_dir);
    }

    free(config_dir);
    return config_path;
}

// 检查配置文件是否存在
int config_file_exists(const char* path) {
    if (path == NULL) {
        return 0;
    }

    struct stat st;
    return (stat(path, &st) == 0 && S_ISREG(st.st_mode)) ? 1 : 0;
}

// 创建默认配置文件
int create_default_config(const char* path) {
    if (path == NULL) {
        return -1;
    }

    FILE* fp = fopen(path, "w");
    if (fp == NULL) {
        return -1;
    }

    fprintf(fp, "# Gacha Configuration\n");
    fprintf(fp, "\n");
    fprintf(fp, "## 字母生成速度\n");
    fprintf(fp, "- 每秒生成字母数：2\n");
    fprintf(fp, "\n");
    fprintf(fp, "## 字典列表\n");
    fprintf(fp, "- Hello\n");
    fprintf(fp, "- World\n");
    fprintf(fp, "\n");
    fprintf(fp, "## 历史统计\n");
    fprintf(fp, "- 历史总匹配次数：0\n");

    fclose(fp);
    return 0;
}

// 解析行类型
static LineType parse_line_type(const char* line) {
    if (line == NULL || line[0] == '\0') {
        return LINE_TYPE_EMPTY;
    }

    // 跳过前导空格
    while (*line == ' ' || *line == '\t') {
        line++;
    }

    // 先检查 ## 节
    if (line[0] == '#' && line[1] == '#') {
        return LINE_TYPE_SECTION;
    }

    // 再检查 # 注释
    if (*line == '#') {
        return LINE_TYPE_COMMENT;
    }

    // 检查 - 列表项
    if (*line == '-' && (line[1] == ' ' || line[1] == '\t')) {
        return LINE_TYPE_LIST_ITEM;
    }

    return LINE_TYPE_UNKNOWN;
}

// 从列表项提取内容
static char* extract_list_item_content(const char* line) {
    if (line == NULL || line[0] != '-') {
        return NULL;
    }

    // 跳过 '-'
    line++;

    // 跳过空格
    while (*line == ' ' || *line == '\t') {
        line++;
    }

    // 查找分隔符 '：'（UTF-8 编码）
    const char* colon = strstr(line, "：");
    if (colon != NULL) {
        // 跳过分隔符（中文冒号是 3 字节的 UTF-8 编码）
        colon += 3;

        // 跳过分隔符后的空格
        while (*colon == ' ' || *colon == '\t') {
            colon++;
        }

        return strdup(colon);
    }

    // 没有分隔符，整个都是内容
    return strdup(line);
}

// 解析配置文件
GachaConfig* parse_config(const char* path) {
    if (path == NULL) {
        return NULL;
    }

    FILE* fp = fopen(path, "r");
    if (fp == NULL) {
        return NULL;
    }

    // 初始化配置
    GachaConfig* config = (GachaConfig*)malloc(sizeof(GachaConfig));
    if (config == NULL) {
        fclose(fp);
        return NULL;
    }

    config->letters_per_second = DEFAULT_LETTERS_PER_SECOND;
    config->dictionary = NULL;
    config->dictionary_size = 0;
    config->history_total_count = DEFAULT_HISTORY_TOTAL_COUNT;

    // 预分配字典数组
    int dict_capacity = 10;
    config->dictionary = (char**)malloc(dict_capacity * sizeof(char*));
    if (config->dictionary == NULL) {
        fclose(fp);
        free(config);
        return NULL;
    }

    SectionType current_section = SECTION_NONE;
    char line[1024];

    while (fgets(line, sizeof(line), fp) != NULL) {
        // 去除换行符
        line[strcspn(line, "\r\n")] = '\0';

        LineType line_type = parse_line_type(line);

        switch (line_type) {
            case LINE_TYPE_SECTION:
                if (strstr(line, "字母生成速度")) {
                    current_section = SECTION_LETTERS_PER_SECOND;
                } else if (strstr(line, "字典列表")) {
                    current_section = SECTION_DICTIONARY;
                } else if (strstr(line, "历史统计")) {
                    current_section = SECTION_HISTORY_STATS;
                } else {
                    current_section = SECTION_NONE;
                }
                break;

            case LINE_TYPE_LIST_ITEM: {
                char* content = extract_list_item_content(line);
                if (content == NULL) {
                    break;
                }

                switch (current_section) {
                    case SECTION_LETTERS_PER_SECOND:
                        config->letters_per_second = atoi(content);
                        if (config->letters_per_second < 1 || config->letters_per_second > 60) {
                            config->letters_per_second = DEFAULT_LETTERS_PER_SECOND;
                        }
                        break;

                    case SECTION_DICTIONARY:
                        // 扩展字典数组
                        if (config->dictionary_size >= dict_capacity) {
                            dict_capacity *= 2;
                            char** new_dict = (char**)realloc(config->dictionary,
                                                               dict_capacity * sizeof(char*));
                            if (new_dict == NULL) {
                                free(content);
                                break;
                            }
                            config->dictionary = new_dict;
                        }
                        config->dictionary[config->dictionary_size++] = content;
                        content = NULL; // 已转移所有权
                        break;

                    case SECTION_HISTORY_STATS:
                        config->history_total_count = atoi(content);
                        if (config->history_total_count < 0) {
                            config->history_total_count = DEFAULT_HISTORY_TOTAL_COUNT;
                        }
                        break;

                    default:
                        break;
                }

                if (content != NULL) {
                    free(content);
                }
                break;
            }

            default:
                break;
        }
    }

    fclose(fp);
    return config;
}

// 获取默认配置
GachaConfig* get_default_config() {
    GachaConfig* config = (GachaConfig*)malloc(sizeof(GachaConfig));
    if (config == NULL) {
        return NULL;
    }

    config->letters_per_second = DEFAULT_LETTERS_PER_SECOND;
    config->history_total_count = DEFAULT_HISTORY_TOTAL_COUNT;

    // 创建默认字典
    config->dictionary_size = DEFAULT_DICTIONARY_SIZE;
    config->dictionary = (char**)malloc(config->dictionary_size * sizeof(char*));
    if (config->dictionary == NULL) {
        free(config);
        return NULL;
    }

    config->dictionary[0] = strdup("Hello");
    config->dictionary[1] = strdup("World");

    return config;
}

// 保存配置文件
int save_config(const char* path, GachaConfig* config) {
    if (path == NULL || config == NULL) {
        return -1;
    }

    FILE* fp = fopen(path, "w");
    if (fp == NULL) {
        return -1;
    }

    fprintf(fp, "# Gacha Configuration\n");
    fprintf(fp, "\n");
    fprintf(fp, "## 字母生成速度\n");
    fprintf(fp, "- 每秒生成字母数：%d\n", config->letters_per_second);
    fprintf(fp, "\n");
    fprintf(fp, "## 字典列表\n");
    for (int i = 0; i < config->dictionary_size; i++) {
        fprintf(fp, "- %s\n", config->dictionary[i]);
    }
    fprintf(fp, "\n");
    fprintf(fp, "## 历史统计\n");
    fprintf(fp, "- 历史总匹配次数：%d\n", config->history_total_count);

    fclose(fp);
    return 0;
}

// 释放配置内存
void free_config(GachaConfig* config) {
    if (config == NULL) {
        return;
    }

    if (config->dictionary != NULL) {
        for (int i = 0; i < config->dictionary_size; i++) {
            if (config->dictionary[i] != NULL) {
                free(config->dictionary[i]);
            }
        }
        free(config->dictionary);
    }

    free(config);
}
