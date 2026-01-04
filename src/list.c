#include "list.h"
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

// 等级到索引的映射
static int rank_to_index(const char* rank) {
    if (strcmp(rank, "N") == 0) return 0;
    if (strcmp(rank, "R") == 0) return 1;
    if (strcmp(rank, "SR") == 0) return 2;
    if (strcmp(rank, "SSR") == 0) return 3;
    if (strcmp(rank, "UR") == 0) return 4;
    return -1;
}

// 获取 gachalist 文件路径
char* get_gachalist_path() {
    char* config_dir = NULL;
    char* gachalist_path = NULL;

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

    // 确保目录存在
    mkdir_(config_dir);

    // 构建 gachalist 文件路径
    gachalist_path = malloc(strlen(config_dir) + strlen("/gachalist") + 1);
    if (gachalist_path) {
        sprintf(gachalist_path, "%s/gachalist", config_dir);
    }

    free(config_dir);
    return gachalist_path;
}

// 检查 gachalist 是否存在
int gachalist_file_exists() {
    char* path = get_gachalist_path();
    if (path == NULL) {
        return 0;
    }

    struct stat st;
    int exists = (stat(path, &st) == 0 && S_ISREG(st.st_mode)) ? 1 : 0;

    free(path);
    return exists;
}

// 验证等级格式
int validate_rank(const char* rank) {
    return rank_to_index(rank) >= 0;
}

// 读取 gachalist 文件
GachaList* read_gachalist(const char* path) {
    if (path == NULL) {
        return NULL;
    }

    FILE* fp = fopen(path, "r");
    if (fp == NULL) {
        return NULL;
    }

    // 第一次遍历：统计行数
    int line_count = 0;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        // 移除换行符
        buffer[strcspn(buffer, "\r\n")] = '\0';
        if (strlen(buffer) > 0) {
            line_count++;
        }
    }

    if (line_count == 0) {
        fclose(fp);
        return NULL;
    }

    // 分配内存
    GachaList* list = (GachaList*)malloc(sizeof(GachaList));
    if (list == NULL) {
        fclose(fp);
        return NULL;
    }

    list->items = (GachaItem*)malloc(sizeof(GachaItem) * line_count);
    if (list->items == NULL) {
        free(list);
        fclose(fp);
        return NULL;
    }

    list->size = 0;
    list->file_path = strdup(path);

    // 第二次遍历：读取内容
    rewind(fp);
    int current_line = 0;
    while (fgets(buffer, sizeof(buffer), fp) != NULL && current_line < line_count) {
        // 移除换行符
        buffer[strcspn(buffer, "\r\n")] = '\0';

        // 跳过空行
        if (strlen(buffer) == 0) {
            continue;
        }

        // 解析等级和菜名
        char* rank_start = strstr(buffer, "【");
        char* rank_end = strstr(buffer, "】");

        if (rank_start != NULL && rank_end != NULL && rank_end > rank_start) {
            // 提取等级（跳过"【" 3字节）
            char* rank_ptr = rank_start + 3;
            char* name_ptr = rank_end + 3;

            // 计算等级长度
            size_t rank_len = rank_end - rank_ptr;
            if (rank_len > 0 && rank_len < 10) {
                list->items[current_line].rank = (char*)malloc(rank_len + 1);
                strncpy(list->items[current_line].rank, rank_ptr, rank_len);
                list->items[current_line].rank[rank_len] = '\0';

                // 验证等级
                if (!validate_rank(list->items[current_line].rank)) {
                    free(list->items[current_line].rank);
                    list->items[current_line].rank = strdup("N");
                }
            } else {
                list->items[current_line].rank = strdup("N");
            }

            // 提取菜名
            list->items[current_line].name = strdup(name_ptr);
        } else {
            // 没有等级标记，按 N 级处理
            list->items[current_line].rank = strdup("N");
            list->items[current_line].name = strdup(buffer);
        }

        current_line++;
    }

    list->size = current_line;
    fclose(fp);

    return list;
}

// 创建默认 gachalist 文件
int create_default_gachalist(const char* path) {
    if (path == NULL) {
        return -1;
    }

    FILE* fp = fopen(path, "w");
    if (fp == NULL) {
        return -1;
    }

    // N 级菜（300道）
    const char* n_dishes[] = {
        "番茄炒蛋", "青椒肉丝", "炒青菜", "炒土豆丝", "炒豆角",
        "炒茄子", "炒黄瓜", "炒西葫芦", "炒胡萝卜", "炒洋葱",
        "炒白菜", "炒生菜", "炒菠菜", "炒油麦菜", "炒韭菜",
        "炒蒜苗", "炒豆芽", "炒空心菜", "炒茭白", "炒藕片",
        "炒芹菜", "炒菜花", "炒西兰花", "炒蘑菇", "炒金针菇",
        "韭菜炒蛋", "洋葱炒蛋", "青椒炒蛋", "葱炒蛋", "蒜炒蛋",
        "红烧豆腐", "麻婆豆腐", "家常豆腐", "煎豆腐", "炒豆腐",
        "凉拌豆腐", "紫菜蛋花汤", "丝瓜蛋汤", "黄瓜蛋汤", "番茄蛋汤",
        "菠菜蛋汤", "豆腐汤", "蛋花汤", "清汤", "白菜汤",
        "萝卜汤", "冬瓜汤", "丝瓜汤", "南瓜汤", "西葫芦汤",
        "土豆汤", "青菜汤", "空心菜汤", "菠菜汤", "韭菜汤",
        "香菜汤", "白米饭", "白粥", "小米粥", "绿豆粥",
        "红豆粥", "南瓜粥", "红薯粥", "玉米粥", "八宝粥",
        "燕麦粥", "蒸蛋", "水煮蛋", "荷包蛋", "蒸米饭",
        "蒸红薯", "煮玉米", "煮红薯", "煮毛豆", "煮花生",
        "煮鸡蛋", "拍黄瓜", "凉拌萝卜丝", "凉拌海带丝", "凉拌木耳",
        "凉拌豆芽", "凉拌黄瓜", "凉拌菠菜", "凉拌茄子", "凉拌番茄",
        "凉拌苦瓜", "蛋炒饭", "酱油炒饭", "扬州炒饭", "蛋炒面",
        "酱油炒面", "葱油面", "阳春面", "清汤面", "素面",
        "拌面", "煮水饺", "蒸饺子", "煮汤圆", "蒸包子",
        "煮粽子", "炒香菇", "炒木耳", "炒银耳", "炒粉丝",
        "炒腐竹", "炒面筋", "炒百叶", "炒豆干", "炒豆腐皮"
    };
    int n_dishes_count = 95;

    // 写入 N 级（重复填充到300）
    for (int i = 0; i < 300; i++) {
        fprintf(fp, "【N】%s\n", n_dishes[i % n_dishes_count]);
    }

    // R 级菜（150道）
    const char* r_dishes[] = {
        "宫保鸡丁", "红烧肉", "麻婆豆腐", "水煮鱼", "糖醋里脊",
        "鱼香肉丝", "回锅肉", "糖醋排骨", "红烧排骨", "可乐鸡翅",
        "白切鸡", "口水鸡", "怪味鸡", "辣子鸡", "土豆烧牛肉",
        "番茄牛腩", "葱爆牛肉", "水煮牛肉", "孜然牛肉", "红烧鱼",
        "清蒸鱼", "糖醋鱼", "酸菜鱼", "剁椒鱼头", "老母鸡汤",
        "排骨汤", "牛骨汤", "鱼头汤", "冬瓜排骨汤", "地三鲜",
        "鱼香茄子", "虎皮青椒", "干煸豆角", "蒜蓉生菜", "红烧牛肉面"
    };
    int r_dishes_count = 30;

    for (int i = 0; i < 150; i++) {
        fprintf(fp, "【R】%s\n", r_dishes[i % r_dishes_count]);
    }

    // SR 级菜（90道）
    const char* sr_dishes[] = {
        "东坡肉", "佛跳墙", "北京烤鸭", "白切鸡", "红烧海参",
        "鲍鱼烧肉", "清蒸龙虾", "蒜蓉龙虾", "红烧螃蟹", "叫花鸡",
        "汽锅鸡", "德州扒鸡", "贵妃鸡", "文昌鸡", "八宝鸭",
        "开水白菜", "文思豆腐", "蟹黄豆腐", "烤全羊", "烤乳猪",
        "烤羊排", "烤牛排", "烤羊腿", "清蒸大闸蟹", "红烧大闸蟹",
        "水煮大闸蟹", "醉蟹", "蟹粉豆腐", "红烧狮子头", "东坡肘子"
    };
    int sr_dishes_count = 30;

    for (int i = 0; i < 90; i++) {
        fprintf(fp, "【SR】%s\n", sr_dishes[i % sr_dishes_count]);
    }

    // SSR 级菜（45道）
    const char* ssr_dishes[] = {
        "开水白菜", "文思豆腐", "孔雀开屏鱼", "蟹黄豆腐", "松鼠桂鱼",
        "佛跳墙", "满汉全席", "烤全猪", "东坡肘子", "红烧狮子头",
        "清蒸石斑鱼", "鲍汁捞饭", "鲍汁扣辽参", "红烧鲍鱼", "红烧海参",
        "龙井虾仁", "西湖醋鱼", "宋嫂鱼羹", "西湖莼菜汤", "蟹粉小笼包",
        "蟹黄汤包", "蟹粉豆腐", "蟹粉狮子头", "蟹粉烧麦", "蟹粉春卷"
    };
    int ssr_dishes_count = 25;

    for (int i = 0; i < 45; i++) {
        fprintf(fp, "【SSR】%s\n", ssr_dishes[i % ssr_dishes_count]);
    }

    // UR 级菜（15道）
    const char* ur_dishes[] = {
        "开水白菜（国宴版）", "佛跳墙（正宗）", "龙井虾仁（极品）",
        "文思豆腐（传世）", "孔雀开屏鱼（御膳）", "东坡肉（祖传）",
        "满汉全席（全席）", "烤全羊（蒙古）", "北京烤鸭（正宗）",
        "小笼包（南翔）", "蟹黄汤包（扬州）", "阳春面（本帮）",
        "松鼠桂鱼（苏帮）", "西湖醋鱼（杭帮）", "佛跳墙（闽菜）"
    };
    int ur_dishes_count = 15;

    for (int i = 0; i < 15; i++) {
        fprintf(fp, "【UR】%s\n", ur_dishes[i % ur_dishes_count]);
    }

    fclose(fp);
    return 0;
}

// 获取内置默认 gachalist
GachaList* get_default_gachalist() {
    GachaList* list = (GachaList*)malloc(sizeof(GachaList));
    if (list == NULL) {
        return NULL;
    }

    // 创建临时文件并读取
    char* temp_path = get_gachalist_path();
    if (temp_path == NULL) {
        free(list);
        return NULL;
    }

    // 生成临时文件路径
    char* tmp = malloc(strlen(temp_path) + 10);
    sprintf(tmp, "%s.tmp", temp_path);

    // 创建临时文件
    if (create_default_gachalist(tmp) != 0) {
        free(tmp);
        free(temp_path);
        free(list);
        return NULL;
    }

    // 读取临时文件
    list = read_gachalist(tmp);

    // 删除临时文件
    remove(tmp);

    free(tmp);
    free(temp_path);

    return list;
}

// 释放 gachalist 内存
void free_gachalist(GachaList* list) {
    if (list == NULL) {
        return;
    }

    if (list->items != NULL) {
        for (int i = 0; i < list->size; i++) {
            if (list->items[i].name != NULL) {
                free(list->items[i].name);
            }
            if (list->items[i].rank != NULL) {
                free(list->items[i].rank);
            }
        }
        free(list->items);
    }

    if (list->file_path != NULL) {
        free(list->file_path);
    }

    free(list);
}
