#!/bin/bash

source "$(dirname "${BASH_SOURCE[0]}")/fzf_config_template.sh"

# 获取 tmux 会话帮助信息
get_tmux_help() {
    cat << 'EOF'
┌ 操作 ──────────┐  ┌ 管理 ──────────┐  ┌ 预览 ──────────┐
│ enter   选择会话 │  │ ctrl-x 终止会话 │  │ ctrl-h 向下滚动 │
│ ctrl-c  取消选择 │  │ ctrl-r 刷新列表 │  │ ctrl-l 向上滚动 │
│                  │  │                  │  │ ctrl-/ 预览开关 │
└──────────────────┘  └──────────────────┘  └──────────────────┘
EOF
}

# 检查参数
if [ -z "$1" ]; then
    echo "用法: $0 <list|new [session_name]>"
    echo "  list: 管理现有会话"
    echo "  new:  创建新会话，需要提供会话名称"
    exit 1
fi

case "$1" in
    "list")
        # 从模板获取基本配置
        FZF_OPTS="$(get_fzf_base_options) $(get_fzf_color_theme)"
        
        # 使用 fzf 预览会话内容并提供快捷键操作
        selected_session=$(tmux list-sessions -F "#{session_name}" | fzf \
            --preview 'tmux capture-pane -pt {}' \
            --preview-window=right:60% \
            --header "$(get_tmux_help)" \
            --header-first \
            --bind 'enter:accept' \
            --bind 'ctrl-x:execute-silent(tmux kill-session -t {})+reload(tmux list-sessions -F "#{session_name}")' \
            --bind 'ctrl-r:reload(tmux list-sessions -F "#{session_name}")' \
            --bind 'ctrl-h:preview-down,ctrl-l:preview-up' \
            --bind 'ctrl-/:change-preview-window(hidden|)' \
            --bind 'ctrl-c:abort' \
            $FZF_OPTS)

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