#include "msglog.h"


MsgLog& msgLog(const QString& tag)
{
    MsgLog::get().tag = tag;
    MsgLog::get().category = MsgCategory::LOG;
    return MsgLog::get();
}

MsgLog& msgWarn(const QString& tag)
{
    MsgLog::get().tag = tag;
    MsgLog::get().category = MsgCategory::WARN;
    return MsgLog::get();
}

MsgLog& msgError(const QString& tag)
{
    MsgLog::get().tag = tag;
    MsgLog::get().category = MsgCategory::ERROR;
    return MsgLog::get();
}

MsgLog& MsgLog::get()
{
    static MsgLog logger;
    return logger;
}

void MsgLog::postMsg(const QString& msg)
{
    QString html = "<l style=\"margin-right:30px\">[" + tag + "]</l>";
    switch(category)
    {
    case MsgCategory::LOG:
        html = msg;
        break;
    case MsgCategory::WARN:
        html = "<span style='color: yellow'>" + msg + "</span>";
        break;
    case MsgCategory::ERROR:
        html = "<span style='color: red'>" + msg + "</span>";
        break;
    }
    html.replace("\n", "<br>");
    emit logPosted(html);
}

