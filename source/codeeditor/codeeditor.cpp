#include "codeeditor.h"
#include "mainwindow.h"

#include <config.h>
#include <ui/utils.h>
#include <ui/newnamedialog.h>

#include <QMenu>
#include <QAction>
#include <QDebug>
#include <QFileInfo>
#include <QScrollBar>

#include "ui_codeeditor.h"
#include "ui/collapsiblesection.h"

#define CONFIG_SESSION_SOURCEFILES "session_sourcefiles"

CodeEditor::CodeEditor(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui_CodeEditor())
{
    ui->setupUi(this);
    code_view = nullptr;
    source_files_view = nullptr;
    watcher = nullptr;
    source_file = nullptr;
    newname_dialog = nullptr;
    reset();
}

CodeEditor::~CodeEditor()
{
    delete watcher;
    delete ui;
}

void CodeEditor::setup(MainWindow* window)
{   
    main_window = window;
    code_view = ui->code_view;
    font = QFont(code_view->fontInfo().family(), code_view->fontInfo().pixelSize());

    // setup filesystem model
    filesystem_model = new QFileSystemModel(this);
    filesystem_model->setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);

    // setup filesystem view
    source_files_view = ui->tree_source_files;
    source_files_view->expandAll();
    source_files_view->setModel(filesystem_model);
    source_files_view->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    source_files_view->setContextMenuPolicy(Qt::CustomContextMenu);

    for (int i = 1; i < filesystem_model->columnCount(); ++i)
        source_files_view->hideColumn(i);

    QObject::connect(source_files_view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_sourceFileOpenContextMenu(QPoint)));
    QObject::connect(source_files_view, SIGNAL(clicked(QModelIndex)), this, SLOT(on_sourceFileSelect(QModelIndex)));
    QObject::connect(source_files_view, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(on_sourceFileLoad(QModelIndex)));
    QObject::connect(code_view, SIGNAL(textChanged()), this, SLOT(on_sourceFileTextChange()));
    QObject::connect(this, SIGNAL(changeText(QString)), code_view, SLOT(setText(QString)));

    // setup filesystem watcher
    watcher = new QFileSystemWatcher();

    QObject::connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(on_fileChange(QString)));
    QObject::connect(watcher, SIGNAL(directoryChanged(QString)), this, SLOT(on_directoryChange(QString)));
}

void CodeEditor::reset()
{
    if(Game* game = EditorInterface::game())
    {
        selected_path = game->getAbsoluteCodePath();
    }

    loaded_path = "";
    source_file = nullptr;
    skip_selection = false;
    if(code_view)
    {
        code_view->setPlainText("");
    }

    if(source_files_view)
    {
        source_files_view->setModel(filesystem_model);
    }

    resetFileSystemWatcher();
}

void CodeEditor::reload()
{
    QString prev_path = loaded_path;
    if(!QFileInfo(prev_path).exists())
    {
        prev_path = "";
    }
    source_file = nullptr;

    Game* game = EditorInterface::game();
    if(game == nullptr || code_view == nullptr || source_files_view == nullptr)
    {
        return;
    }

    game->reloadSourceFiles();

    QString code_path = game->getAbsoluteCodePath();
    filesystem_model->setRootPath(code_path);

    source_files_view->setRootIndex(filesystem_model->index(code_path));

    if(prev_path.size())
    {
        changeSourceFile(prev_path);
    }
    else
    {
        QString session_sourcefiles = Config::get(CONFIG_SESSION_SOURCEFILES);
        if(session_sourcefiles.size())
        {
            QStringList session_sourcefile_list = session_sourcefiles.split(",");
            foreach(QString session_sourcefile, session_sourcefile_list)
            {
                // TODO: load into a new tab
                //loadSourceFile(session_sourcefile);
                changeSourceFile(session_sourcefile);
            }
        }
        else if(game->source_files.size())
        {
            changeSourceFile(game->source_files.at(0)->getFilePath());
        }

    }

    resetFileSystemWatcher();

    if(watcher)
    {
        watcher->addPath(code_path);
        QDirIterator dir_it(code_path, QDir::NoDotAndDotDot | QDir::Dirs , QDirIterator::Subdirectories);
        while(dir_it.hasNext())
        {
            watcher->addPath(dir_it.next());
        }
    }
}

void CodeEditor::undo()
{

}

void CodeEditor::redo()
{
}

void CodeEditor::zoomIn()
{
    font.setPointSize(font.pointSize()+1);
    source_files_view->setFont(font);
    code_view->setFont(font);
}

void CodeEditor::zoomOut()
{
    font.setPointSize(font.pointSize()-1);
    source_files_view->setFont(font);
    code_view->setFont(font);
}

void CodeEditor::makeUniqueFilePath(QString& file_path)
{
    Game* game = EditorInterface::game();
    if(game == nullptr)
    {
        return;
    }

    QFileInfo info(file_path);
    QString base_name = info.baseName();

    int count = 0;
    foreach(SourceFile* source_file, game->source_files)
    {
        if(file_path == source_file->getFilePath())
        {
            file_path = base_name + QString::number(count) + ".c";
            count++;
        }
    }
}

void CodeEditor::changeSourceFile(const QString &file_path)
{
    Game* game = EditorInterface::game();
    if(game == nullptr)
    {
        return;
    }

    QFileInfo file_info(file_path);
    if (!file_info.exists() || !file_info.isFile())
    {
        return;
    }

    // Save previous source file scroll
    if(source_file)
    {
        QScrollBar* vertical_scroll = code_view->verticalScrollBar();
        file_path_to_line_scroll.insert(source_file->getFilePath(), vertical_scroll->value());
    }

    if(watcher && watcher->files().size())
    {
        watcher->removePaths(watcher->files());
    }

    QString absolute_file_path = file_info.absoluteFilePath();
    source_file = game->findSourceFile(absolute_file_path);
    if(source_file)
    {
        if(watcher)
        {
            watcher->addPath(absolute_file_path);
        }

        emit changeText(source_file->getContent());

        if(code_view && code_view->verticalScrollBar())
        {
            QScrollBar* vertical_scroll = code_view->verticalScrollBar();
            if(file_path_to_line_scroll.contains(absolute_file_path))
            {
                int scroll = file_path_to_line_scroll[absolute_file_path];
                vertical_scroll->setValue(scroll);
            }
        }
    }

    loaded_path = absolute_file_path;

    Config::set(CONFIG_SESSION_SOURCEFILES, loaded_path);
    Config::save();
}

void CodeEditor::resetFileSystemWatcher()
{
    if(watcher && watcher->files().size())
    {
        watcher->removePaths(watcher->files());
    }

    if(watcher && watcher->directories().size())
    {
        watcher->removePaths(watcher->directories());
    }
}

QString CodeEditor::promptNewSourceFileName(QString name, bool /*isDir*/)
{
    newname_dialog = new NewNameDialog(main_window, name);
    QObject::connect(newname_dialog, SIGNAL(checkName(QString,bool&)), this, SLOT(on_sourceFileCheckName(QString,bool&)));

    newname_dialog->exec();
    if(newname_dialog->accepted())
    {
        name = newname_dialog->getName();
        QString illegal="<>:\"|?*";
        for(int i = 0; i < illegal.size(); ++i)
            name = name.remove(illegal[i]);
        main_window->markDirty();
    }

    delete newname_dialog;
    newname_dialog = nullptr;
    return name;
}

void CodeEditor::on_sourceFileCheckName(QString name, bool& ok)
{
    ok = true;
    if(newname_dialog)
    {
        newname_dialog->setName(name);
    }
}


void CodeEditor::on_sourceFileAdd()
{
    QFileInfo file_info(selected_path);
    QDir dir(file_info.isDir() ? file_info.absoluteFilePath() : file_info.absolutePath());

    QString new_name = promptNewSourceFileName(file_info.baseName(), false);
    QFile new_file(dir.filePath(new_name));
    new_file.open(QFile::ReadWrite);
    new_file.close();
}

void CodeEditor::on_sourceFileRemove()
{
    QFile file(QFileInfo(selected_path).absoluteFilePath());
    file.remove();
    if(Game* game = EditorInterface::game())
    {
        selected_path = game->getAbsoluteCodePath();
    }
}

void CodeEditor::on_sourceFileRename()
{
    //TODO: Open New Name Dialog
    QFileInfo file_info(selected_path);

    QString new_name = promptNewSourceFileName(file_info.baseName(), false);

    QFile file(file_info.absoluteFilePath());
    file.rename(new_name);
    if(selected_path == loaded_path)
    {
        reload();
    }
    else
    {
        Game* game = EditorInterface::game();
        if(game )
        {
            game->reloadSourceFiles();
        }
    }
}

void CodeEditor::on_sourceFolderAdd()
{
    QFileInfo file_info(selected_path);
    QDir dir(file_info.isDir() ? file_info.absoluteFilePath() : file_info.absolutePath());

    QString new_name = promptNewSourceFileName(file_info.baseName(), false);
    dir.mkpath(new_name);
}

void CodeEditor::on_sourceFolderRemove()
{
    QDir dir(QFileInfo(selected_path ).absoluteFilePath());
    dir.removeRecursively();
    if(Game* game = EditorInterface::game())
    {
        selected_path = game->getAbsoluteCodePath();
    }
}

void CodeEditor::on_sourceFolderRename()
{
    QFileInfo file_info(selected_path);

    QString new_name = promptNewSourceFileName(file_info.baseName(), false);

    QFile file(file_info.absoluteFilePath());
    file.rename(new_name);
    if(QFileInfo(selected_path).absolutePath() == QFileInfo(loaded_path).absolutePath())
    {
        reload();
    }
    else
    {
        Game* game = EditorInterface::game();
        if(game )
        {
            game->reloadSourceFiles();
        }
    }
}


void CodeEditor::on_sourceFileSelect(QModelIndex index)
{
    selected_path = filesystem_model->filePath(index);
    selected_path.remove('*');

    if(selected_path.size() == 0)
    {
        Game* game = EditorInterface::game();
        if(game)
            selected_path = game->getAbsoluteCodePath();
    }
}

void CodeEditor::on_sourceFileLoad(QModelIndex index)
{
    Game* game = EditorInterface::game();
    if(game == nullptr)
    {
        return;
    }

    QString new_loaded_path = filesystem_model->filePath(index);
    new_loaded_path.remove('*');
    changeSourceFile(new_loaded_path );
}

void CodeEditor::on_sourceFileTextChange()
{
    if(source_file)
    {
        source_file->setContent(code_view->toPlainText());
    }
}

void CodeEditor::on_sourceFileOpenContextMenu(const QPoint &pos)
{
    QFileInfo file_info(selected_path);
    bool is_dir_selected = file_info.exists() && file_info.isDir();
    bool is_file_selected = file_info.exists() && file_info.isFile();

    QMenu contextMenu("Context menu", source_files_view);

    QAction add_file_action("New File", source_files_view);
    QAction remove_file_action("Delete File", source_files_view);
    QAction rename_file_action("Rename File", source_files_view);
    QAction add_folder_action("New Folder", source_files_view);
    QAction remove_folder_action("Delete Folder", source_files_view);
    QAction rename_folder_action("Rename Folder", source_files_view);

    // Detect if folder or file is selected
    contextMenu.addAction(&add_file_action);
    connect(&add_file_action, SIGNAL(triggered()), this, SLOT(on_sourceFileAdd()));

    if(is_file_selected)
    {
        connect(&remove_file_action, SIGNAL(triggered()), this, SLOT(on_sourceFileRemove()));
        contextMenu.addAction(&remove_file_action);

        contextMenu.addAction(&rename_file_action);
        connect(&rename_file_action, SIGNAL(triggered()), this, SLOT(on_sourceFileRename()));
    }

    contextMenu.addAction(&add_folder_action);
    connect(&add_folder_action, SIGNAL(triggered()), this, SLOT(on_sourceFolderAdd()));

    if(is_dir_selected)
    {
        // If not selecting the root code dir
        Game* game = EditorInterface::game();
        if(game && file_info.absoluteFilePath() != game->getAbsoluteCodePath())
        {
            contextMenu.addAction(&remove_folder_action);
            connect(&remove_folder_action, SIGNAL(triggered()), this, SLOT(on_sourceFolderRemove()));

            contextMenu.addAction(&rename_folder_action);
            connect(&rename_folder_action, SIGNAL(triggered()), this, SLOT(on_sourceFolderRename()));
        }

    }

   contextMenu.exec(source_files_view->mapToGlobal(pos));
}


void CodeEditor::on_fileChange(QString path)
{
    QFileInfo info(path);
    if(info.exists() && source_file->getFilePath() == info.absoluteFilePath())
    {
        changeSourceFile(path);
    }
    else
    {
        reload();
    }
}

void CodeEditor::on_directoryChange(QString /*path*/)
{
    reload();
}
