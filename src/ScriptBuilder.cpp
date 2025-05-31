#include "ScriptBuilder.h"
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QUuid>
#include <QFile>
#include <QTextStream>
#include <QDebug>

void ScriptExecutionInfo::printInfo() const {
    qDebug() << "ScriptExecutionInfo:";
    qDebug() << "  命令/脚本路径:" << commandOrScriptPath;
    qDebug() << "  参数列表:" << (arguments.isEmpty() ? "无" : arguments.join(" "));
    qDebug() << "  工作目录:" << workingDirectory;
    qDebug() << "  结果文件:" << (resultFilePath.isEmpty() ? "无" : resultFilePath);
    qDebug() << "  执行方式:" << (useShell ? "通过 shell 执行" : "直接执行");
}

void ScriptExecutionInfo::printScriptContent() const {
    qDebug() << "脚本/命令内容:";
    if (commandOrScriptPath.isEmpty()) {
        qDebug() << "  未设置命令或脚本路径";
        return;
    }

    if (useShell) {
        if (QFile::exists(commandOrScriptPath)) {
            QFile file(commandOrScriptPath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                QString content = in.readAll();
                qDebug() << "  === 脚本文件内容 ===";
                qDebug().noquote() << content;
                qDebug() << "  ===================";
                file.close();
            } else {
                qDebug() << "  无法打开脚本文件:" << commandOrScriptPath;
            }
        } else {
            qDebug() << "  Shell命令:" << commandOrScriptPath;
        }
    } else {
        qDebug() << "  程序:" << commandOrScriptPath;
        if (!arguments.isEmpty()) {
            qDebug() << "  参数:" << arguments.join(" ");
        }
    }
}

ScriptExecutionInfo ScriptBuilder::build(const CommandDefinition& definition, const QString& queryArgs, const QString& tempFilePath)
{
    ScriptExecutionInfo info;
    info.workingDirectory = resolveWorkingDirectory(definition, queryArgs);

    QString resultFilePath = "";
    QString tempScriptPath = "";
    bool needsScriptFile = false;
    bool needsResultFile = !definition.resultFileTemplate.isEmpty();

    // 确定是否需要临时脚本文件 (如果命令复杂或需要在终端执行脚本)
    // 或者如果需要结果文件且模板依赖于脚本路径
    if (definition.executionMode == CommandDefinition::ExecutionMode::Terminal ||
        definition.commandTemplate.contains('|') || // 包含管道符
        definition.commandTemplate.contains('>') || // 包含重定向符 (简单判断)
        definition.commandTemplate.contains(';') || // 包含命令分隔符
        (needsResultFile && definition.resultFileTemplate.contains("%temp_script%")))
    {
        needsScriptFile = true;
    }

    // 如果需要脚本文件或结果文件，则 tempFilePath 必须有效
    if ((needsScriptFile || needsResultFile) && tempFilePath.isEmpty()) {
        qWarning() << "ScriptBuilder: tempFilePath is required but not provided for definition:" << definition.id;
        // 返回空的 info 表示失败
        return ScriptExecutionInfo();
    }

    if (needsScriptFile) {
        tempScriptPath = tempFilePath; // 使用传入的路径作为脚本路径
    }
    if (needsResultFile) {
        // 解析结果文件路径模板
        resultFilePath = definition.resultFileTemplate;
        resultFilePath.replace("%temp_script%", tempScriptPath); // 替换占位符
        // 如果结果文件模板不依赖脚本路径，但仍需要临时文件
        if (!resultFilePath.contains(tempFilePath) && tempFilePath.isEmpty()) {
             // 这种情况理论上不应发生，因为前面检查了 tempFilePath
             qWarning() << "ScriptBuilder: Logic error in temp file path assignment.";
             return ScriptExecutionInfo();
        } else if (!resultFilePath.contains(tempFilePath)) {
            // 如果结果文件模板是独立的，例如固定名称或基于 UUID
            // 这里假设 resultFileTemplate 已经包含了一个完整的路径或模式
            // 如果 resultFileTemplate 只是一个文件名，需要拼接路径
             if (!QFileInfo(resultFilePath).isAbsolute()) {
                 resultFilePath = QDir(QDir::tempPath()).filePath(resultFilePath);
                 // 可以加入 UUID 保证唯一性，如果模板本身不保证
                 // resultFilePath.replace("%uuid%", QUuid::createUuid().toString());
             }
        }
        info.resultFilePath = resultFilePath;
    }


    // 替换命令模板中的占位符
    QString processedTemplate = substitutePlaceholders(definition.commandTemplate, queryArgs, resultFilePath, tempScriptPath);

    if (needsScriptFile) {
        // --- 需要生成脚本文件 ---
        info.useShell = true; // 标记需要通过 shell 执行脚本
        info.commandOrScriptPath = tempScriptPath; // 要执行的是脚本文件

        QFile scriptFile(tempScriptPath);
        if (!scriptFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "ScriptBuilder: Failed to open temporary script file for writing:" << tempScriptPath;
            return ScriptExecutionInfo(); // 返回空表示失败
        }
        QTextStream out(&scriptFile);
        // 添加 shebang 和必要的设置
        out << "#!/bin/sh\n"; // 使用 /bin/sh 增加兼容性
        out << "set -e\n"; // 出错时退出
        // 可以考虑添加 export LANG=C.UTF-8 等环境变量设置
        out << processedTemplate << "\n";
        scriptFile.close();

        // 设置脚本文件权限为可执行
        if (!scriptFile.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                                       QFile::ReadGroup | QFile::ExeGroup |
                                       QFile::ReadOther | QFile::ExeOther)) {
            qWarning() << "ScriptBuilder: Failed to set executable permissions on script:" << tempScriptPath;
            // 不一定需要返回失败，取决于场景
        }
         qDebug() << "ScriptBuilder: Generated script file:" << tempScriptPath << "for definition:" << definition.id;

    } else {
        // --- 尝试直接执行程序 ---
        QString program;
        QStringList arguments;
        if (tryParseDirectCommand(processedTemplate, program, arguments)) {
            // 可以直接执行，更安全
            info.useShell = false;
            info.commandOrScriptPath = program;
            info.arguments = arguments;
             qDebug() << "ScriptBuilder: Prepared direct command execution for:" << definition.id << "Program:" << program << "Args:" << arguments;
        } else {
            // 无法解析为直接命令，必须通过 shell 执行
            info.useShell = true;
            info.commandOrScriptPath = processedTemplate; // 将整个处理后的模板作为命令字符串
            // arguments 列表为空，因为命令将传递给 sh -c
             qDebug() << "ScriptBuilder: Command requires shell execution for:" << definition.id << "Command:" << processedTemplate;
        }
    }

    // only for debug
    info.printInfo();
    info.printScriptContent();

    return info;
}

/**
 * @brief Resolves the working directory based on command definition and query arguments.
 * 
 * This function determines the appropriate working directory according to the WorkingDirMode
 * specified in the command definition and the provided query arguments.
 *
 * The resolution follows these rules:
 * - QueryOrHome: Uses query arguments as working directory if it's a valid directory path,
 *               otherwise falls back to home directory
 * - Home: Uses the user's home directory
 * - Current: Currently treated same as Home mode due to ambiguity in KRunner context
 * - ExplicitPath: Uses the explicitly defined path, supports "~/" expansion,
 *                 falls back to home directory if path doesn't exist
 *
 * @param definition The command definition containing working directory mode and settings
 * @param queryArgs Query arguments that might contain a directory path
 * @return QString The resolved absolute path of the working directory
 */
QString ScriptBuilder::resolveWorkingDirectory(const CommandDefinition& definition, const QString& queryArgs)
{
    switch (definition.workingDirMode) {
        case CommandDefinition::WorkingDirMode::QueryOrHome: {
            QFileInfo queryInfo(queryArgs);
            // 检查 queryArgs 是否是一个存在的目录路径
            // 注意：这可能不完全可靠，用户可能输入相对路径或不规范路径
            // 简单的检查：如果包含路径分隔符且存在
            if (!queryArgs.isEmpty() && (queryArgs.contains('/') || queryArgs.contains('\\'))) {
                 QDir queryDir(queryArgs);
                 if (queryDir.exists() && QFileInfo(queryArgs).isDir()){
                     return queryDir.absolutePath(); // 返回绝对路径
                 }
            }
            // 否则返回 Home 目录
            return QDir::homePath();
        }
        case CommandDefinition::WorkingDirMode::Home:
            return QDir::homePath();
        case CommandDefinition::WorkingDirMode::Current:
            // KRunner 的 "当前目录" 概念可能不明确，返回 Home 作为安全默认值
            qWarning() << "ScriptBuilder: WorkingDirMode::Current is ambiguous in KRunner context, using Home instead for definition:" << definition.id;
            return QDir::homePath();
        case CommandDefinition::WorkingDirMode::ExplicitPath: {
            QString path = definition.explicitWorkingDirPath;
            // 处理 "~/" 前缀
            if (path.startsWith("~/")) {
                path.replace(0, 1, QDir::homePath());
            }
            QDir explicitDir(path);
             if (explicitDir.exists()){
                 return explicitDir.absolutePath();
             } else {
                  qWarning() << "ScriptBuilder: Explicit working directory does not exist:" << path << "for definition:" << definition.id << ". Falling back to Home.";
                  return QDir::homePath();
             }
        }
    }
    return QDir::homePath(); // 默认返回 Home
}

// 基本的为 Shell 安全引用字符串的方法
// 注意：这个实现非常基础，可能无法处理所有复杂的 Shell 注入场景。
// 对于高安全要求，应使用更健壮的库或方法。
QString ScriptBuilder::quoteForShell(const QString& input)
{
    // 1. 将所有单引号 ' 替换为 '\'' (结束引用，插入转义的单引号，开始新的引用)
    QString escaped = input;
    escaped.replace("'", "'\\''");
    // 2. 用单引号将整个结果包裹起来
    return QString("'%1'").arg(escaped);
}


QString ScriptBuilder::substitutePlaceholders(const QString& commandTemplate, const QString& queryArgs, const QString& resultFilePath, const QString& tempScriptPath)
{
    QString result = commandTemplate;

    result.replace("{FZF_EXTENDS_DIR}", FZF_EXTENDS_DIR);

    // 替换 {query} - 需要进行 Shell 转义/引用！
    // 使用 quoteForShell 进行基本的引用处理
    result.replace("{query}", quoteForShell(queryArgs));

    // 替换 {output_file} - 通常是文件路径，也建议引用以防路径中有特殊字符
    if (!resultFilePath.isEmpty()) {
        result.replace("{output_file}", quoteForShell(resultFilePath));
    }

    // 替换 {temp_script} - 脚本路径，同样建议引用
     if (!tempScriptPath.isEmpty()) {
        result.replace("{temp_script}", quoteForShell(tempScriptPath));
    }



    // 可以添加更多占位符替换，例如 {home}, {user} 等

    return result;
}

bool ScriptBuilder::tryParseDirectCommand(const QString& processedTemplate, QString& program, QStringList& arguments)
{
    // 这是一个简化的解析尝试
    // 假设命令模板是 "program arg1 arg2 ..." 的形式，没有复杂的 shell 元字符

    // 如果包含 shell 特殊字符，则无法直接执行
    if (processedTemplate.contains('|') || processedTemplate.contains('>') ||
        processedTemplate.contains('<') || processedTemplate.contains('&') ||
        processedTemplate.contains(';') || processedTemplate.contains('`') ||
        processedTemplate.contains('$') || // $() 或 ${} 等可能需要 shell
        processedTemplate.contains('*') || // 通配符
        processedTemplate.contains('?') || // 通配符
        processedTemplate.contains('[') || // 通配符
        processedTemplate.contains('~'))   // 波浪号展开需要 shell
    {
        return false;
    }

    // 按空格分割，但需要处理引号内包含空格的情况 (这里简化，不处理引号)
    // 一个更健壮的方法是查找第一个空格，之前的是 program，之后的是 arguments
    // 或者使用 QProcess::splitArgs (如果可用且合适)

    QStringList parts = processedTemplate.split(' ', Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        return false;
    }

    program = parts.first();
    arguments = parts.mid(1);

    // 进一步检查 program 是否看起来像一个可执行文件 (例如，是否包含路径分隔符或在 PATH 中)
    // 这里简化，假设第一个部分就是程序名
    QFileInfo progInfo(program);
    // 如果程序路径是相对的，QProcess 会在 PATH 中查找，所以这里不强制绝对路径
    // 但至少它不能为空
    if (program.isEmpty()) {
        return false;
    }


    return true;
}
