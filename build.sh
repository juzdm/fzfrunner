#!/bin/bash

# 设置构建目录
BUILD_DIR="build"

# 清理之前的构建目录
echo "清理之前的构建目录..."
rm -rf "$BUILD_DIR"

# 创建构建目录
echo "创建构建目录..."
mkdir -p "$BUILD_DIR"

# 切换到构建目录
cd "$BUILD_DIR"

# 运行 CMake 配置项目
echo "运行 CMake 配置项目..."
cmake ..

# 检查 CMake 是否配置成功
if [ $? -ne 0 ]; then
  echo "CMake 配置失败，请检查错误信息。"
  exit 1
fi

# 运行 Make 编译项目
echo "运行 Make 编译项目..."
make

# 检查 Make 是否编译成功
if [ $? -ne 0 ]; then
  echo "Make 编译失败，请检查错误信息。"
  exit 1
fi

echo "编译成功！"

# 可选：安装插件
sudo  make install

exit 0
