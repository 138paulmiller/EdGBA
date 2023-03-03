#ifndef COMMON_H
#define COMMON_H

#include <QString>

namespace Common
{
    // ensures that the given path has a trailing slash '/'
    QString absolutePath(const QString& in_path);

    QString absoluteFile(const QString& in_path);

    // Get the full path to the exe
    QString getExePath(const QString& file_or_path_suffix);

    QString getSystemVariable(const QString& variable, const QString&  default_value="");

    bool fileExists(const QString& path);
    bool dirExists(const QString& path);
}

#endif // COMMON_H
