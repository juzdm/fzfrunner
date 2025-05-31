#!/bin/bash

# 定义常用的代码目录（按优先级排序）
COMMON_DIRS=(
    "$HOME/disk"
    "$HOME/code"
    "$HOME/projects"
)

# 定义搜索深度和 git 目录
MAX_DEPTH=5
GIT_DIR=".repo"

# 检查 fd 是否安装
if ! command -v fd &> /dev/null; then
    echo "请先安装 fd 命令行工具"
    echo "Ubuntu/Debian: sudo apt install fd-find"
    echo "Arch Linux: sudo pacman -S fd"
    echo "Fedora: sudo dnf install fd-find"
    exit 1
fi

# 创建临时文件
TEMP_FILE=$(mktemp)

# 搜索常用目录
for dir in "${COMMON_DIRS[@]}"; do
    if [ -d "$dir" ]; then
        fd -H -t d "^${GIT_DIR}$" "$dir" -d "$MAX_DEPTH" --exec dirname {} \; >> "$TEMP_FILE"
    fi
done

# 获取脚本所在目录的绝对路径
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# 使用 fzf 进行交互式搜索
selected=$(sort -u "$TEMP_FILE" | fzf \
    --prompt="repo仓库 (ENTER:选择, ALT-G:AOSP子仓库, ALT-ENTER:文件搜索) > " \
    --preview 'ls -a --color=always {}' \
    --preview-window=right:40% \
    --height=100% \
    --bind 'enter:accept' \
    --bind "alt-g:execute($SCRIPT_DIR/aosp-find-repo.sh {})+abort" \
    --bind "alt-enter:execute($SCRIPT_DIR/fzf_find_files.sh {})+abort")

# 如果用户选择了一个仓库
if [ -n "$selected" ]; then
    echo "$selected"
fi

# 清理临时文件
rm -f "$TEMP_FILE"
