#ifndef CUSTOMEACTIONCMD_H
#define CUSTOMEACTIONCMD_H

#include <QString>
#include <QStringList>
#include <QProcess>
#include <QDebug>

class CustomeActionCmd 
{
public:
    CustomeActionCmd(const QString& actionToPerform, const QString& resultData);
    ~CustomeActionCmd();

    // 执行自定义动作
    void executeCustomAction();

private:
    // 替换占位符
    QString substitutePlaceholders();
    
    QString m_actionToPerform;
    QString m_arg;
};

#endif // CUSTOMEACTIONCMD_H