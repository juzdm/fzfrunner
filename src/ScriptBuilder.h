#ifndef SCRIPTBUILDER_H
#define SCRIPTBUILDER_H

#include <QString>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include "CommandDefinition.h" // 包含命令定义

// 必须在头文件中注册这些类型
Q_DECLARE_METATYPE(CommandDefinition::WorkingDirMode)
Q_DECLARE_METATYPE(CommandDefinition::ExecutionMode)

// 用于存储构建结果的信息结构体
struct ScriptExecutionInfo {
    QString commandOrScriptPath; // 要执行的程序或脚本路径
    QStringList arguments;       // 传递给程序的参数列表 (如果直接执行程序)
    QString workingDirectory;    // 执行的工作目录
    QString resultFilePath;      // 结果文件的路径 (如果需要)
    bool useShell = false;       // 是否需要通过 shell 执行 (例如 sh -c)

    // 输出执行信息的详细内容
    void printInfo() const;
    
    // 打印脚本内容
    void printScriptContent() const;
};

// 负责构建要执行的命令或脚本
class ScriptBuilder
{
public:
    ScriptBuilder() = default;

    // 构建执行信息
    // queryArgs: 用户在触发词后输入的内容
    // tempFilePath: (可选) 如果需要临时脚本或结果文件，传入生成的唯一路径
    ScriptExecutionInfo build(const CommandDefinition& definition, const QString& queryArgs, const QString& tempFilePath = QString());

protected:
    // 解析工作目录
    QString resolveWorkingDirectory(const CommandDefinition& definition, const QString& queryArgs);
    // 安全地替换占位符
    QString substitutePlaceholders(const QString& commandTemplate, const QString& queryArgs, const QString& resultFilePath, const QString& tempScriptPath);
    // 尝试将模板解析为程序和参数 (更安全的方式)
    bool tryParseDirectCommand(const QString& processedTemplate, QString& program, QStringList& arguments);
     // 为 Shell 安全地引用字符串 (基本实现)
    QString quoteForShell(const QString& input);

private:
    // 声明测试类为友元，这样它可以访问 protected 方法
    friend class ScriptBuilderTest;
};

#endif // SCRIPTBUILDER_H
