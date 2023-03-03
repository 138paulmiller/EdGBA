#ifndef MSGLOG_H
#define MSGLOG_H

#include <QObject>
#include <QString>
#include <QMutex>
#include <QTextStream>

// Primary API. Usage msgLog() << "Message";
class MsgLog;
MsgLog& msgLog(const QString& tag);
MsgLog& msgWarn(const QString& tag);
MsgLog& msgError(const QString& tag);

enum class MsgCategory
{
    LOG, WARN, ERROR
};

class MsgLog : public QObject
{
    Q_OBJECT
public:
    static MsgLog& get();

private:
    QMutex mutex;
    QString tag;
    MsgCategory category;

    void postMsg(const QString& msg);

private:
    friend MsgLog& msgLog(const QString& tag);
    friend MsgLog& msgWarn(const QString& tag);
    friend MsgLog& msgError(const QString& tag);

    template<typename Type>
    friend MsgLog& operator<<(MsgLog& logger, const Type& arg);

signals:
    void logPosted(QString msg);
};

template<typename Type>
MsgLog& operator<<(MsgLog& logger, const Type& arg)
{
    QString msg;
    QTextStream stream(&msg);
    stream << arg;
    logger.mutex.lock();
    logger.postMsg(msg);
    logger.mutex.unlock();
    return logger;
}

#endif // MSGLOG_H
