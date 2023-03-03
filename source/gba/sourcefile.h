#ifndef SOURCEFILE_H
#define SOURCEFILE_H

#include "asset.h"

class SourceFile
{
public:
    SourceFile(QString file_path = "");

    void load();
    void save();

    QString getName() const;
    QString getFilePath() const;

    bool isValid() const;
    bool isDirty() const;
    bool isHeader() const;
    bool rename(QString new_file_name);
    void setContent(const QString& content);
    QString& getContent();

private:
    QString content;
    QString file_path;
    bool is_dirty;
};

#endif // SOURCEFILE_H
