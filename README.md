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

### 必需依赖

- KDE Plasma 6.x
- Qt 6.x
- cmake (构建)
- extra-cmake-modules (构建)
- KF6 Runner (构建)
- g++ 或 clang++ (构建)

### 可选依赖

- fzf (用于文件搜索功能)
- fd-find (更快的文件查找)
- bat (文件预览)
- ripgrep (文本搜索)

### 依赖安装指南

#### Arch Linux / Manjaro
```bash
# 安装编译依赖
sudo pacman -S cmake extra-cmake-modules gcc qt6-base kf6 kf6-runner

# 安装可选依赖
sudo pacman -S fzf fd bat ripgrep
```

#### Ubuntu / Debian
```bash
# 安装编译依赖
sudo apt update
sudo apt install cmake extra-cmake-modules g++ \
    qt6-base-dev \
    libkf6runner-dev \
    libkf6config-dev \
    libkf6coreaddons-dev \
    libkf6i18n-dev \
    libkf6service-dev

# 安装可选依赖
sudo apt install fzf fd-find bat ripgrep
```

#### Fedora
```bash
# 安装编译依赖
sudo dnf install cmake extra-cmake-modules gcc-c++ \
    qt6-qtbase-devel \
    kf6-krunner-devel \
    kf6-kconfigcore-devel \
    kf6-kcoreaddons-devel \
    kf6-ki18n-devel \
    kf6-kservice-devel

# 安装可选依赖
sudo dnf install fzf fd-find bat ripgrep
```

## 🚀 安装

### 从源码安装

1. 克隆仓库：
```bash
git clone https://github.com/yourusername/krunner-cmdrunner.git
cd krunner-cmdrunner
```

2. 切换到 KF6 分支（如果使用 Plasma 6）：
```bash
git checkout KF6
```

3. 编译安装：
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
sudo make install
```

4. 重启 KRunner：
```bash
kquitapp6 krunner
kstart6 krunner
```

### 注意事项

- 如果你使用的是 Plasma 6，请确保使用 KF6 分支
- 如果插件安装后没有被发现，可以尝试以下步骤：
  1. 确认插件已正确安装到系统路径
  2. 检查 KRunner 设置中是否启用了该插件
  3. 如果仍然无法找到插件，可以尝试重新登录或重启系统

### 故障排除

如果遇到安装问题：

1. 确保已安装所有必要的依赖
2. 检查 CMake 输出是否有错误信息
3. 确认安装路径是否正确
4. 检查系统日志中是否有相关错误信息

## ⚙️ 配置

配置文件位置：`~/.config/krunner-fzf-settings`

### 配置项详解

#### 通用配置项

| 配置项 | 说明 | 示例 |
|--------|------|------|
| Name | 命令名称 | `Find Files (fzf)` |
| Description | 命令描述 | `使用 fd 和 fzf 交互式查找文件` |
| Icon | 图标名称 | `system-search` |
| TriggerWords | 触发词列表 | `ff, findf` |
| CommandTemplate | 命令模板 | `{FZF_EXTENDS_DIR}/fzf_find_files.sh {query}` |

#### 执行相关配置

| 配置项 | 说明 | 可选值 |
|--------|------|--------|
| ExecutionMode | 执行模式 | Background（后台）/ Terminal（终端） |
| WorkingDirectoryMode | 工作目录模式 | QueryOrHome / Home / Current / ExplicitPath |
| ResultType | 结果类型 | None / PlainText / FilePath / DirectoryPath |
| ResultFileTemplate | 结果文件模板 | `%temp_script%.result` |
| DefaultAction | 默认动作 | None / OpenFileOrCD / CopyToClipboard / KRunnerQuery |

### 动作处理

#### 默认动作类型

1. **None**
   - 不执行任何操作
   - 适用于自处理结果的命令

2. **OpenFileOrCD**
   - 文件：使用默认应用打开
   - 目录：在文件管理器中打开
   - 支持终端中打开目录

3. **CopyToClipboard**
   - 将结果复制到系统剪贴板
   - 支持文本和路径

4. **KRunnerQuery**
   - 将结果作为新的 KRunner 查询
   - 用于链式命令

#### 自定义动作

可以为每个命令定义特定动作：
```ini
Action_动作名=处理方式
```

常用动作示例：
- `Action_vscode=OpenFileWithVSCode`
- `Action_kate=OpenFileWithKate`
- `Action_konsole=/usr/bin/konsole -e bash {FZF_EXTENDS_DIR}/terminal_open.sh {SelectedItem}`

### 模板占位符

| 占位符 | 说明 | 示例 |
|--------|------|------|
| {query} | 用户输入的查询参数 | `find {query}` |
| {FZF_EXTENDS_DIR} | 扩展脚本目录 | `{FZF_EXTENDS_DIR}/script.sh` |
| {output_file} | 输出文件路径 | `> {output_file}` |
| {SelectedItem} | 选中的结果项 | `open {SelectedItem}` |

### 工作目录模式

1. **QueryOrHome**
   - 如果查询是有效路径，使用该路径
   - 否则使用用户主目录

2. **Home**
   - 始终使用用户主目录
   - 适用于全局搜索

3. **Current**
   - 使用 KRunner 当前目录
   - 注意：可能不稳定

4. **ExplicitPath**
   - 使用显式指定的路径
   - 需要在配置中设置 `explicitWorkingDirPath`

### 结果类型

1. **None**
   - 不处理命令输出
   - 适用于自包含命令

2. **PlainText**
   - 处理为纯文本
   - 支持复制到剪贴板

3. **FilePath**
   - 处理为文件路径
   - 支持打开文件和编辑器操作

4. **DirectoryPath**
   - 处理为目录路径
   - 支持文件管理器和终端操作

## 🎯 使用方法

### 基本用法

1. 按 Alt+Space 或配置的快捷键唤起 KRunner
2. 输入配置的触发词和查询参数
3. 选择结果并执行默认动作或选择其他动作

### 常用命令示例

1. 文件搜索（ff）：
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

### 内置命令详解

#### ff/findf（文件查找）
```bash
ff [目录]    # 使用 fzf 交互式搜索文件

功能：
- 使用 fd 和 fzf 进行交互式文件搜索
- 使用 bat 进行文件预览（如果安装）
- 可直接打开文件或在编辑器中编辑

快捷键：
- 回车：打开选中的文件
- Alt+v：在 VSCode 中打开
- Alt+k：在 Kate 中打开
- Alt+c：复制文件路径
- Alt+t：在终端中打开所在目录
- Ctrl+/：切换预览窗口

示例：
  ff                  # 在当前目录下搜索文件
  ff ~/Documents      # 在文档目录下搜索文件
  ff .               # 在当前目录搜索
```

#### fz/findz（文件和目录查找）
```bash
fz [目录]    # 交互式查找文件或目录

功能：
- 在指定目录下浏览目录结构
- 使用 ls 预览目录内容
- 支持快速切换到文件搜索模式

快捷键：
- 回车：打开选中的目录
- Alt+Enter：在选中目录下搜索文件
- Alt+v：在 VSCode 中打开
- Alt+k：在 Kate 中打开
- Alt+t：在终端中打开
- Alt+c：复制路径

示例：
  fz                  # 从主目录开始浏览
  fz ~/Projects       # 浏览项目目录
```

#### tm/tmux（Tmux 会话管理）
```bash
tm [动作]    # 管理 Tmux 会话

动作：
  list          # 列出并管理会话
  new <名称>    # 创建新会话

快捷键（list 模式）：
  Enter：连接到选中的会话
  Ctrl+X：终止选中的会话
  Ctrl+R：刷新会话列表
  Ctrl+H/L：预览窗口滚动
  Ctrl+/：切换预览窗口
  Ctrl+C：退出

示例：
  tm list           # 列出并管理现有会话
  tm new dev        # 创建并连接到名为 "dev" 的新会话
```

#### 其他实用命令

1. **VSCode 最近项目 (code/vr)**
```bash
code [搜索词]   # 浏览并打开 VS Code 最近的项目

功能：
- 从 VS Code 配置中读取最近的项目列表
- 使用 ls 预览项目目录内容
- 快速打开选中的项目

快捷键：
- Enter：打开选中的项目
- Ctrl+R：刷新项目列表
- Ctrl+/：切换预览窗口
- Ctrl+H/L：预览窗口滚动

示例：
  code           # 显示所有最近的项目
```

2. **仓库查找 (fr/repo)**
```bash
fr [搜索词]    # 查找 Git/Repo 仓库

功能：
- 在常用目录下查找 Git/Repo 仓库
- 支持目录预览
- 支持 AOSP 子仓库搜索
- 可快速切换到文件搜索

搜索目录：
- ~/disk
- ~/code
- ~/projects

快捷键：
- Enter：选择仓库
- Alt+G：搜索 AOSP 子仓库
- Alt+Enter：在选中仓库中搜索文件

示例：
  fr            # 列出所有找到的仓库
```

3. **sing-box 管理 (sin/sing-box)**
```bash
sin           # 管理 sing-box 服务

功能：
- 在 tmux 会话中管理 sing-box 服务
- 支持服务的启动和停止
- 自动处理 sudo 权限

选项：
  sing-box-run：
    - 创建新的 tmux 会话（如果不存在）
    - 使用指定配置启动服务
    - 自动请求 sudo 权限
  
  sing-box-stop：
    - 优雅地停止服务
    - 自动关闭 tmux 会话

配置文件：
  ~/.config/sing-box/me.json
```

4. **历史命令搜索 (hist/fh)**
```bash
hist [搜索词]   # 交互式搜索命令历史

功能：
- 使用 fzf 搜索 shell 历史命令
- 支持实时过滤
- 选中后自动复制到剪贴板

示例：
  hist         # 显示所有历史命令
  hist git     # 搜索包含 git 的命令
```

5. **Git 分支切换 (fzb/gitb)**
```bash
fzb           # 交互式切换 Git 分支

功能：
- 列出所有本地分支
- 使用 fzf 交互式选择
- 自动切换到选中的分支

示例：
  fzb         # 显示并选择分支
```

### 命令结果处理

每个命令支持以下默认动作：

1. 文件结果：
   - 回车：打开文件
   - Alt+c：复制文件路径
   - Alt+e：在编辑器中打开
   - Alt+t：在终端中打开所在目录

2. 目录结果：
   - 回车：在文件管理器中打开
   - Alt+c：复制目录路径
   - Alt+t：在终端中打开

3. 文本结果：
   - 回车：复制到剪贴板
   - Alt+e：在编辑器中打开
   - Alt+t：在终端中执行

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

### 系统架构

#### 1. 命令执行流程

1. **命令触发**
   - 用户输入触发词和参数
   - KRunner 匹配相应的命令定义
   - 准备执行环境

2. **命令构建**
   - 解析命令模板
   - 替换占位符
   - 确定工作目录
   - 生成执行信息

3. **执行处理**
   - 后台执行或终端执行
   - 监控进程状态
   - 收集命令输出

4. **结果处理**
   - 根据结果类型处理输出
   - 执行相应的动作
   - 清理临时文件

#### 2. 核心组件

1. **命令运行器 (CommandRunner)**
   - 实现 KRunner 插件接口
   - 管理命令匹配和执行
   - 处理进程生命周期
   - 支持动作系统

2. **配置管理器 (ConfigManager)**
   - 加载和解析配置文件
   - 管理命令定义
   - 支持配置重载
   - 验证配置有效性

3. **脚本构建器 (ScriptBuilder)**
   - 构建执行命令
   - 处理占位符替换
   - 管理工作目录
   - 生成临时文件

4. **结果处理器 (ResultHandler)**
   - 处理命令输出
   - 执行结果动作
   - 管理剪贴板
   - 处理文件操作

### 扩展开发

#### 1. 添加新命令

1. **创建命令定义**
```ini
[Command_NewCommand]
Name=新命令名称
Description=命令描述
Icon=命令图标
TriggerWords=触发词1,触发词2
CommandTemplate=命令模板
ExecutionMode=执行模式
ResultType=结果类型
DefaultAction=默认动作
```

2. **实现命令脚本**
```bash
#!/bin/bash
# 1. 接收参数
# 2. 执行核心逻辑
# 3. 输出结果
```

3. **添加自定义动作**
```ini
Action_custom=CustomActionName
```

#### 2. 开发注意事项

1. **命令模板**
   - 使用占位符引用变量
   - 注意路径和参数转义
   - 考虑错误处理

2. **结果处理**
   - 选择合适的结果类型
   - 实现必要的动作
   - 处理异常情况

3. **安全考虑**
   - 验证用户输入
   - 安全处理文件路径
   - 避免命令注入

4. **性能优化**
   - 减少不必要的进程
   - 优化文件操作
   - 合理使用缓存

### 目录结构

```
.
├── src/                    # 源代码目录
│   ├── CommandRunner.*     # KRunner 插件主类
│   ├── ConfigManager.*     # 配置管理
│   ├── ScriptBuilder.*    # 脚本构建
│   ├── ResultHandler.*    # 结果处理
│   └── CommandDefinition.h # 命令定义结构
├── extends/               # 扩展脚本目录
│   ├── fzf_find_files.sh  # 文件搜索
│   ├── tmux_session.sh    # Tmux 管理
│   └── ...                # 其他脚本
├── config/               # 配置目录
│   └── krunner-fzf-settings  # 默认配置
└── CMakeLists.txt       # CMake 构建文件
```

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

- Issue Tracker: [GitHub Issues](https://github.com/juzdm/fzfrunner/issues)
