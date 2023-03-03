#include "common.h"
#include "msglog.h"
#include "config.h"
#include <QFileInfo>
#include <QApplication>
#include <QProcessEnvironment>

QString Common::absoluteFile(const QString& in_file)
{
    return QFileInfo(in_file).canonicalFilePath();
}

QString Common::absolutePath(const QString& in_path)
{
    QFileInfo info(in_path);
    QString path;
    if(info.isDir()) // prevent removing the dir name
    {
        path = info.canonicalFilePath();
    }
    else
    {
        path = info.canonicalPath();
    }
    int len = path.size();
    if(len > 0 && !(path[len-1] == '/' || path[len-1] == '\\'))
    {
        path += "/";
    }
    return path;
}

// Get the full path to the exe
QString Common::getExePath(const QString& file_or_path_suffix)
{
    return absolutePath(QApplication::applicationDirPath()) + file_or_path_suffix;
}

QString Common::getSystemVariable(const QString& variable, const QString& default_value)
{
    QString value = Config::get(variable);
    if(value.size() == 0)
    {
#if LOG_VERBOSE
        msgWarn("System") << "Failed to find user config for " << variable << "\n";
        msgWarn("System") << "Falling back to environment variable...\n";
#endif //LOG_VERBOSE
        value = QProcessEnvironment::systemEnvironment().value(variable, "");
        if(value.size() == 0)
        {
            value = default_value;
        }
#if LOG_VERBOSE
        else
        {
            msgError("System") << "Error " << variable << " not set!\n";
        }
#endif
    }
    return value;
}

bool Common::fileExists(const QString& path)
{
    QFileInfo info(path);
    if(info.isFile())
    {
        return info.exists();
    }
    return false;
}

bool Common::dirExists(const QString& path)
{
    QFileInfo info(path);
    if(info.isDir())
    {
        return info.exists();
    }
    return false;
}
