#include "CommandRunner.h"
#include "ConfigManager.h"
#include "ScriptBuilder.h"
#include "ResultHandler.h"
#include "CommandDefinition.h"
#include <KRunner/AbstractRunner>
#include <KRunner/RunnerContext>
#include <KRunner/QueryMatch>
#include <KRunner/Action>
#include <KPluginFactory>
#include <KLocalizedString>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QUuid>
#include <QFileInfo>
#include <QTemporaryFile>

K_PLUGIN_CLASS_WITH_JSON(CommandRunner, "metadata.json")

CommandRunner::CommandRunner(QObject *parent, const KPluginMetaData &metaData)
    : KRunner::AbstractRunner(parent, metaData),
      m_configManager(new ConfigManager(this)),
      m_scriptBuilder(new ScriptBuilder()),
      m_resultHandler(new ResultHandler(this))
{
    setObjectName(i18n("Generic Command Runner")); // 插件名称
    setMinLetterCount(1); // 触发词本身可能很短

    init(); // 初始化加载配置等
}

CommandRunner::~CommandRunner()
{
    // 清理 new 出来的对象 (如果 ScriptBuilder 不是成员变量)
    delete m_scriptBuilder;

    // 尝试终止并清理所有仍在运行的进程
    qDebug() << "CommandRunner: Shutting down. Cleaning up running processes...";
    // 使用迭代器或 keys() 遍历 map，因为 cleanupProcess 会修改 map
    QList<QProcess*> processes = m_runningProcesses.keys();
    for (QProcess* process : processes) {
        if (process) { // 检查指针是否有效
            qWarning() << "CommandRunner: Terminating active process during shutdown (PID:" << process->processId() << ")";
            process->disconnect(); // 断开所有信号连接，防止在析构期间触发槽
            process->kill(); // 尝试终止
            cleanupProcess(process); // 清理上下文和临时文件
        }
    }
    m_runningProcesses.clear();
     qDebug() << "CommandRunner: Shutdown complete.";
}

void CommandRunner::init()
{
    m_reloading = true; // 标记开始加载
    m_configManager->loadConfig();

    // 更新触发词 (如果有变化)
    // KRunner 可能需要重新注册触发词，这里简化处理
    // setTriggerWords(...) // 如果 KRunner API 支持动态更新触发词

    m_reloading = false; // 标记加载完成
    qDebug() << "CommandRunner initialized/reloaded. Loaded definitions:" << m_configManager->getCommandDefinitions().count();
}

void CommandRunner::reloadConfiguration()
{
    qDebug() << "CommandRunner: Reloading configuration...";
    init(); // 重新加载配置
}

// 获取动作匹配的图标名称
QString CommandRunner::getActionMatchIcon(const QString& suffix, const QString& defaultIcon) {
    // VSCode 相关动作
    if (suffix == "vscode" || suffix.startsWith("code")) return "com.visualstudio.code";
    // 编辑器
    if (suffix == "kate") return "kate";
    if (suffix == "notepadpluplus") return "notepad-plus-plus";
    // 终端
    if (suffix == "konsole" || suffix == "terminal") return "utilities-terminal";
    // 默认图标
    return defaultIcon;
}

void CommandRunner::match(KRunner::RunnerContext &context)
{
    if (m_reloading || !context.isValid()) {
        return; // 如果正在重载或上下文无效，则跳过
    }

    const QString query = context.query().trimmed();
    const QList<CommandDefinition>& definitions = m_configManager->getCommandDefinitions();

    QList<KRunner::QueryMatch> matches; // 存储匹配结果

    for (const CommandDefinition& def : definitions) {
        for (const QString& trigger : def.triggerWords) {
            // 检查查询是否以触发词开头 (需要空格分隔或查询就是触发词本身)
            if (query == trigger || query.startsWith(trigger + " ")) {
                QString queryArgs;
                if (query.length() > trigger.length()) {
                    queryArgs = query.mid(trigger.length() + 1).trimmed(); // 获取触发词后的参数
                }

                // --- 创建默认匹配项 ---
                KRunner::QueryMatch match(this);
                match.setText(def.name); // 显示命令名称
                match.setSubtext(def.description.isEmpty() ? query : def.description); // 显示描述或原始查询
                match.setIconName(def.icon);
                match.setRelevance(0.8); // 设置相关性 (可以根据匹配质量调整)

                // 将命令 ID 和查询参数编码到数据中
                match.setData(def.id + "|" + queryArgs);
                matches.append(match);

                 // --- 为特定动作创建匹配项 (如果配置了) ---
                 for (auto it = def.specificActions.constBegin(); it != def.specificActions.constEnd(); ++it) {
                     QString suffix = it.key();
                    QString actionName = it.value();

                    KRunner::QueryMatch actionMatch(this);
                    actionMatch.setText(QString("%1 (%2)").arg(def.name).arg(suffix));
                     actionMatch.setSubtext(def.description);
                     actionMatch.setIconName(getActionMatchIcon(suffix, def.icon));
                    actionMatch.setRelevance(match.relevance() - 0.1);
                     actionMatch.setData(def.id + "|" + queryArgs + "|" + suffix);
                     matches.append(actionMatch);
                 }

                goto next_definition;
            }
        }
        next_definition:;
    }

    context.addMatches(matches);
}

void CommandRunner::run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match)
{
    Q_UNUSED(context); // 上下文可能在 run 中不需要

    QString data = match.data().toString();
    QStringList parts = data.split('|'); // 使用之前定义的分隔符

    if (parts.isEmpty()) {
        qWarning() << "CommandRunner: Invalid match data:" << data;
        return;
    }

    QString definitionId = parts[0];
    QString queryArgs = (parts.size() > 1) ? parts[1] : "";
    QString actionSuffix = (parts.size() > 2) ? parts[2] : ""; // 获取动作后缀

    CommandDefinition definition = m_configManager->getCommandDefinitionById(definitionId);

    if (!definition.isValid()) {
        qWarning() << "CommandRunner: Could not find or invalid definition for ID:" << definitionId;
        return;
    }

     qDebug() << "CommandRunner: Running command for definition:" << definition.id
              << "with args:" << queryArgs << "and action suffix:" << actionSuffix;

    executeCommand(definition, queryArgs, actionSuffix);
}

void CommandRunner::executeCommand(const CommandDefinition& definition, const QString& queryArgs, const QString& actionSuffix)
{
    QString tempFilePath; // 用于临时脚本或结果文件

    // 检查是否需要临时文件
    bool needsResultFile = !definition.resultFileTemplate.isEmpty();
    bool needsScriptFile = definition.executionMode == CommandDefinition::ExecutionMode::Terminal ||
                           definition.commandTemplate.contains('|') ||
                           definition.commandTemplate.contains('>') ||
                           definition.commandTemplate.contains(';') ||
                           (needsResultFile && definition.resultFileTemplate.contains("%temp_script%"));

    // 如果需要，生成唯一的临时文件路径 (使用 UUID 保证唯一性)
    if (needsScriptFile || needsResultFile) {
        // 使用 QStandardPaths::writableLocation(QStandardPaths::TempLocation) 获取临时目录
        // 结合 UUID 生成唯一文件名
        QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        if (tempDir.isEmpty()) {
             qWarning() << "CommandRunner: Could not get temporary directory path. Aborting.";
             return; // 无法创建临时文件
        }
        // 确保临时目录存在
        QDir().mkpath(tempDir);

        tempFilePath = QDir(tempDir).filePath(QUuid::createUuid().toString(QUuid::WithoutBraces));
        // 可以根据需要添加后缀，例如 .sh
        if (needsScriptFile) {
            tempFilePath += ".sh";
        }
         qDebug() << "CommandRunner: Generated temporary file path:" << tempFilePath;
    }

    // 使用 ScriptBuilder 构建执行信息
    ScriptExecutionInfo execInfo = m_scriptBuilder->build(definition, queryArgs, tempFilePath);

    if (execInfo.commandOrScriptPath.isEmpty()) {
        qWarning() << "CommandRunner: ScriptBuilder failed to build execution info for definition:" << definition.id;
        m_resultHandler->cleanupTempFile(tempFilePath); // 使用 ResultHandler 的方法
        return;
    }

    // --- 启动进程 ---
    QProcess *process = new QProcess(this); // 设置 parent 为 this，便于管理

    // --- 存储上下文信息 ---
    RunningCommandContext context;
    context.definition = definition;
    context.tempFilePath = tempFilePath; // 存储临时文件路径用于后续清理
    context.originalWorkingDirectory = execInfo.workingDirectory;
    context.actionSuffix = actionSuffix;
    m_runningProcesses.insert(process, context); // 关联进程和上下文

    // --- 连接信号槽 ---
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &CommandRunner::onProcessFinished);
    connect(process, &QProcess::errorOccurred,
            this, &CommandRunner::onProcessErrorOccurred);
     // 只有在后台模式且需要读取 stdout 时才连接读取信号
     if (definition.executionMode == CommandDefinition::ExecutionMode::Background &&
         execInfo.resultFilePath.isEmpty() && // 结果不是来自文件
         definition.resultType != CommandDefinition::ResultType::None) // 且需要处理结果
     {
         connect(process, &QProcess::readyReadStandardOutput,
                 this, &CommandRunner::onProcessReadyReadStandardOutput);
     }
     // 总是连接错误输出信号，用于调试
     connect(process, &QProcess::readyReadStandardError,
             this, &CommandRunner::onProcessReadyReadStandardError);


    // --- 设置工作目录 ---
    process->setWorkingDirectory(execInfo.workingDirectory);
     qDebug() << "CommandRunner: Setting working directory to:" << execInfo.workingDirectory;

    // --- 根据执行模式启动 ---
    if (definition.executionMode == CommandDefinition::ExecutionMode::Terminal) {
        // 启动终端执行脚本
        QString terminalApp = m_resultHandler->getTerminalExecutable(); // 获取配置的终端程序
        QStringList terminalArgs;
        QString scriptToRun = execInfo.commandOrScriptPath; // 这是 .sh 脚本的路径

        // 为不同终端构造参数 (示例)
        if (terminalApp.contains("konsole")) {
            terminalArgs << "--new-tab" << "-e" << scriptToRun;
        } else if (terminalApp.contains("gnome-terminal")) {
             terminalArgs << "--" << scriptToRun; // gnome-terminal 使用 -- 分隔选项和命令
        } else {
             terminalArgs << "-e" << scriptToRun; // 尝试通用的 -e 选项
        }

         qDebug() << "CommandRunner: Starting terminal" << terminalApp << "with args" << terminalArgs;
         process->start(terminalApp, terminalArgs); // 启动终端进程

    } else { // Background 模式
        if (execInfo.useShell) {
            // 需要通过 shell 执行
            QString commandString = execInfo.commandOrScriptPath; // 这可能是脚本路径或命令字符串
             qDebug() << "CommandRunner: Starting via shell: sh -c" << commandString;
             #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                 process->startCommand(commandString); // Qt 6 推荐方式
             #else
                 process->start("sh", QStringList() << "-c" << commandString); // Qt 5 方式
             #endif
        } else {
            // 直接执行程序
             qDebug() << "CommandRunner: Starting directly:" << execInfo.commandOrScriptPath << "with args" << execInfo.arguments;
            process->start(execInfo.commandOrScriptPath, execInfo.arguments);
        }
    }

    // 启动后检查是否立即出错 (例如程序未找到)
    if (process->state() == QProcess::NotRunning) {
         qWarning() << "CommandRunner: Process failed to start immediately for definition:" << definition.id << "Error:" << process->errorString();
         onProcessErrorOccurred(process->error()); // 手动触发错误处理
    } else {
         qDebug() << "CommandRunner: Process started (PID:" << process->processId() << ") for definition:" << definition.id;
    }
}

// --- 信号槽实现 ---

void CommandRunner::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *process = qobject_cast<QProcess*>(sender());
    if (!process) return;

    qDebug() << "CommandRunner: Process finished (PID:" << process->processId() << ") ExitCode:" << exitCode << "ExitStatus:" << exitStatus;

    // 检查进程是否在我们的管理映射中
    if (m_runningProcesses.contains(process)) {
        RunningCommandContext context = m_runningProcesses.value(process);

        // 调用 ResultHandler 处理结果 - 使用结果文件路径
        QString resultFilePath = context.tempFilePath + ".result";
        m_resultHandler->handleResult(exitCode, exitStatus, context.definition,
                                      context.stdoutData,
                                      resultFilePath,
                                      context.originalWorkingDirectory,
                                      context.actionSuffix);

        // 清理此进程相关资源
        cleanupProcess(process);
    } else {
         qWarning() << "CommandRunner: Finished signal received for an unknown process.";
         process->deleteLater(); // 尝试删除未跟踪的进程对象
    }
}

void CommandRunner::onProcessErrorOccurred(QProcess::ProcessError error)
{
    QProcess *process = qobject_cast<QProcess*>(sender());
    if (!process) return;

    qWarning() << "CommandRunner: Process error occurred (PID:" << (process->processId() > 0 ? QString::number(process->processId()) : "N/A")
               << ") Error:" << error << "-" << process->errorString();

    // 检查进程是否在我们的管理映射中
    if (m_runningProcesses.contains(process)) {
        RunningCommandContext context = m_runningProcesses.value(process);
        qWarning() << "CommandRunner: Error occurred for definition:" << context.definition.id;
        // 这里可以添加用户通知 (KNotification)

        // 清理此进程相关资源
        cleanupProcess(process);
    } else {
        qWarning() << "CommandRunner: Error signal received for an unknown process.";
        process->deleteLater(); // 尝试删除未跟踪的进程对象
    }
}

void CommandRunner::onProcessReadyReadStandardOutput()
{
    QProcess *process = qobject_cast<QProcess*>(sender());
    if (!process) return;

    // 检查进程是否在我们的管理映射中，并且需要读取 stdout
    if (m_runningProcesses.contains(process)) {
         RunningCommandContext& context = m_runningProcesses[process]; // 获取引用以修改
         QByteArray newData = process->readAllStandardOutput();
         context.stdoutData.append(newData); // 追加数据
         // qDebug() << "CommandRunner: Read stdout (PID:" << process->processId() << "):" << newData;
    } else {
         // 对于未知进程或不需要读取 stdout 的进程，仍然读取并丢弃，防止管道阻塞
         process->readAllStandardOutput();
    }
}

void CommandRunner::onProcessReadyReadStandardError()
{
     QProcess *process = qobject_cast<QProcess*>(sender());
    if (!process) return;

    QByteArray errorData = process->readAllStandardError();
    // 总是打印错误输出用于调试
    qWarning() << "CommandRunner: Process stderr (PID:" << (process->processId() > 0 ? QString::number(process->processId()) : "N/A") << "):" << QString::fromUtf8(errorData).trimmed();
}


// 清理与特定进程相关的资源
void CommandRunner::cleanupProcess(QProcess* process)
{
    if (!process) return;

    if (m_runningProcesses.contains(process)) {
        RunningCommandContext context = m_runningProcesses.value(process);

        // 1. 清理临时文件 (如果路径存在)
        if (!context.tempFilePath.isEmpty() && QFile::exists(context.tempFilePath)) {
             if (QFile::remove(context.tempFilePath)) {
                 qDebug() << "CommandRunner: Cleaned up temporary file:" << context.tempFilePath;
             } else {
                 qWarning() << "CommandRunner: Failed to clean up temporary file:" << context.tempFilePath;
             }
        }

        // 2. 从映射中移除
        m_runningProcesses.remove(process);
         qDebug() << "CommandRunner: Removed process context (PID:" << process->processId() << "). Remaining processes:" << m_runningProcesses.count();
    } else {
         qWarning() << "CommandRunner: cleanupProcess called for an untracked process.";
    }

    // 3. 安全删除 QProcess 对象
    process->deleteLater();
}

// --- 需要包含 .moc 文件 ---
#include "CommandRunner.moc"

