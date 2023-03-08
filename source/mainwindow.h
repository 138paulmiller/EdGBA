#ifndef EDITOR_WINDOW_H
#define EDITOR_WINDOW_H

#include "editorinterface.h"
#include "ui_mainwindow.h"

#include <gba/game.h>
#include <ui/utils.h>
#include <compiler/romcompiler.h>

#include <QString>
#include <QProcess>
#include <QPushButton>
#include <QTabWidget>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    QApplication* app;
    Ui_MainWindow* ui;

private:
    EditContext edit_context;

    bool project_dirname_valid;

    QVector<EditorInterface*> editors;

    QPushButton* clear_message_log_button;
    QTextBrowser* message_log_view;

    bool checkSave();

    void newGame(QString project_path);
    void openGame(QString project_file);
    void saveGame(QString project_file);

    EditorInterface* activeEditor() const;
    void syncActionEnabledState();

    RomCompiler* rom_compiler;
    QProcess* emuprocess;

public:
    MainWindow(QApplication* app);
    ~MainWindow();

    void setupUI(Ui_MainWindow* ui);
    void saveSession();
    void loadSession();
    void markDirty();
    EditContext* getEditContext() { return &edit_context;}

    void closeEvent(QCloseEvent* event) override;

public slots:
    // Toolbar callback
    void on_new();
    void on_open();
    void on_save();
    void on_saveAs();
    void on_quit();
    void on_undo();
    void on_redo();
    void on_zoomIn();
    void on_zoomOut();
    void on_run();

    // Menu
    void on_messageClear(bool);
    void on_setGBAEmulator();
    void on_setDevKitProPath();
    void on_setDarkMode( bool is_dark_mode);
    void on_openBuildSettings();

    // rom callbacks

    void on_romCompileFinished(bool success, QString rom_file);
    void on_emuFinished(int exitCode, QProcess::ExitStatus exitStatus);};

#endif

