#ifndef RESULTHANDLER_H
#define RESULTHANDLER_H

#include <QObject>
#include <QString>
#include <QProcess> // 需要包含 QProcess::ExitStatus
#include <KIO/OpenUrlJob>
#include <KIO/CommandLauncherJob>
#include "CommandDefinition.h" // 包含命令定义

// 负责处理已完成进程的结果
class ResultHandler : public QObject
{
    Q_OBJECT
public:
    explicit ResultHandler(QObject *parent = nullptr);

    // 处理结果
    // processExitCode: 进程退出码
    // processExitStatus: 进程退出状态
    // definition: 执行的命令定义
    // outputData: 从 stdout 读取的数据 (如果适用)
    // resultFilePath: 结果文件的路径 (如果适用)
    // originalWorkingDirectory: 命令执行时的原始工作目录 (用于解析相对路径)
    // actionSuffix: 从 QueryMatch 数据中提取的特定动作后缀 (例如 "vscode")
    void handleResult(int processExitCode,
                      QProcess::ExitStatus processExitStatus,
                      const CommandDefinition& definition,
                      const QByteArray& outputData,
                      const QString& resultFilePath,
                      const QString& originalWorkingDirectory,
                      const QString& actionSuffix);

    // 获取配置中的终端执行程序 (需要从外部传入或通过 ConfigManager 获取)
    // 这里暂时留空，需要在 CommandRunner 中处理
    QString getTerminalExecutable() const;

    // 清理临时文件
    void cleanupTempFile(const QString& filePath);

private:
    // 执行具体的 KRunner 动作
    void performAction(const CommandDefinition& definition,
                       const QString& resultData, // 处理后的结果字符串 (路径或文本)
                       const QString& originalWorkingDirectory,
                       const QString& actionSuffix);

    // 动作的具体实现
    void actionOpenFileOrCD(const QString& path, const QString& workingDir, const QString& terminalExecutable); // 需要终端执行程序路径
    void actionCopyToClipboard(const QString& text);
    void actionKRunnerQuery(const QString& text);
    void actionOpenFileWithApp(const QString& filePath, const QString& appExecutable); // 例如 VSCode, Kate

    // Shell 安全引用
    QString quoteForShell(const QString& input);

    void openDirectoryInTerminal(const QString& path, const QString& terminalExecutable);

    void executeCustomAction(const QString& actionToPerform, const QString& resultData);
};

#endif // RESULTHANDLER_H
