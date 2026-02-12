#!/bin/bash
# 编译 CodingSnake 示例程序

echo "编译中..."
g++ -std=c++11 demo.cpp -o bot -lpthread

if [ $? -eq 0 ]; then
    echo "✓ 编译成功！"
    echo "运行: ./bot"
else
    echo "✗ 编译失败"
    exit 1
fi
