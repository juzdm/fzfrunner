#pragma once

#include <QObject>
#include <KRunner/AbstractRunner>
#include <KRunner/Action>
#include <KRunner/RunnerContext>
#include <KRunner/QueryMatch>
#include <QProcess>
#include <QMap>
#include <QUuid>
#include "CommandDefinition.h"

// 前置声明
class ConfigManager;
class ScriptBuilder;
class ResultHandler;

// 用于存储正在运行的命令的上下文信息
struct RunningCommandContext {
    CommandDefinition definition;
    QString tempFilePath;
    QString originalWorkingDirectory;
    QString actionSuffix;
    QByteArray stdoutData;
};

// KRunner 插件主类
class CommandRunner : public KRunner::AbstractRunner
{
    Q_OBJECT

public:
    explicit CommandRunner(QObject *parent, const KPluginMetaData &metaData);
    ~CommandRunner() override;

protected:
    void match(KRunner::RunnerContext &context) override;
    void run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match) override;

private slots:
    void reloadConfiguration() override;
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessErrorOccurred(QProcess::ProcessError error);
    void onProcessReadyReadStandardOutput();
    void onProcessReadyReadStandardError();

private:
    void init() override;
    void executeCommand(const CommandDefinition& definition, const QString& queryArgs, const QString& actionSuffix = QString());
    void cleanupProcess(QProcess* process);
    QString getActionMatchIcon(const QString& suffix, const QString& defaultIcon);

    ConfigManager* m_configManager;
    ScriptBuilder* m_scriptBuilder;
    ResultHandler* m_resultHandler;

    QMap<QProcess*, RunningCommandContext> m_runningProcesses;

    bool m_reloading = false;
};
