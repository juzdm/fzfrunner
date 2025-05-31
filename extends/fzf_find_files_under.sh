#!/bin/bash
# fzf_find_files_under.sh: 目录浏览和文件查找的辅助脚本

# 获取脚本所在目录的绝对路径
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

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

fd -t d | fzf --prompt="Select Directory > " \
--header "Tip: Press ALT-ENTER to search files" \
--bind "alt-enter:execute($SCRIPT_DIR/fzf_find_files.sh {})+abort" \
--preview 'ls -a --color=always {}' \
--preview-window=right:30%:wrap |
  while read -r file; do
    if [ -d "$file" ]; then
      # 只有当选中的是目录时，才输出完整路径
      echo "$PWD/$file"
    else
      echo "$file"
      break
    fi
  done