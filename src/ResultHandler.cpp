#include "ResultHandler.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QDebug>
#include <QUrl>
#include <QDesktopServices> // 用于 openUrl
#include <QClipboard>       // 用于剪贴板
#include <QGuiApplication>  // 用于访问剪贴板
#include <KIO/Job>          // 用于 KIO::OpenUrlJob (更 KDE 的方式)
#include <KIO/JobUiDelegate>
#include <KConfigGroup>     // 可能需要访问配置来获取终端等
#include <KSharedConfig>    // 同上

#include "CustomeActionCmd.h" // 自定义动作类

ResultHandler::ResultHandler(QObject *parent) : QObject(parent)
{
}

void ResultHandler::handleResult(int processExitCode,
                                 QProcess::ExitStatus processExitStatus,
                                 const CommandDefinition& definition,
                                 const QByteArray& outputData,
                                 const QString& resultFilePath,
                                 const QString& originalWorkingDirectory,
                                 const QString& actionSuffix)
{
    qDebug() << "ResultHandler: Handling result for definition:" << definition.id
             << "ExitCode:" << processExitCode << "ExitStatus:" << processExitStatus;

    // 首先检查进程是否成功退出
    if (processExitStatus != QProcess::NormalExit || processExitCode != 0) {
        qWarning() << "ResultHandler: Process for" << definition.id << "did not exit normally. ExitCode:" << processExitCode << "Status:" << processExitStatus;
        // 根据需要可以添加错误通知 KNotification
        cleanupTempFile(resultFilePath); // 即使失败也要尝试清理
        return;
    }

    // 如果结果类型为 None，则无需进一步处理
    if (definition.resultType == CommandDefinition::ResultType::None && definition.defaultAction == CommandDefinition::DefaultAction::None && actionSuffix.isEmpty()) {
         qDebug() << "ResultHandler: No result processing needed for definition:" << definition.id;
         cleanupTempFile(resultFilePath);
         return;
    }


    QString resultDataStr;
    bool readSuccess = true;

    // 确定结果来源
    if (!resultFilePath.isEmpty()) {
        // 从结果文件读取
        QFile resultFile(resultFilePath);
        if (resultFile.exists() && resultFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&resultFile);
            resultDataStr = in.readAll().trimmed();
            resultFile.close();
            // 去除 ANSI 颜色转义码
            resultDataStr.replace(QRegularExpression("\x1B\\[[0-9;]*[A-Za-z]"), "");
            qDebug() << "ResultHandler: Read result from file:" << resultFilePath << "Data:" << resultDataStr;
        } else {
            qWarning() << "ResultHandler: Failed to open or find result file:" << resultFilePath;
            readSuccess = false;
        }
        // 清理两个临时文件
        cleanupTempFile(resultFilePath);

    } else if (definition.resultType != CommandDefinition::ResultType::None) {
        // 从 stdout 读取
        resultDataStr = QString::fromUtf8(outputData).trimmed();
        qDebug() << "ResultHandler: Read result from stdout. Data:" << resultDataStr;
    } else {
         // ResultType 是 None，但可能有默认动作或特定动作，resultDataStr 保持为空
         qDebug() << "ResultHandler: ResultType is None, resultData is empty.";
    }


    // 如果读取失败且需要结果，则中止
    if (!readSuccess && definition.resultType != CommandDefinition::ResultType::None) {
         qWarning() << "ResultHandler: Aborting due to failed result reading for definition:" << definition.id;
         return;
    }

    // 处理路径结果 (解析相对路径)
    if (definition.resultType == CommandDefinition::ResultType::FilePath ||
        definition.resultType == CommandDefinition::ResultType::DirectoryPath)
    {
        if (!resultDataStr.isEmpty() && !QDir::isAbsolutePath(resultDataStr)) {
            // 将相对路径转换为相对于原始工作目录的绝对路径
            QDir workingDir(originalWorkingDirectory);
            resultDataStr = workingDir.absoluteFilePath(resultDataStr);
             qDebug() << "ResultHandler: Resolved relative path to:" << resultDataStr;
        }
         // 验证路径是否存在
         if (!QFileInfo::exists(resultDataStr)) {
              qWarning() << "ResultHandler: Resolved path does not exist:" << resultDataStr;
              // 根据需要决定是否中止或通知用户
              // return;
              resultDataStr = ""; // 将结果置空，避免后续动作出错
         }
    }


    // 执行最终动作
    performAction(definition, resultDataStr, originalWorkingDirectory, actionSuffix);

}

void ResultHandler::performAction(const CommandDefinition& definition,
                                  const QString& resultData,
                                  const QString& originalWorkingDirectory,
                                  const QString& actionSuffix)
{
     qDebug() << "ResultHandler: Performing action. Suffix:" << actionSuffix << "DefaultAction:" << static_cast<int>(definition.defaultAction) << "ResultData:" << resultData;

    QString actionToPerform = "";

    // 优先检查是否有特定动作后缀匹配
    if (!actionSuffix.isEmpty() && definition.specificActions.contains(actionSuffix)) {
        actionToPerform = definition.specificActions.value(actionSuffix);
        actionToPerform = actionToPerform.replace("{FZF_EXTENDS_DIR}", FZF_EXTENDS_DIR);

         qDebug() << "ResultHandler: Using specific action:" << actionToPerform;
    } else {
        // 没有匹配的后缀，使用默认动作
        switch (definition.defaultAction) {
            case CommandDefinition::DefaultAction::OpenFileOrCD:
                actionToPerform = "OpenFileOrCD"; // 使用内置标识符
                break;
            case CommandDefinition::DefaultAction::CopyToClipboard:
                 actionToPerform = "CopyToClipboard";
                 break;
            case CommandDefinition::DefaultAction::KRunnerQuery:
                 actionToPerform = "KRunnerQuery";
                 break;
            case CommandDefinition::DefaultAction::None:
            default:
                 qDebug() << "ResultHandler: No specific or default action defined.";
                 return; // 没有动作需要执行
        }
         qDebug() << "ResultHandler: Using default action:" << actionToPerform;
    }

    // --- 根据 actionToPerform 执行 ---
    // 注意：这里的 actionToPerform 是我们在配置中定义的字符串标识符

    if (actionToPerform == "OpenFileOrCD") {
        if (!resultData.isEmpty()) {
             // 需要知道终端执行程序路径，这里暂时硬编码或留空
             // 实际应该从 ConfigManager 或 CommandRunner 获取
            actionOpenFileOrCD(resultData, originalWorkingDirectory, getTerminalExecutable());
        } else {
             qWarning() << "ResultHandler: OpenFileOrCD action requested but result data is empty.";
        }
    } else if (actionToPerform == "CopyToClipboard") {
        if (!resultData.isEmpty()) {
            actionCopyToClipboard(resultData);
        } else {
             qWarning() << "ResultHandler: CopyToClipboard action requested but result data is empty.";
        }
    } else if (actionToPerform == "KRunnerQuery") {
         if (!resultData.isEmpty()) {
            actionKRunnerQuery(resultData);
        } else {
             qWarning() << "ResultHandler: KRunnerQuery action requested but result data is empty.";
        }
    } else if (actionToPerform == "OpenFileWithVSCode") { // 示例特定应用动作
         if (!resultData.isEmpty()) {
             actionOpenFileWithApp(resultData, "code"); // 假设 'code' 是 VSCode 的可执行文件名
         } else {
              qWarning() << "ResultHandler: OpenFileWithVSCode action requested but result data is empty or not a file path.";
         }
    } else if (actionToPerform == "OpenFileWithKate") { // 示例特定应用动作
         if (!resultData.isEmpty()) {
             actionOpenFileWithApp(resultData, "kate");
         } else {
              qWarning() << "ResultHandler: OpenFileWithKate action requested but result data is empty or not a file path.";
         }
    } else {
        qWarning() << "ResultHandler: use " << actionToPerform  << " as custom cmd.";
        // 这里可以调用 CustomeActionCmd 来执行自定义命令
        CustomeActionCmd *customCmd = new CustomeActionCmd(actionToPerform, resultData);
        customCmd->executeCustomAction();
    }
    // 警告信息移动到这里，避免每次都输出
    if (actionToPerform != "OpenFileOrCD" && 
        actionToPerform != "CopyToClipboard" && 
        actionToPerform != "KRunnerQuery" && 
        actionToPerform != "OpenFileWithVSCode" && 
        actionToPerform != "OpenFileWithKate") {
        qWarning() << "ResultHandler: Unknown action identifier:" << actionToPerform;
    }
}

// --- 动作的具体实现 ---

void ResultHandler::openDirectoryInTerminal(const QString& path, const QString& terminalExecutable) {
    qDebug() << "ResultHandler: Opening directory in terminal:" << path;
    QProcess *terminalProcess = new QProcess(); // 需要管理生命周期
    QStringList args;
    QString termExec = terminalExecutable;
    if (termExec.isEmpty()) {
        termExec = "konsole"; // 默认回退到 konsole
        qWarning() << "ResultHandler: Terminal executable not configured, defaulting to konsole.";
    }

    // 为不同终端构造参数 (示例)
    if (termExec.contains("konsole")) {
        args << "--workdir" << path;
    } else if (termExec.contains("gnome-terminal")) {
        args << "--working-directory=" << path;
    } else {
        // 其他终端可能需要不同的参数，或者先 cd 再启动 shell
        args << "-e" << QString("sh -c 'cd %1 && exec $SHELL'").arg(quoteForShell(path)); // 通用但可能不完美
    }
    terminalProcess->setProgram(termExec);
    terminalProcess->setArguments(args);
    terminalProcess->setWorkingDirectory(path); // 设置工作目录可能有助于某些终端
    terminalProcess->startDetached(); // 启动并分离，不等待
    delete terminalProcess; // 清理资源，因为我们使用了 startDetached
}

void ResultHandler::actionOpenFileOrCD(const QString& path, const QString& workingDir, const QString& terminalExecutable)
{
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        qWarning() << "actionOpenFileOrCD: Path does not exist:" << path;
        return;
    }

    qDebug() << "actionOpenFileOrCD: Opening file with KIO:" << path;
    KIO::OpenUrlJob *job = new KIO::OpenUrlJob(QUrl::fromLocalFile(path));
    // KIO::JobUiDelegate *delegate = new KIO::JobUiDelegate(); // 可以添加 UI 代理处理错误等
    // job->setUiDelegate(delegate);
    job->start(); // KIO 会处理后续操作和错误报告
}

void ResultHandler::actionCopyToClipboard(const QString& text)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (clipboard) {
        clipboard->setText(text);
        qDebug() << "actionCopyToClipboard: Copied text to clipboard.";
        // 可以考虑发送一个 KNotification 通知用户
    } else {
        qWarning() << "actionCopyToClipboard: Failed to get clipboard instance.";
    }
}

void ResultHandler::actionKRunnerQuery(const QString& text)
{
     qDebug() << "actionKRunnerQuery: Sending query back to KRunner:" << text;
    // 实现方式通常是通过 D-Bus 调用 KRunner 的接口
    // 需要包含 <QDBusInterface> 和 <QDBusConnection>
    // QDBusInterface krunnerInterface("org.kde.krunner", "/App", "org.kde.krunner.App", QDBusConnection::sessionBus());
    // if (krunnerInterface.isValid()) {
    //     krunnerInterface.call("query", text);
    // } else {
    //     qWarning() << "actionKRunnerQuery: Failed to create D-Bus interface to KRunner.";
    // }
     qWarning() << "actionKRunnerQuery: D-Bus call to KRunner not fully implemented in this example.";

}

void ResultHandler::actionOpenFileWithApp(const QString& filePath, const QString& appExecutable)
{
     qDebug() << "actionOpenFileWithApp: Opening" << filePath << "with" << appExecutable;
    if (!QProcess::startDetached(appExecutable, QStringList() << filePath)) {
        qWarning() << "actionOpenFileWithApp: Failed to start" << appExecutable << "for file" << filePath;
        // 可以发送通知
    }
}

// 清理临时文件
void ResultHandler::cleanupTempFile(const QString& filePath)
{
    if (!filePath.isEmpty() && QFile::exists(filePath)) {
        if (QFile::remove(filePath)) {
            qDebug() << "ResultHandler: Successfully removed temporary file:" << filePath;
        } else {
            qWarning() << "ResultHandler: Failed to remove temporary file:" << filePath;
        }
    }
}

// 获取终端执行程序路径 (需要 CommandRunner 提供)
QString ResultHandler::getTerminalExecutable() const {
    // 这是一个占位符，实际实现需要从配置或 CommandRunner 获取
    // 例如，可以从 KSharedConfig 读取
    KSharedConfig::Ptr config = KSharedConfig::openConfig("krunner-fzf-settings");
    KConfigGroup generalGroup = config->group("General"); // 假设有一个 General 组
    return generalGroup.readEntry("TerminalExecutable", "konsole"); // 提供默认值
}

// 为 Shell 安全地引用字符串 (基本实现, 与 ScriptBuilder 中一致)
QString ResultHandler::quoteForShell(const QString& input)
{
    QString escaped = input;
    escaped.replace("'", "'\\''");
    return QString("'%1'").arg(escaped);
}


