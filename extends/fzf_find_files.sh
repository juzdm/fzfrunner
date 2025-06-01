#!/bin/bash
# fzf_find_files.sh: fzf execute 动作的辅助脚本
# 用法: ./fzf_find_files.sh "可能带引号的路径"

# 启用调试输出
exec 2>/tmp/fzf_find_files.log
set -x

# 获取从 fzf execute 传来的第一个参数
raw_path="$1"

# 记录进程信息
echo "Script PID: $$" >> /tmp/fzf_find_files.log
echo "Parent PID: $PPID" >> /tmp/fzf_find_files.log
ps -f -p $PPID >> /tmp/fzf_find_files.log 2>&1

# 如果参数为空，则使用HOME目录
if [[ -z "$raw_path" ]]; then
  raw_path="$HOME"
fi

# 使用 sed 去除参数两端可能存在的单引号
# 这是为了解决 {} 被替换成 'path/' 的问题
cleaned_path=$(echo "$raw_path" | sed "s/^'//;s/'$//")
echo "Cleaned path: $cleaned_path" >> /tmp/fzf_find_files.log

# 切换到目标目录
cd "$cleaned_path" || exit 1
echo "Changed to directory: $(pwd)" >> /tmp/fzf_find_files.log

# 在当前目录下搜索文件，并输出完整路径
echo "Starting fzf with incremental fd search..." >> /tmp/fzf_find_files.log

# 使用 fzf 的动态重载功能，当用户输入时才执行 fd
selected_file=$(fzf \
  --prompt="文件 @ [$(pwd)] > " \
  --height="100%" \
  --preview 'bat --color=always --style=plain {} || cat {}' \
  --preview-window='right:60%' \
  --bind 'ctrl-/:change-preview-window(hidden|)' \
  --bind "change:reload:fd -t f -H {q}" \
  --bind "start:reload:fd -t f -H" \
  --select-1 \
  --exit-0 \
  --print-query \
  --disabled \
  --delimiter / \
  --nth=-1)

echo "FZF completed. Selected: $selected_file" >> /tmp/fzf_find_files.log

# 如果有选中的文件，输出完整路径
if [ -n "$selected_file" ]; then
  # 获取最后一行（实际选中的文件）
  file=$(echo "$selected_file" | tail -n1)
  if [ -n "$file" ]; then
    echo "$PWD/$file"
    echo "Output complete path: $PWD/$file" >> /tmp/fzf_find_files.log
  fi
fi

echo "Script ending" >> /tmp/fzf_find_files.log
# 正常退出
exit 0