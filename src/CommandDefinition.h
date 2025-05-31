#ifndef COMMANDDEFINITION_H
#define COMMANDDEFINITION_H

#include <QString>
#include <QStringList>
#include <QMap>

// 定义命令的配置结构
struct CommandDefinition {
    // 命令的唯一标识符 (例如，配置文件中的组名 "Command_FindFile")
    QString id;
    // 显示在 KRunner 中的名称
    QString name;
    // 显示在 KRunner 中的图标名称
    QString icon;
    // 触发此命令的关键字列表
    QStringList triggerWords;
    // 命令执行模板
    QString commandTemplate;
    // 描述信息
    QString description;

    // 命令执行模式
    enum class ExecutionMode {
        Background, // 后台执行
        Terminal    // 在新终端中执行
    };
    ExecutionMode executionMode = ExecutionMode::Background;

    // 工作目录模式
    enum class WorkingDirMode {
        QueryOrHome,   // 如果查询参数是有效路径则使用它，否则使用 Home
        Home,          // 用户主目录
        Current,       // KRunner 当前目录 (可能不适用或不明确，谨慎使用)
        ExplicitPath   // 使用指定的 explicitWorkingDirPath
    };
    WorkingDirMode workingDirMode = WorkingDirMode::Home;
    // 显式指定的工作目录路径 (当 workingDirMode 为 ExplicitPath 时使用)
    QString explicitWorkingDirPath;

    // 预期结果类型
    enum class ResultType {
        None,       // 无预期输出或不处理
        PlainText,  // 纯文本输出
        FilePath,   // 单个文件路径
        DirectoryPath // 单个目录路径
        // 可以扩展更多类型，如 CommandText (执行的命令文本)
    };
    ResultType resultType = ResultType::None;

    // 结果文件模板 (可选, 例如 "%temp_script%.result")
    // 如果为空，则结果默认从 stdout 读取
    QString resultFileTemplate;

    // 默认动作 (当没有指定特定动作后缀时执行)
    enum class DefaultAction {
        None,          // 不执行任何操作
        OpenFileOrCD,  // 打开文件或切换到目录
        CopyToClipboard, // 复制结果到剪贴板
        KRunnerQuery   // 将结果作为新查询发送给 KRunner
    };
    DefaultAction defaultAction = DefaultAction::None;

    // 特定动作映射 (例如: "vscode" -> "OpenFileWithVSCode")
    // 键是 QueryMatch 数据中使用的后缀，值是动作标识符 (可自定义)
    QMap<QString, QString> specificActions;

    // 检查定义是否有效 (至少需要 id 和 triggerWords)
    bool isValid() const {
        return !id.isEmpty() && !triggerWords.isEmpty();
    }
};

#endif // COMMANDDEFINITION_H
