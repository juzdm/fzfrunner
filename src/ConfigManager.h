#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QList>
#include <KConfigCore/KSharedConfig>
#include <KConfigCore/KConfigGroup>
#include "CommandDefinition.h"

// 负责加载和解析插件配置
class ConfigManager : public QObject
{
    Q_OBJECT
public:
    explicit ConfigManager(QObject *parent = nullptr);

    // 加载配置
    void loadConfig();

    // 获取所有加载的命令定义
    const QList<CommandDefinition>& getCommandDefinitions() const;

    // 根据 ID 获取命令定义
    CommandDefinition getCommandDefinitionById(const QString& id) const;


private:
    // 解析单个配置组
    CommandDefinition parseGroup(const KConfigGroup& group, const QString& groupId);

    // 指向共享配置文件的指针
    KSharedConfig::Ptr m_config;
    // 存储所有解析后的命令定义
    QList<CommandDefinition> m_definitions;
    // 配置文件中命令组的前缀
    const QString m_commandGroupPrefix = "Command_";
};

#endif // CONFIGMANAGER_H
