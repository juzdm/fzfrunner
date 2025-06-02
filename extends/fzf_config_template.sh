#!/bin/bash
# fzf_config_template.sh: fzf 的主题和按键绑定配置模板

# 设置默认的文件查找命令
export FZF_DEFAULT_COMMAND='fd --type f --hidden --follow --exclude .git'

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
--prompt=搜索 \
--height=100% \
--layout=reverse \
--border=rounded \
--margin=1,2 \
--padding=1 \
--info=inline"
}

# 获取 fzf 颜色主题（Tokyo Night）
get_fzf_color_theme() {
    # 基础颜色
    local base_colors="\
--color=fg:#c0caf5,bg:#24283b,preview-bg:#24283b \
--color=fg+:#c0caf5,bg+:#2f354a"

    # 信息和提示颜色
    local info_colors="\
--color=info:#7aa2f7,prompt:#7aa2f7 \
--color=pointer:#bb9af7,marker:#9ece6a"

    # 其他 UI 元素颜色
    local ui_colors="\
--color=spinner:#9ece6a,header:#7aa2f7 \
--color=hl:#ff9e64,hl+:#ff9e64"

    echo "$base_colors $info_colors $ui_colors"
}

# 获取 fzf 预览配置
get_fzf_preview_config() {
    # 预览命令：根据文件类型使用不同的预览方式
    local preview_cmd='\
if [[ -d {} ]]; then \
  ls -la --color=always {}; \
elif [[ -f {} ]]; then \
  case $(file --mime-type -b {}) in \
    text/*|application/json|application/x-shellscript|application/x-php|application/x-yaml|application/xml) \
      cat {} 2>/dev/null | head -200;; \
    image/*) \
      echo "图片文件: $(file -b {})";; \
    application/pdf) \
      echo "PDF 文件: $(file -b {})";; \
    application/zip|application/x-tar|application/x-gzip|application/x-bzip2) \
      echo "压缩文件: $(file -b {})";; \
    *) \
      echo "二进制文件: $(file -b {})";; \
  esac \
fi'

    # 预览窗口配置
    local preview_window="\
--preview-window=right:50%:border-rounded \
--preview-window=cycle"

    echo "--preview '$preview_cmd' $preview_window"
}

# 获取基础按键绑定
get_fzf_key_bindings() {
    local HELP_MSG=$(get_help_msg)

    # 基础导航绑定
    local nav_bindings="\
--bind='ctrl-p:toggle-preview' \
--bind='ctrl-s:toggle-sort' \
--bind='ctrl-/:change-preview-window(hidden|)'"

    # 预览窗口导航
    local preview_bindings="\
--bind='ctrl-u:preview-page-up' \
--bind='ctrl-d:preview-page-down' \
--bind='alt-u:preview-up' \
--bind='alt-d:preview-down'"

    # 动作绑定
    local action_bindings="\
--bind='ctrl-v:execute:code {}' \
--bind='ctrl-e:execute:kate {}' \
--bind='ctrl-o:execute:xdg-open {}'"

    # 帮助信息绑定
    local help_binding="--bind='ctrl-h:change-header:${HELP_MSG}'"

    echo "$nav_bindings $preview_bindings $action_bindings $help_binding"
}

# 获取完整的 fzf 配置
get_fzf_complete_config() {
    echo "$(get_fzf_base_options) $(get_fzf_color_theme) $(get_fzf_preview_config) $(get_fzf_key_bindings)"
} 