#include "ConfigManager.h"
#include <KConfigGroup>
#include <QDebug>

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
{
    // 打开插件的配置文件 (例如 ~/.config/krunner-fzf-settings)
    m_config = KSharedConfig::openConfig("krunner-fzf-settings");
}

void ConfigManager::loadConfig()
{
    m_definitions.clear(); // 清除旧定义

    // 重新加载配置，以防外部修改
    m_config->reparseConfiguration();

    // 获取所有组名
    QStringList groups = m_config->groupList();

    qDebug() << "Loading command runner config. Found groups:" << groups;

    // 遍历所有组，查找命令定义组
    for (const QString &groupName : groups) {
        if (groupName.startsWith(m_commandGroupPrefix)) {
            KConfigGroup group = m_config->group(groupName);
            CommandDefinition definition = parseGroup(group, groupName);
            if (definition.isValid()) {
                m_definitions.append(definition);
                qDebug() << "Loaded definition:" << definition.id << "with trigger:" << definition.triggerWords;
            } else {
                 qWarning() << "Skipping invalid or incomplete definition in group:" << groupName;
            }
        }
    }
     qDebug() << "Finished loading config. Total definitions loaded:" << m_definitions.count();
}

const QList<CommandDefinition>& ConfigManager::getCommandDefinitions() const
{
    return m_definitions;
}

CommandDefinition ConfigManager::getCommandDefinitionById(const QString& id) const
{
    for(const auto& def : m_definitions) {
        if (def.id == id) {
            return def;
        }
    }
    // 返回一个无效的定义如果找不到
    return CommandDefinition();
}


CommandDefinition ConfigManager::parseGroup(const KConfigGroup& group, const QString& groupId)
{
    CommandDefinition def;
    def.id = groupId; // 使用组名作为 ID

    // 读取基本信息，提供默认值
    def.name = group.readEntry("Name", groupId.mid(m_commandGroupPrefix.length())); // 默认使用去除前缀的组名
    def.icon = group.readEntry("Icon", "system-run"); // 默认图标
    def.triggerWords = group.readEntry("TriggerWords", QStringList());
    def.commandTemplate = group.readEntry("CommandTemplate", "");
    def.description = group.readEntry("Description", "");

    // --- 解析枚举类型 ---

    // ExecutionMode
    QString execModeStr = group.readEntry("ExecutionMode", "Background").toLower();
    if (execModeStr == "terminal") {
        def.executionMode = CommandDefinition::ExecutionMode::Terminal;
    } else {
        def.executionMode = CommandDefinition::ExecutionMode::Background; // 默认为 Background
    }

    // WorkingDirMode
    QString workDirModeStr = group.readEntry("WorkingDirectoryMode", "Home").toLower(); // 配置键名建议清晰
    if (workDirModeStr == "queryorhome") {
        def.workingDirMode = CommandDefinition::WorkingDirMode::QueryOrHome;
    } else if (workDirModeStr == "current") {
         def.workingDirMode = CommandDefinition::WorkingDirMode::Current;
    } else if (workDirModeStr == "explicitpath") {
         def.workingDirMode = CommandDefinition::WorkingDirMode::ExplicitPath;
         def.explicitWorkingDirPath = group.readEntry("ExplicitWorkingDirectory", ""); // 读取显式路径
         if (def.explicitWorkingDirPath.isEmpty() && def.workingDirMode == CommandDefinition::WorkingDirMode::ExplicitPath) {
             qWarning() << "WorkingDirMode set to ExplicitPath but ExplicitWorkingDirectory is empty for group:" << groupId << ". Falling back to Home.";
             def.workingDirMode = CommandDefinition::WorkingDirMode::Home;
         }
    } else {
        def.workingDirMode = CommandDefinition::WorkingDirMode::Home; // 默认为 Home
    }
     // 如果用户只提供了 WorkingDirectory 键，为了向后兼容或简化，可以尝试解析它
     if (!group.hasKey("WorkingDirectoryMode") && group.hasKey("WorkingDirectory")) {
         QString workDirValue = group.readEntry("WorkingDirectory", "~");
         if (workDirValue == "~") {
            def.workingDirMode = CommandDefinition::WorkingDirMode::Home;
         } else if (workDirValue == ".") {
             def.workingDirMode = CommandDefinition::WorkingDirMode::Current;
         } else if (workDirValue == "%query_or_home%") { // 定义一个特殊值
             def.workingDirMode = CommandDefinition::WorkingDirMode::QueryOrHome;
         }
         else {
             def.workingDirMode = CommandDefinition::WorkingDirMode::ExplicitPath;
             def.explicitWorkingDirPath = workDirValue;
         }
     }


    // ResultType
    QString resultTypeStr = group.readEntry("ResultType", "None").toLower();
    if (resultTypeStr == "plaintext") {
        def.resultType = CommandDefinition::ResultType::PlainText;
    } else if (resultTypeStr == "filepath") {
        def.resultType = CommandDefinition::ResultType::FilePath;
    } else if (resultTypeStr == "directorypath") {
        def.resultType = CommandDefinition::ResultType::DirectoryPath;
    } else {
        def.resultType = CommandDefinition::ResultType::None; // 默认为 None
    }

    // ResultFileTemplate
    def.resultFileTemplate = group.readEntry("ResultFileTemplate", "");


    // DefaultAction
    QString defaultActionStr = group.readEntry("DefaultAction", "None").toLower();
     if (defaultActionStr == "openfileorcd") {
        def.defaultAction = CommandDefinition::DefaultAction::OpenFileOrCD;
    } else if (defaultActionStr == "copytoclipboard") {
        def.defaultAction = CommandDefinition::DefaultAction::CopyToClipboard;
    } else if (defaultActionStr == "krunnerquery") {
        def.defaultAction = CommandDefinition::DefaultAction::KRunnerQuery;
    } else {
        def.defaultAction = CommandDefinition::DefaultAction::None; // 默认为 None
    }

    // 解析特定动作
    // 假设特定动作的键以 "Action_" 开头，例如 Action_vscode=OpenFileWithVSCode
    QStringList keys = group.keyList();
    for (const QString& key : keys) {
        if (key.startsWith("Action_")) {
            QString suffix = key.mid(7); // 获取 "Action_" 后面的部分作为后缀
            QString actionIdentifier = group.readEntry(key, "");
            if (!suffix.isEmpty() && !actionIdentifier.isEmpty()) {
                def.specificActions[suffix] = actionIdentifier;
            }
        }
    }


    // 基本验证
    if (def.triggerWords.isEmpty() || def.commandTemplate.isEmpty()) {
        qWarning() << "Command definition for group" << groupId << "is missing TriggerWords or CommandTemplate.";
        // 返回一个无效的定义，将在 loadConfig 中被跳过
        return CommandDefinition();
    }


    return def;
}
