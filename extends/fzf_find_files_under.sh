#!/bin/bash
# fzf_find_files_under.sh: 目录浏览和文件查找的辅助脚本

# 导入配置模板
source "$(dirname "${BASH_SOURCE[0]}")/fzf_config_template.sh"

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

# 构建 fzf 命令
FZF_OPTS="$(get_fzf_complete_config) --bind='alt-enter:execute($SCRIPT_DIR/fzf_find_files.sh {})+abort'"

# 执行命令
selected_dir=$(fd -t d | eval "fzf $FZF_OPTS")

# 处理选中的目录
if [[ -n "$selected_dir" ]]; then
  if [ -d "$selected_dir" ]; then
    # 只有当选中的是目录时，才输出完整路径
    echo "$PWD/$selected_dir"
  else
    echo "$selected_dir"
  fi
fi