#!/bin/bash

SESSION_NAME="sing-box"
WORK_DIR="${HOME}/.config/sing-box"
CONFIG_PATH="$WORK_DIR/me.json"    # 指定 me.json 的绝对路径
SINGBOX_PATH="$WORK_DIR/sing-box"  # sing-box 的绝对路径

# 确保目录存在
if [ ! -d "$WORK_DIR" ]; then
    echo "错误: 目录 $WORK_DIR 不存在"
    exit 1
fi

# 检查参数
if [ -z "$1" ]; then
    echo "用法: $0 <on/off>"
    exit 1
fi

# 根据参数执行相应的操作
if [ "$1" = "on" ]; then
    echo "正在启动 sing-box..."
    
    # 检查会话是否存在
    if ! tmux has-session -t "$SESSION_NAME" 2>/dev/null; then
        echo "会话 $SESSION_NAME 不存在，正在创建..."
        # 直接创建会话并在前台运行 sing-box
        tmux new-session -s "$SESSION_NAME" -c "$WORK_DIR" "sudo sing-box run -c $CONFIG_PATH"
    else
        echo "会话 $SESSION_NAME 已存在，发送启动命令..."
        tmux attach-session -t "$SESSION_NAME"
    fi
else
    echo "正在停止 sing-box..."
    # 发送 Ctrl+C 信号停止进程
    if tmux has-session -t "$SESSION_NAME" 2>/dev/null; then
        tmux send-keys -t "$SESSION_NAME" C-c
    else
        echo "会话 $SESSION_NAME 不存在"
    fi

    echo "sing-box 已退出..." 
    read -p "按任意键返回..."
fi