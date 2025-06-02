#!/bin/bash
# fzf_find_files.sh: fzf execute 动作的辅助脚本
# 用法: ./fzf_find_files.sh "可能带引号的路径"

# 导入配置模板
source "$(dirname "$0")/fzf_config_template.sh"

# 日志功能
DEBUG=${DEBUG:-0}
LOG_FILE="/tmp/fzf_find_files.log"

log() {
    [[ $DEBUG -eq 1 ]] && echo "$1" >> "$LOG_FILE"
}

# 启用调试输出
if [[ $DEBUG -eq 1 ]]; then
    exec 2>"$LOG_FILE"
    set -x
    log "Script PID: $$"
    log "Parent PID: $PPID"
    ps -f -p $PPID >> "$LOG_FILE" 2>&1
fi

# 处理输入路径
raw_path="${1:-$HOME}"
cleaned_path=$(echo "$raw_path" | sed "s/^'//;s/'$//")
log "Cleaned path: $cleaned_path"

# 切换到目标目录
cd "$cleaned_path" || {
    log "Error: Failed to change directory to $cleaned_path"
    exit 1
}
log "Changed to directory: $(pwd)"

# 使用 fzf 搜索文件
log "Starting fzf with fd search..."
selected_file=$(eval "fzf $(get_fzf_complete_config)")

log "FZF completed. Selected: $selected_file"

# 输出选中文件的完整路径
if [[ -n "$selected_file" ]]; then
    file=$(echo "$selected_file" | tail -n1)
    if [[ -n "$file" ]]; then
        echo "$PWD/$file"
        log "Output complete path: $PWD/$file"
    fi
fi

log "Script ending"
exit 0