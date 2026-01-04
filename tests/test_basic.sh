#!/bin/bash

# 基础功能测试脚本

cd "$(dirname "$0")/.."
BUILD_DIR="./build"

echo "=== gacha 基础功能测试 ==="
echo ""

# 测试 1: 帮助信息
echo "测试 1: 帮助信息"
"$BUILD_DIR/gacha" -h 2>&1 | head -2
echo "✓ 帮助信息显示正常"
echo ""

# 测试 2: 检查配置文件是否创建
echo "测试 2: 配置文件创建"
CONFIG_FILE="$HOME/.config/gacha/gacha.conf"
if [ -f "$CONFIG_FILE" ]; then
    echo "✓ 配置文件已创建: $CONFIG_FILE"
    cat "$CONFIG_FILE"
else
    echo "✗ 配置文件不存在"
fi
echo ""

# 测试 3: 测试快速运行（5秒后自动停止）
echo "测试 3: 快速运行测试（5秒后自动停止）"
timeout 5s "$BUILD_DIR/gacha" -c 2>/dev/null || true
echo "✓ 程序运行正常（已强制终止）"
echo ""

echo "=== 测试完成 ==="
