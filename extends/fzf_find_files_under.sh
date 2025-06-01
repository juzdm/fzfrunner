#!/bin/bash
# fzf_find_files_under.sh: 目录浏览和文件查找的辅助脚本

# 启用调试输出
exec 2>/tmp/fzf_find_files_under.log
set -x

# 获取脚本所在目录的绝对路径
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 获取从 fzf execute 传来的第一个参数
raw_path="$1"

# 记录进程信息
echo "Script PID: $$" >> /tmp/fzf_find_files_under.log
echo "Parent PID: $PPID" >> /tmp/fzf_find_files_under.log
ps -f -p $PPID >> /tmp/fzf_find_files_under.log 2>&1

# 如果参数为空，则使用HOME目录
if [[ -z "$raw_path" ]]; then
  raw_path="$HOME"
fi

# 使用 sed 去除参数两端可能存在的单引号
# 这是为了解决 {} 被替换成 'path/' 的问题
cleaned_path=$(echo "$raw_path" | sed "s/^'//;s/'$//")
echo "Cleaned path: $cleaned_path" >> /tmp/fzf_find_files_under.log

# 切换到目标目录
cd "$cleaned_path" || exit 1
echo "Changed to directory: $(pwd)" >> /tmp/fzf_find_files_under.log

# 使用 fzf 的动态重载功能，当用户输入时才执行 fd
echo "Starting fzf with incremental fd search..." >> /tmp/fzf_find_files_under.log

selected_dir=$(fzf \
  --prompt="Select Directory > " \
  --header "Tip: Press ALT-ENTER to search files" \
  --bind "alt-enter:execute($SCRIPT_DIR/fzf_find_files.sh {})+abort" \
  --bind "change:reload:fd -t d {q}" \
  --bind "start:reload:fd -t d" \
  --preview 'ls -a --color=always {}' \
  --preview-window=right:30%:wrap \
  --select-1 \
  --exit-0 \
  --print-query \
  --disabled \
  --delimiter / \
  --nth=-1)

echo "FZF completed. Selected: $selected_dir" >> /tmp/fzf_find_files_under.log

# 如果有选中的目录，输出完整路径
if [ -n "$selected_dir" ]; then
  # 获取最后一行（实际选中的目录）
  dir=$(echo "$selected_dir" | tail -n1)
  if [ -n "$dir" ] && [ -d "$dir" ]; then
    echo "$PWD/$dir"
    echo "Output complete path: $PWD/$dir" >> /tmp/fzf_find_files_under.log
  fi
fi

echo "Script ending" >> /tmp/fzf_find_files_under.log
# 正常退出
exit 0