#ifndef COMMANDRUNNER_H
#define COMMANDRUNNER_H

#include <KRunner/AbstractRunner>
#include <QObject>
#include <QProcess>
#include <QMap>
#include <QUuid> // 用于生成唯一 ID
#include "CommandDefinition.h" // 包含完整的命令定义

// 前置声明
class ConfigManager;
class ScriptBuilder;
class ResultHandler;

// 用于存储正在运行的命令的上下文信息
struct RunningCommandContext {
    CommandDefinition definition;
    QString tempFilePath; // 临时脚本/结果文件的路径
    QString originalWorkingDirectory;
    QString actionSuffix; // 执行时选择的特定动作后缀
    QByteArray stdoutData; // 存储 stdout 数据
};

// KRunner 插件主类
class CommandRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

public:
    // 构造函数
    CommandRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    // 析构函数
    ~CommandRunner() override;

    // KRunner 接口实现: 匹配用户查询
    void match(Plasma::RunnerContext &context) override;
    // KRunner 接口实现: 执行匹配项
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) override;
    // KRunner 接口实现: 重新加载配置 (可选)
    void reloadConfiguration() override;

private slots:
    // QProcess 信号处理槽: 进程完成
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    // QProcess 信号处理槽: 进程出错
    void onProcessErrorOccurred(QProcess::ProcessError error);
    // QProcess 信号处理槽: 读取标准输出
    void onProcessReadyReadStandardOutput();
    // QProcess 信号处理槽: 读取标准错误
    void onProcessReadyReadStandardError();

private:
    // 初始化插件
    void init();
    // 启动命令执行
    void executeCommand(const CommandDefinition& definition, const QString& queryArgs, const QString& actionSuffix);
    // 清理与特定进程相关的资源
    void cleanupProcess(QProcess* process);
    // 获取动作匹配图标
    QString getActionMatchIcon(const QString& suffix, const QString& defaultIcon);

    // 成员变量
    ConfigManager* m_configManager; // 配置管理器
    ScriptBuilder* m_scriptBuilder; // 脚本构建器
    ResultHandler* m_resultHandler; // 结果处理器

    // 存储正在运行的进程及其上下文
    QMap<QProcess*, RunningCommandContext> m_runningProcesses;

    // 标记是否正在重新加载配置，防止重入
    bool m_reloading = false;
};

#endif // COMMANDRUNNER_H
