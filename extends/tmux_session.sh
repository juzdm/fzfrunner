#!/bin/bash

# 启用调试输出
exec 2>/tmp/tmux_session.log
set -x

# 记录进程信息
echo "Script PID: $$" >> /tmp/tmux_session.log
echo "Parent PID: $PPID" >> /tmp/tmux_session.log
ps -f -p $PPID >> /tmp/tmux_session.log 2>&1

# 检查参数
if [ -z "$1" ]; then
    echo "用法: $0 <list|new [session_name]>"
    echo "  list: 管理现有会话"
    echo "  new:  创建新会话，需要提供会话名称"
    exit 1
fi

# 创建临时文件用于存储会话列表
TEMP_FILE=$(mktemp)
echo "Created temporary file: $TEMP_FILE" >> /tmp/tmux_session.log

case "$1" in
    "list")
        echo "Listing tmux sessions..." >> /tmp/tmux_session.log
        
        # 获取会话列表到临时文件
        tmux list-sessions -F "#{session_name}" > "$TEMP_FILE"
        
        # 使用 fzf 预览会话内容并提供快捷键操作
        selected_session=$(cat "$TEMP_FILE" | fzf \
            --preview 'tmux capture-pane -pt {}'  \
            --preview-window=right:60% \
            --header '查看和管理 tmux 会话' \
            --header-first \
            --prompt '选择会话 > ' \
            --bind 'enter:accept' \
            --bind "ctrl-x:execute-silent(tmux kill-session -t {})+reload(cat $TEMP_FILE)" \
            --bind "ctrl-r:reload(tmux list-sessions -F '#{session_name}' > $TEMP_FILE && cat $TEMP_FILE)" \
            --bind 'ctrl-h:preview-down,ctrl-l:preview-up' \
            --bind 'ctrl-/:change-preview-window(hidden|)' \
            --bind 'ctrl-c:abort' \
            --info=inline \
            --layout=reverse \
            --pointer='>' \
            --marker='*' \
            --header $'操作说明:\n\tEnter: 连接会话\n\tCtrl-X: 终止会话\n\tCtrl-R: 刷新列表\n\tCtrl-H/L: 预览滚动\n\tCtrl-/: 切换预览窗口' \
            --select-1 \
            --exit-0)

        echo "FZF completed. Selected session: $selected_session" >> /tmp/tmux_session.log

        # 如果选择了会话，则连接到该会话
        if [ -n "$selected_session" ]; then
            echo "Attaching to session: $selected_session" >> /tmp/tmux_session.log
            # 清理临时文件和进程
            rm -f "$TEMP_FILE"
            pkill -P $$ 2>/dev/null
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

        echo "Creating/attaching to session: $2" >> /tmp/tmux_session.log

        # 检查会话是否已存在
        if tmux has-session -t "$2" 2>/dev/null; then
            echo "会话 '$2' 已存在，正在连接..."
            echo "Session exists, attaching..." >> /tmp/tmux_session.log
        else
            echo "创建新会话 '$2'..."
            echo "Creating new session..." >> /tmp/tmux_session.log
            tmux new-session -d -s "$2"
        fi
        # 清理临时文件和进程
        rm -f "$TEMP_FILE"
        pkill -P $$ 2>/dev/null
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

# 清理临时文件
rm -f "$TEMP_FILE"
echo "Script ending" >> /tmp/tmux_session.log
# 确保所有子进程都已终止
pkill -P $$ 2>/dev/null

# 正常退出
exit 0