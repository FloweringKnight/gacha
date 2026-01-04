# gacha - 随机字母生成与单词匹配命令行工具

一个跨平台的 C 语言命令行工具，通过随机生成字母并匹配字典单词，实现趣味性的文本生成功能。

## 功能特性

- ✅ 随机生成字母（范围 a-zA-Z）
- ✅ 实时匹配字典单词（严格完整匹配）
- ✅ 匹配成功时换行并加粗显示
- ✅ 支持自定义配置文件（Markdown 格式）
- ✅ 历史匹配次数统计
- ✅ 跨平台支持（Windows、Linux、macOS）

## 系统要求

- C99 兼容的编译器（GCC、Clang、MSVC）
- CMake 3.10 或更高版本

## 编译安装

### 使用 CMake 编译

```bash
# 创建构建目录
mkdir build
cd build

# 配置
cmake ..

# 编译
cmake --build .

# 运行
./gacha -c
```

### 手动编译

```bash
# Linux/macOS
gcc -std=c99 -o gacha src/*.c -I src

# Windows (MinGW)
gcc -std=c99 -o gacha.exe src/*.c -I src

# Windows (MSVC)
cl /std:c99 /Fe:gacha.exe src/*.c /I src
```

## 使用方法

### 基本用法

```bash
gacha -c
```

参数说明：
- `-c`：chaos 模式，启动随机字母生成与单词匹配

### 停止程序

- 按 `Ctrl+C` 手动停止
- 当任一字典单词匹配次数 ≥ 3 时自动停止

## 配置文件

### 配置文件位置

- Windows: `%APPDATA%/gacha/gacha.conf`
- Linux/macOS: `~/.config/gacha/gacha.conf`

### 配置文件格式

配置文件使用 Markdown 格式：

```markdown
# Gacha Configuration

## 字母生成速度
- 每秒生成字母数：2

## 字典列表
- Hello
- World

## 历史统计
- 历史总匹配次数：0
```

### 配置项说明

| 配置项     | 说明          | 默认值          | 范围   |
|---------|-------------|--------------|------|
| 每秒生成字母数 | 随机字母生成速度    | 2            | 1-60 |
| 字典列表    | 要匹配的单词列表    | Hello, World | 任意数量 |
| 历史总匹配次数 | 所有运行的累计匹配次数 | 0            | ≥0   |

## 工作原理

1. 程序以指定速度随机生成字母（a-zA-Z）
2. 生成的字母连续输出到终端
3. 使用滑动窗口实时搜索字典单词
4. 当生成的字母序列与字典单词完全匹配时：
   - 在单词后插入换行
   - 以加粗样式显示匹配的单词
   - 计数器 +1
5. 当任一单词匹配次数 ≥ 3 时自动停止
6. 显示本次匹配总数和历史总匹配次数

## 示例输出

```
开始随机生成 (每秒 2 个字母)
字典包含 2 个单词，按 Ctrl+C 停止

xyzHello
World
abHello
World
匹配总数: 4
历史总匹配次数: 4
```

## 项目结构

```
gacha/
├── CMakeLists.txt              # CMake 构建配置
├── README.md                   # 项目说明
├── .rules/                     # 项目规则
│   └── project-background.md
├── .requirement/               # 需求文档
│   ├── functional-requirements.md
│   └── technical-design.md
├── src/                        # 源代码
│   ├── main.c                  # 主程序
│   ├── config.h/c              # 配置管理
│   ├── random.h/c              # 随机生成
│   ├── matcher.h/c             # 匹配引擎
│   └── output.h/c             # 输出控制
└── tests/                      # 测试代码
    └── test_basic.sh           # 基础测试
```

## 许可证

本项目仅供学习和研究使用。

## 贡献

欢迎提交 Issue 和 Pull Request！
