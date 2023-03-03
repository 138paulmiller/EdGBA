#include "sourcefile.h"

#include <QFile>
#include <QTextStream>
#include <QFileInfo>

SourceFile::SourceFile(QString file_path)
    : file_path(file_path)
{
    QFile f(file_path);
    is_dirty = !f.exists();
}

void SourceFile::save()
{
    if(is_dirty)
    {
        is_dirty = false;
        QFile f(file_path);
        if(f.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream text(&f);
            text << content;
            text.flush();
            f.close();
        }
    }
}

QString SourceFile::getName() const
{
    QFileInfo info(file_path);
    return info.fileName();
}

QString SourceFile::getFilePath() const
{
    QFileInfo info(file_path);
    return info.absoluteFilePath();
}

bool SourceFile::isValid() const
{
    return file_path.size();
}

bool SourceFile::isDirty() const
{
    return is_dirty;
}

bool SourceFile::isHeader() const
{
    return QFileInfo(getFilePath()).suffix() == "h";
}

bool SourceFile::rename(QString new_file_name)
{
    QFileInfo info(file_path);
    QString new_file_path = info.absolutePath() + new_file_name;

    QFile f(file_path);
    if(f.rename(new_file_path))
    {
        file_path = new_file_path;
        return true;
    }
    return false;
}

void SourceFile::setContent(const QString& new_content)
{
    is_dirty = true;
    content = new_content;
}

QString& SourceFile::getContent()
{
    if(is_dirty)
    {
        return content;
    }
    else
    {
        content = "";
        QFile f(file_path);
        if(f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream text(&f);
            content = text.readAll();
            text.flush();
            f.close();
        }
    }
    return content;
}
