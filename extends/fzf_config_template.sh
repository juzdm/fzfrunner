#!/bin/bash
# fzf_config_template.sh: fzf 的主题和按键绑定配置模板

# 定义帮助信息模板
get_help_msg() {
    cat << 'EOF'
┌ 导航 ──────────┐  ┌ 动作 ──────────┐  ┌ 切换 ──────────┐
│ ctrl-j/k 上下选择 │  │ enter   确认选择 │  │ ctrl-y 复制路径 │
│ ctrl-u/d 翻页    │  │ ctrl-v  VSCode  │  │ ctrl-p 预览开关 │
│ alt-u/d  滚动    │  │ ctrl-e  Kate    │  │ ctrl-s 排序开关 │
│                  │  │ ctrl-o  打开     │  │ ctrl-h 帮助开关 │
│                  │  │ ctrl-/  预览     │  │                 │
└──────────────────┘  └──────────────────┘  └──────────────────┘
EOF
}

# 获取基础 fzf 选项
get_fzf_base_options() {
    echo "\
  --prompt=\"搜索 (按Ctrl-h显示帮助) > \" \
  --height=\"100%\" \
  --layout=reverse \
  --border=rounded \
  --margin=1,2 \
  --padding=1 \
  --info=inline \
  --header '' \
  --print-query \
  --disabled"
}

# 获取 fzf 颜色主题（Tokyo Night）
get_fzf_color_theme() {
    echo "\
  --color=fg:#c0caf5,bg:#24283b,preview-bg:#24283b \
  --color=fg+:#c0caf5,bg+:#2f354a \
  --color=info:#7aa2f7,prompt:#7aa2f7,border:#7aa2f7 \
  --color=pointer:#bb9af7,marker:#9ece6a,header:#7aa2f7 \
  --color=spinner:#9ece6a,hl:#ff9e64,hl+:#ff9e64"
}

# 获取 fzf 预览配置
get_fzf_preview_config() {
    echo "\
  --preview 'bat --color=always --style=numbers,changes --line-range :200 {} 2>/dev/null || cat {} 2>/dev/null || echo \"Binary file\"' \
  --preview-window='right:60%:border-rounded'"
}

# 获取基础按键绑定
get_fzf_key_bindings() {
    local HELP_MSG="\$(get_help_msg)"
    echo "\
  --bind 'ctrl-/:change-preview-window(hidden|)' \
  --bind 'ctrl-y:execute-silent(echo -n {} | xclip -selection clipboard)+abort' \
  --bind 'ctrl-v:execute(code {})+abort' \
  --bind 'ctrl-e:execute(kate {})+abort' \
  --bind 'ctrl-o:execute(xdg-open {})+abort' \
  --bind 'ctrl-p:toggle-preview' \
  --bind 'ctrl-s:toggle-sort' \
  --bind 'ctrl-u:preview-page-up' \
  --bind 'ctrl-d:preview-page-down' \
  --bind 'alt-u:preview-up' \
  --bind 'alt-d:preview-down' \
  --bind \"ctrl-h:change-header($HELP_MSG)\""
}

# 获取完整的 fzf 配置
get_fzf_complete_config() {
    echo "$(get_fzf_base_options) \
$(get_fzf_color_theme) \
$(get_fzf_preview_config) \
$(get_fzf_key_bindings)"
} 