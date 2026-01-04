cmake_minimum_required(VERSION 3.10)
project(gacha C)

set(CMAKE_C_STANDARD 99)

# 源文件
set(SOURCES
    src/main.c
    src/config.c
    src/random.c
    src/matcher.c
    src/output.c
    src/gacha.c
    src/list.c
)

# 头文件目录
include_directories(src)

# 可执行文件
add_executable(gacha ${SOURCES})

# 编译选项
if(MSVC)
    target_compile_options(gacha PRIVATE /W4)
else()
    target_compile_options(gacha PRIVATE -Wall -Wextra -pedantic)
endif()
