#!/bin/bash

target="$1"

if [ -f "$target" ]; then
    # 如果是文件，切换到文件所在目录并显示文件详情
    cd "$(dirname "$target")"
    ls -l "$(basename "$target")"
elif [ -d "$target" ]; then
    # 如果是目录，直接切换到该目录
    cd "$target"
fi

# 启动一个新的交互式shell
exec bash
