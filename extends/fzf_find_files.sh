#!/bin/bash
# fzf_find_files.sh: fzf execute 动作的辅助脚本
# 用法: ./fzf_find_files.sh "可能带引号的路径"

# 获取从 fzf execute 传来的第一个参数
raw_path="$1"

# 如果参数为空，则使用HOME目录
if [[ -z "$raw_path" ]]; then
  raw_path="$HOME"
fi

# 使用 sed 去除参数两端可能存在的单引号
# 这是为了解决 {} 被替换成 'path/' 的问题
cleaned_path=$(echo "$raw_path" | sed "s/^'//;s/'$//")

# 切换到目标目录
cd "$cleaned_path" || exit 1

# 在当前目录下搜索文件，并输出完整路径
fd -t f | fzf \
  --prompt="文件 @ [$(pwd)] > " \
  --height="100%" \
  --preview 'bat --color=always --style=plain {} || cat {}' \
  --with-nth=-1 \
  --bind 'ctrl-/:change-preview-window(hidden|)' |
  while read -r file; do
    # 输出选中文件的完整路径
    echo "$PWD/$file"
  done