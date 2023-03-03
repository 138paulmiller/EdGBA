#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include "codeview.h"

#include <gba/game.h>
#include <editorinterface.h>

#include <QTextBrowser>
#include <QTreeView>
#include <QFileSystemModel>
#include <QFileSystemWatcher>

class CodeEditor : public QWidget, public EditorInterface
{
    Q_OBJECT

private:
    class Ui_CodeEditor* ui;
    MainWindow* main_window;
    QFileSystemWatcher* watcher;
    QFileSystemModel* filesystem_model;
    QTreeView* source_files_view;
    CodeView* code_view;
    SourceFile* source_file; // current source_file being edited
    QString selected_path;
    QString loaded_path;
    bool skip_selection;
    QFont font;
    QMap<QString, int> file_path_to_line_scroll;

    class NewNameDialog* newname_dialog;
public:
    CodeEditor(QWidget *parent);
    ~CodeEditor();

    // Begin EditorInterface
    void setup(MainWindow* window) override;
    void reset() override;
    void reload() override;
    void undo() override;
    void redo() override;
    void zoomIn() override;
    void zoomOut() override;
    // End EditorInterface

protected:
    void makeUniqueFilePath(QString& file_path);
    void changeSourceFile(const QString& file_path);
    void resetFileSystemWatcher();

    QString promptNewSourceFileName(QString name, bool isDir);

signals:
    void changeText(QString file_content);

public slots:
    void on_sourceFileAdd();
    void on_sourceFileRemove();
    void on_sourceFileRename();
    void on_sourceFileSelect(QModelIndex index);
    void on_sourceFileLoad(QModelIndex index);
    void on_sourceFileTextChange();

    void on_sourceFolderAdd();
    void on_sourceFolderRemove();
    void on_sourceFolderRename();

    void on_sourceFileOpenContextMenu(const QPoint &pos);
    void on_sourceFileCheckName(QString name, bool& ok);

    void on_fileChange(QString dir);
    void on_directoryChange(QString dir);

};


#endif // CODEEDITOR_H
