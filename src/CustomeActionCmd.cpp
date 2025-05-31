#include "CustomeActionCmd.h"
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QDebug>

CustomeActionCmd::~CustomeActionCmd()
{
    // 构造函数
}

CustomeActionCmd::CustomeActionCmd(const QString& actionToPerform, const QString& resultData)
{
    m_actionToPerform = actionToPerform;
    m_arg = resultData;
}

/**
 * @brief 执行自定义动作的主要方法
 * 
 * 该方法负责：
 * 1. 替换命令模板中的占位符
 * 2. 解析命令行字符串为程序和参数
 * 3. 启动新进程执行命令
 */
void CustomeActionCmd::executeCustomAction()
{
    qDebug() << "开始执行自定义动作";
    qDebug() << "- 动作模板:" << m_actionToPerform;
    qDebug() << "- 选中项:" << m_arg;

    QString command = substitutePlaceholders();

    if (command.isEmpty()) {
        qWarning() << "命令替换占位符后为空，终止执行";
        return;
    }

    QStringList commandParts = command.split(' ', Qt::SkipEmptyParts);
    if (commandParts.isEmpty()) {
        qWarning() << "命令行解析失败:" << command;
        return;
    }

    QString program = commandParts.takeFirst();
    QStringList arguments = commandParts;

    QProcess::startDetached(program, arguments);
}


/**
 * @brief 替换命令模板中的占位符
 * 
 * 当前支持的占位符：
 * - {SelectedItem}: 被选中的项目路径或文本
 * 
 * @return QString 替换完成的命令字符串
 */
QString CustomeActionCmd::substitutePlaceholders()
{

    if (m_actionToPerform.isEmpty()) {
        qWarning() << "CustomeActionCmd: m_actionToPerform is empty.";
        return QString();
    }

    if (m_actionToPerform.contains("{SelectedItem}")){
        if (m_arg.isEmpty()) {
            return QString();
        } else {
            m_actionToPerform.replace("{SelectedItem}", m_arg);
        }
    }
    m_actionToPerform.replace("{FZF_EXTENDS_DIR}", FZF_EXTENDS_DIR);

    // 替换占位符
    // 这里可以添加具体的替换逻辑
    return m_actionToPerform;
}