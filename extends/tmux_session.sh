#!/bin/bash

# 检查参数
if [ -z "$1" ]; then
    echo "用法: $0 <list|new [session_name]>"
    echo "  list: 管理现有会话"
    echo "  new:  创建新会话，需要提供会话名称"
    exit 1
fi

case "$1" in
    "list")
        # 使用 fzf 预览会话内容并提供快捷键操作
        selected_session=$(tmux list-sessions -F "#{session_name}" | fzf \
            --preview 'tmux capture-pane -pt {}'  \
            --preview-window=right:60% \
            --header '查看和管理 tmux 会话' \
            --header-first \
            --prompt '选择会话 > ' \
            --bind 'enter:accept' \
            --bind 'ctrl-x:execute-silent(tmux kill-session -t {})+reload(tmux list-sessions -F "#{session_name}")' \
            --bind 'ctrl-r:reload(tmux list-sessions -F "#{session_name}")' \
            --bind 'ctrl-h:preview-down,ctrl-l:preview-up' \
            --bind 'ctrl-/:change-preview-window(hidden|)' \
            --bind 'ctrl-c:abort' \
            --info=inline \
            --layout=reverse \
            --pointer='>' \
            --marker='*' \
            --header $'操作说明:\n\tEnter: 连接会话\n\tCtrl-X: 终止会话\n\tCtrl-R: 刷新列表\n\tCtrl-H/L: 预览滚动\n\tCtrl-/: 切换预览窗口')

        # 如果选择了会话，则连接到该会话
        if [ -n "$selected_session" ]; then
            exec tmux attach-session -t "$selected_session"
        fi
        ;;
        
    "new")
        # 检查是否提供了会话名称
        if [ -z "$2" ]; then
            echo "错误: 创建新会话需要提供会话名称"
            echo "用法: $0 new <session_name>"
            exit 1
        fi

        # 检查会话是否已存在
        if tmux has-session -t "$2" 2>/dev/null; then
            echo "会话 '$2' 已存在，正在连接..."
        else
            echo "创建新会话 '$2'..."
            tmux new-session -d -s "$2"
        fi
        # 连接到会话
        exec tmux attach-session -t "$2"
        ;;
        
    *)
        echo "用法: $0 <list|new [session_name]>"
        echo "  list: 管理现有会话"
        echo "  new:  创建新会话，需要提供会话名称"
        exit 1
        ;;
esac