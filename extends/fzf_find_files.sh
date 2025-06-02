#!/bin/bash
# fzf_find_files.sh: fzf execute 动作的辅助脚本
# 用法: ./fzf_find_files.sh "可能带引号的路径"

# 导入配置模板
source "$(dirname "$0")/fzf_config_template.sh"

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
┌ 导航 ──────────┐  ┌ 动作 ──────────┐  ┌ 切换 ──────────┐
│ ctrl-j/k 上下选择 │  │ enter   确认选择 │  │ ctrl-y 复制路径 │
│ ctrl-u/d 翻页    │  │ ctrl-v  VSCode  │  │ ctrl-p 预览开关 │
│ alt-u/d  滚动    │  │ ctrl-e  Kate    │  │ ctrl-s 排序开关 │
│                  │  │ ctrl-o  打开     │  │ ctrl-h 帮助开关 │
│                  │  │ ctrl-/  预览     │  │                 │
└──────────────────┘  └──────────────────┘  └──────────────────┘
EOF
)

# 创建临时文件用于存储当前状态
echo "0" > /tmp/help_state

# 在当前目录下搜索文件，并输出完整路径
log "Starting fzf with fd search..."

# 使用 fzf 的动态重载功能，当用户输入时才执行 fd
selected_file=$(eval "fzf $(get_fzf_complete_config)")

# 清理临时文件
rm -f /tmp/help_state

log "FZF completed. Selected: $selected_file"

# 如果有选中的文件，输出完整路径
if [ -n "$selected_file" ]; then
  file=$(echo "$selected_file" | tail -n1)
  if [ -n "$file" ]; then
    echo "$PWD/$file"
    log "Output complete path: $PWD/$file"
  fi
fi

log "Script ending"
exit 0