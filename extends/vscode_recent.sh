#!/bin/bash

source "$(dirname "${BASH_SOURCE[0]}")/fzf_config_template.sh"

# VS Code 的最近项目信息在 profileAssociations 中
STORAGE_FILE="$HOME/.config/Code/User/globalStorage/storage.json"

# 获取 VSCode 项目选择帮助信息
get_vscode_help() {
    cat << 'EOF'
┌ 操作 ──────────┐  ┌ 视图 ──────────┐  ┌ 预览 ──────────┐
│ enter   选择项目 │  │ ctrl-r 刷新列表 │  │ ctrl-h 向下滚动 │
│ ctrl-c  取消操作 │  │ ctrl-/ 预览开关 │  │ ctrl-l 向上滚动 │
└──────────────────┘  └──────────────────┘  └──────────────────┘
EOF
}

# 检查文件是否存在
if [ ! -f "$STORAGE_FILE" ]; then
    echo "找不到 VS Code 存储文件"
    exit 1
fi

# 从 storage.json 中提取所有文件夹路径
recent_folders=$(jq -r '
    .profileAssociations.workspaces | keys[] | 
    select(startswith("file://"))
' "$STORAGE_FILE" | sed 's|^file://||' | sed 's|%20| |g')

# 从模板获取基本配置
FZF_OPTS="$(get_fzf_base_options) $(get_fzf_color_theme)"

# 使用 fzf 让用户选择项目
selected_project=$(echo "$recent_folders" | fzf \
    --preview 'ls -la {}' \
    --preview-window=right:60% \
    --header "$(get_vscode_help)" \
    --header-first \
    --bind 'enter:accept' \
    --bind 'ctrl-r:reload(echo "$recent_folders")' \
    --bind 'ctrl-/:change-preview-window(hidden|)' \
    --bind 'ctrl-h:preview-down,ctrl-l:preview-up' \
    --bind 'ctrl-c:abort' \
    $FZF_OPTS)

# 输出选择的项目路径
if [ -n "$selected_project" ]; then
    echo "$selected_project"
fi