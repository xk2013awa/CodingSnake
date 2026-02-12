@echo off
REM 编译 CodingSnake 示例程序

echo 编译中...
g++ -std=c++11 demo.cpp -o bot.exe -lpthread

if %errorlevel% equ 0 (
    echo 编译成功！
    echo 运行: bot.exe
) else (
    echo 编译失败
    exit /b 1
)
