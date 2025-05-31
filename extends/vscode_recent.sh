#!/bin/bash

# VS Code 的最近项目信息在 profileAssociations 中
STORAGE_FILE="$HOME/.config/Code/User/globalStorage/storage.json"

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

# 使用 fzf 让用户选择项目
selected_project=$(echo "$recent_folders" | fzf \
    --preview 'ls -la {}' \
    --preview-window=right:60% \
    --header '选择要打开的项目' \
    --prompt '最近的项目 > ' \
    --bind 'ctrl-r:reload(echo "$recent_folders")' \
    --bind 'ctrl-/:change-preview-window(hidden|)' \
    --bind 'ctrl-h:preview-down,ctrl-l:preview-up' \
    --info=inline \
    --layout=reverse \
    --pointer='>' \
    --marker='*' \
    --header $'快捷键说明:\n\tEnter: 打开项目\n\tCtrl-R: 刷新列表\n\tCtrl-/: 切换预览窗口\n\tCtrl-H/L: 预览滚动')

# 输出选择的项目路径
if [ -n "$selected_project" ]; then
    echo "$selected_project"
fi