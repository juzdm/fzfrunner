# KRunner Command Runner

一个强大而灵活的 KDE Plasma KRunner 插件，让你能够通过 KRunner 执行自定义命令并处理其输出结果。

**本项目还在开发中，目前对fzf和terminal支持比较好，其他功能可能存BUG**

## 🌟 主要特性

- 支持执行任意 Shell 命令并在 KRunner 中展示结果
- 支持后台执行和终端交互两种模式
- 强大的命令模板系统，支持多种占位符
- 灵活的配置系统，易于定制和扩展
- 内置多种实用命令（如 fzf 文件搜索、目录浏览等）
- 支持多种结果处理方式（文件打开、复制路径等）
- 可自定义触发词和图标
- 支持异步命令执行
- 支持命令结果的预览和格式化

## 📋 依赖项

- KDE Plasma 5.x
- Qt 5.x
- cmake (构建)
- extra-cmake-modules (构建)
- krunner-dev (构建)
- g++ 或 clang++ (构建)

可选依赖：
- fzf (用于文件搜索功能)
- fd-find (更快的文件查找)
- bat (文件预览)
- ripgrep (文本搜索)

## 🚀 安装

### 从源码安装

1. 克隆仓库：
```bash
git clone https://github.com/yourusername/krunner-cmdrunner.git
cd krunner-cmdrunner
```

2. 编译安装：
```bash
mkdir build && cd build
cmake ..
make
sudo make install
```

3. 重启 KRunner：
```bash
kquitapp5 krunner
kstart5 krunner
```

## ⚙️ 配置

配置文件位置：`~/.config/krunner-cmd-settings`

### 基本配置示例

```ini
[General]
# 全局设置
DefaultTerminal=konsole
DefaultEditor=kate
PreviewEnabled=true
MaxResults=20

[Command_FindFile]
Name=Find File
Description=使用 fzf 搜索文件
Icon=system-search
TriggerWords=ff,find
CommandTemplate={FZF_EXTENDS_DIR}/fzf_find_files.sh {query}
ExecutionMode=Background
WorkingDirectoryMode=QueryOrHome
ResultType=FilePath
DefaultAction=OpenFileOrCD
Action_vscode=OpenFileWithVSCode
Action_kate=OpenFileWithKate
Action_copy=CopyToClipboard

[Command_GitGrep]
Name=Git Grep
Description=在 Git 仓库中搜索内容
Icon=code-context
TriggerWords=gg,grep
CommandTemplate=git grep -n {query}
ExecutionMode=Background
WorkingDirectoryMode=Current
ResultType=PlainText
DefaultAction=CopyToClipboard
Action_open=OpenFileAtLine
```

### 配置项详解

#### 通用配置项

| 配置项 | 说明 | 可选值 |
|--------|------|--------|
| Name | 命令名称 | 任意文本 |
| Description | 命令描述 | 任意文本 |
| Icon | 图标名称 | KDE 图标名 |
| TriggerWords | 触发词列表 | 逗号分隔的词列表 |
| CommandTemplate | 命令模板 | Shell 命令 |

#### 执行相关配置

| 配置项 | 说明 | 可选值 |
|--------|------|--------|
| ExecutionMode | 执行模式 | Background/Terminal |
| WorkingDirectoryMode | 工作目录模式 | QueryOrHome/Home/Current/ExplicitPath |
| ResultType | 结果类型 | None/PlainText/FilePath/DirectoryPath |
| DefaultAction | 默认动作 | None/OpenFileOrCD/CopyToClipboard/KRunnerQuery |

### 模板占位符

| 占位符 | 说明 |
|--------|------|
| {query} | 用户输入的查询参数 |
| {FZF_EXTENDS_DIR} | 扩展脚本目录 |
| {output_file} | 输出文件路径 |
| {temp_script} | 临时脚本路径 |
| {HOME} | 用户主目录 |
| {PWD} | 当前工作目录 |

## 🎯 使用方法

### 基本用法

1. 按 Alt+Space 或配置的快捷键唤起 KRunner
2. 输入配置的触发词和查询参数
3. 选择结果并执行默认动作或选择其他动作

### 常用命令示例

1. 文件搜索：
```
ff document     # 搜索包含 "document" 的文件
```

2. Git 仓库搜索：
```
gg TODO         # 搜索代码中的 TODO 注释
```

3. 目录浏览：
```
fzd ~/projects  # 在 projects 目录下浏览文件
```

### 快捷键

- `Enter`: 执行默认动作
- `Shift+Enter`: 显示可用动作列表
- `Ctrl+Enter`: 在终端中执行命令
- `Alt+数字键`: 快速选择动作

## 🛠️ 自定义扩展

### 添加新命令

1. 创建命令脚本：
在 `~/.local/share/krunner-cmd/extends/` 目录下创建脚本

2. 在配置文件中添加命令定义：
```ini
[Command_CustomName]
Name=自定义命令
Description=命令描述
TriggerWords=触发词
CommandTemplate=/path/to/script.sh {query}
...
```

### 自定义动作

可以为命令添加自定义动作：
```ini
Action_customaction=CustomActionName
```

### 支持的动作类型

- OpenFileOrCD
- OpenFileWithVSCode
- OpenFileWithKate
- CopyToClipboard
- KRunnerQuery
- OpenFileAtLine
- Custom_{ActionName}

## 🐛 故障排除

1. 命令无法执行
   - 检查脚本权限
   - 确认依赖工具已安装
   - 查看系统日志

2. 配置不生效
   - 重启 KRunner
   - 检查配置文件格式
   - 确认配置文件权限

3. 性能问题
   - 减少后台命令数量
   - 优化命令执行时间
   - 调整 MaxResults 设置

## 🔧 开发说明

### 目录结构

```
.
├── src/              # 源代码
├── extends/          # 扩展脚本
├── config/           # 配置模板
├── tests/            # 测试文件
└── CMakeLists.txt   # CMake 构建文件
```

### 开发指南

- 遵循 KDE/Qt 编码规范
- 使用 CMake 构建系统
- 提供单元测试
- 保持向后兼容性

## 📄 许可证

本项目采用 GPL-3.0 许可证。详见 [LICENSE](LICENSE) 文件。

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

1. Fork 本仓库
2. 创建特性分支
3. 提交更改
4. 推送到分支
5. 提交 Pull Request

## 📚 更多资源

- [KRunner 开发文档](https://develop.kde.org/docs/plasma/krunner/)
- [Qt 文档](https://doc.qt.io/)
- [KDE 开发者社区](https://community.kde.org/Get_Involved/development)

## 💬 联系方式
