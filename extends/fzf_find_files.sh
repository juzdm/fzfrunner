#!/bin/bash
# fzf_find_files.sh: fzf execute 动作的辅助脚本
# 用法: ./fzf_find_files.sh "可能带引号的路径"

# 日志功能开关（默认关闭）
DEBUG=0
LOG_FILE="/tmp/fzf_find_files.log"

# 日志函数
log() {
    if [ "$DEBUG" -eq 1 ]; then
        echo "$1" >> "$LOG_FILE"
    fi
}

# 启用调试输出
if [ "$DEBUG" -eq 1 ]; then
    exec 2>"$LOG_FILE"
    set -x
    # 记录进程信息
    log "Script PID: $$"
    log "Parent PID: $PPID"
    ps -f -p $PPID >> "$LOG_FILE" 2>&1
fi

# 获取从 fzf execute 传来的第一个参数
raw_path="$1"

# 如果参数为空，则使用HOME目录
if [[ -z "$raw_path" ]]; then
  raw_path="$HOME"
fi

# 使用 sed 去除参数两端可能存在的单引号
# 这是为了解决 {} 被替换成 'path/' 的问题
cleaned_path=$(echo "$raw_path" | sed "s/^'//;s/'$//")
log "Cleaned path: $cleaned_path"

# 切换到目标目录
cd "$cleaned_path" || {
    log "Error: Failed to change directory to $cleaned_path"
    exit 1
}
log "Changed to directory: $(pwd)"

# 定义帮助信息
HELP_MSG=$(cat << 'EOF'
文件搜索 @ %s

快捷键说明:
╭─ 导航 ──────────╮ ╭─ 动作 ──────────╮ ╭─ 切换 ──────────╮
│ ctrl-j/k: 上下行 │ │ enter: 输入选择 │ │ ctrl-y: 复制路径 │
│ ctrl-u/d: 翻页   │ │ ctrl-v: VSCode  │ │ ctrl-p: 预览开关 │
│ alt-u/d:  滚动   │ │ ctrl-e: Kate    │ │ ctrl-s: 排序开关 │
│                  │ │ ctrl-o: 打开     │ │ ctrl-h: 帮助显示 │
│                  │ │ ctrl-/: 预览     │ │                  │
╰──────────────────╯ ╰──────────────────╯ ╰──────────────────╯
EOF
)

# 格式化帮助信息，插入当前目录
HELP_MSG=$(printf "$HELP_MSG" "$(pwd)")

# 在当前目录下搜索文件，并输出完整路径
log "Starting fzf with incremental fd search..."

# 使用 fzf 的动态重载功能，当用户输入时才执行 fd
selected_file=$(fzf \
  --prompt="搜索 > " \
  --height="100%" \
  --preview 'bat --color=always --style=plain --line-range :200 {} 2>/dev/null || cat {} 2>/dev/null || echo "Binary file"' \
  --preview-window='right:60%:wrap' \
  --bind 'ctrl-/:change-preview-window(hidden|)' \
  --bind "change:reload:fd -t f {q}" \
  --bind "start:reload:fd -t f" \
  --bind 'ctrl-y:execute-silent(echo -n {} | xclip -selection clipboard)+abort' \
  --bind 'ctrl-v:execute(code {})+abort' \
  --bind 'ctrl-e:execute(kate {})+abort' \
  --bind 'ctrl-o:execute(xdg-open {})+abort' \
  --bind 'ctrl-p:toggle-preview' \
  --bind 'ctrl-s:toggle-sort' \
  --bind 'ctrl-u:preview-page-up' \
  --bind 'ctrl-d:preview-page-down' \
  --bind 'alt-u:preview-up' \
  --bind 'alt-d:preview-down' \
  --bind "ctrl-h:change-header($HELP_MSG)+change-prompt(搜索 (按Ctrl-h显示帮助) > )" \
  --header '' \
  --prompt="搜索 (按Ctrl-h显示帮助) > " \
  --print-query \
  --disabled)

log "FZF completed. Selected: $selected_file"

# 如果有选中的文件，输出完整路径
if [ -n "$selected_file" ]; then
  # 获取最后一行（实际选中的文件）
  file=$(echo "$selected_file" | tail -n1)
  if [ -n "$file" ]; then
    echo "$PWD/$file"
    log "Output complete path: $PWD/$file"
  fi
fi

log "Script ending"
exit 0