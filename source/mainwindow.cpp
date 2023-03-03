#include "mainwindow.h"
#include "config.h"
#include "common.h"
#include "defines.h"
#include "msglog.h"

#include <ui/utils.h>
#include <ui/configmenu.h>

#include <stdio.h>
#include <QMouseEvent>
#include <QLineEdit>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>

MainWindow::MainWindow(QApplication* parent_app)
    : ui(new Ui_MainWindow())
{
    app = parent_app;
    project_dirname_valid = false;
    setWindowTitle(EDGBA_TITLE);
    game = new Game();
    ui->setupUi(this);
    setupUI(ui);
    emuprocess = nullptr;
    rom_compiler = nullptr;
}

MainWindow::~MainWindow()
{
    delete ui;
    delete game;
    if(emuprocess)
    {
        emuprocess->terminate();
        delete emuprocess;
    }
}

/* set up all the signal and slot triggers for each action */
void MainWindow::setupUI(Ui_MainWindow* ui)
{
    editors.clear();
    editors.insert(ui->tabview_editor->indexOf(ui->tab_map), ui->map_editor);
    editors.insert(ui->tabview_editor->indexOf(ui->tab_sprite), ui->sprite_editor);
    editors.insert(ui->tabview_editor->indexOf(ui->tab_code), ui->code_editor);

    for(int i = 0; i < editors.size(); ++i)
    {
        editors[i]->setup(this);
    }
    ui->tabview_editor->setCurrentIndex(0);

    // Create collapsible section's layout
    QVBoxLayout* collapsible_layout = new QVBoxLayout();

    // Setup Message Log
    message_log_view = new QTextBrowser(this);
    QObject::connect(&MsgLog::get(), SIGNAL(logPosted(QString)), message_log_view, SLOT(insertHtml(QString)));

    clear_message_log_button = new QPushButton(this);
    clear_message_log_button->setStyleSheet("QPushButton {border: none;}");
    clear_message_log_button->setText("Clear");
    QObject::connect(clear_message_log_button, SIGNAL(clicked(bool)), this, SLOT(on_messageClear(bool)));

    // Setup the collapsible section
    collapsible_layout->addWidget(message_log_view);
    collapsible_layout->addWidget(clear_message_log_button);

    ui->collapsible_section->setTitle("Message Log");
    ui->collapsible_section->setContentLayout(collapsible_layout);

    // Setup actions
    QObject::connect(ui->action_open, SIGNAL(triggered()), this, SLOT(on_open()));
    QObject::connect(ui->action_save, SIGNAL(triggered()), this, SLOT(on_save()));
    QObject::connect(ui->action_save_as, SIGNAL(triggered()), this, SLOT(on_saveAs()));
    QObject::connect(ui->action_new_game, SIGNAL(triggered()), this, SLOT(on_new()));
    QObject::connect(ui->action_quit, SIGNAL(triggered()), this, SLOT(on_quit()));
    QObject::connect(ui->action_undo, SIGNAL(triggered()), this, SLOT(on_undo()));
    QObject::connect(ui->action_redo, SIGNAL(triggered()), this, SLOT(on_redo()));
    QObject::connect(ui->action_zoom_in, SIGNAL(triggered()), this, SLOT(on_zoomIn()));
    QObject::connect(ui->action_zoom_out, SIGNAL(triggered()), this, SLOT(on_zoomOut()));
    QObject::connect(ui->action_run, SIGNAL(triggered()), this, SLOT(on_run()));
    QObject::connect(ui->action_set_gba_emulator, SIGNAL(triggered()), this, SLOT(on_setGBAEmulator()));
    QObject::connect(ui->action_set_devkitarm_path, SIGNAL(triggered()), this, SLOT(on_setDevKitProPath()));
    QObject::connect(ui->action_toggle_dark_mode, SIGNAL(triggered(bool)), this, SLOT(on_setDarkMode(bool)));
    QObject::connect(ui->action_build_settings, SIGNAL(triggered()), this, SLOT(on_openBuildSettings()));

    // Setup the styles. Set all of the icons
    Utils::setupAction(ui->action_new_game      , ":/edgba/icons/new.png");
    Utils::setupAction(ui->action_open          , ":/edgba/icons/open.png");
    Utils::setupAction(ui->action_save          , ":/edgba/icons/save.png");
    Utils::setupAction(ui->action_save_as       , ":/edgba/icons/save-as.png");
    Utils::setupAction(ui->action_quit          , ":/edgba/icons/quit.png");
    Utils::setupAction(ui->action_undo          , ":/edgba/icons/undo.png");
    Utils::setupAction(ui->action_redo          , ":/edgba/icons/redo.png");
    Utils::setupAction(ui->action_zoom_in       , ":/edgba/icons/zoom-in.png");
    Utils::setupAction(ui->action_zoom_out      , ":/edgba/icons/zoom-out.png");
    Utils::setupAction(ui->action_show_grid     , ":/edgba/icons/show-grid.png");
    Utils::setupAction(ui->action_run           , ":/edgba/icons/play.png");

    syncActionEnabledState();

    // Default to dark mode
    if(Config::get(CONFIG_KEY_DARK_MODE) == "")
        Config::set(CONFIG_KEY_DARK_MODE, QString::number(1));
    if(Config::get(CONFIG_KEY_DARK_MODE).toInt() == 1)
        ui->action_toggle_dark_mode->trigger();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (!checkSave())
    {
        event->ignore();
        return;
    }
    event->accept();
}

bool MainWindow::checkSave()
{
    if(!game->isDirty())
    {
        return true;
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle(EDGBA_TITLE);
    msgBox.setText("Unsaved changes");
    msgBox.setInformativeText(" Do you wish to save before exiting?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    switch (msgBox.exec())
    {
        case QMessageBox::Save:
            on_save();
            return true;
        case QMessageBox::Discard:
            return true;
        case QMessageBox::Cancel:
            return false;
        default:
            return false;
    }
}

void MainWindow::newGame(QString project_path)
{
    project_dirname_valid = false;
    if (project_path != "")
    {
        game->newGame(project_path);
        saveGame(project_path);
    }

    openGame(game->getAbsoluteProjectFile());
}

void MainWindow::openGame(QString project_file)
{
    project_dirname_valid = game->load(project_file);
    syncActionEnabledState();
    for(int i = 0; i < editors.size(); ++i)
    {
        editors[i]->reset();
        editors[i]->reload();
    }

    game->save();
    saveSession();

    msgLog("Game") << "Opened Game " << game->getAbsoluteProjectFile() << "\n";
}

void MainWindow::saveGame(QString project_path)
{
    if(project_path.size() == 0)
    {
        game->save();
        saveSession();
        return;
    }

    project_dirname_valid = false;

    if (project_path  != "")
    {
        if(project_path[project_path.size()-1] != '/' || project_path[project_path.size()-1] != '/')
            project_path += '/';
        QString file_path =  project_path + game->getName() + "." EDGBA_FILE_EXT;
        game->saveAs(file_path);
        project_dirname_valid = true;
    }

    saveSession();

    msgLog("Game") << "Saved Game " << game->getAbsoluteProjectFile() << "\n";
}

void MainWindow::saveSession()
{
    Config::set(CONFIG_KEY_RECENT_PROJECT, game->getAbsoluteProjectFile());
    Config::save();
}

void MainWindow::loadSession()
{
    QString project_file = Config::get(CONFIG_KEY_RECENT_PROJECT);
    openGame(project_file);
}

EditorInterface* MainWindow::activeEditor() const
{
    if(ui && ui->tabview_editor)
    {
        int active_index = ui->tabview_editor->currentIndex();
        if(active_index < editors.size())
        {
            return editors[active_index];
        }
    }
    return nullptr;
}

void MainWindow::syncActionEnabledState()
{
    if(ui == nullptr)
    {
        return;
    }

    bool enabled = game->isValid();
    ui->action_save->setEnabled(enabled);
    ui->action_save_as->setEnabled(enabled);
    ui->action_redo->setEnabled(enabled);
    ui->action_undo->setEnabled(enabled);
    ui->action_show_grid->setEnabled(enabled);
    ui->action_run->setEnabled(enabled);
    ui->action_zoom_in->setEnabled(enabled);
    ui->action_zoom_out->setEnabled(enabled);
    //ui->tabview_editor->setEnabled(enabled);
}

void MainWindow::markDirty()
{
    game->markDirty();
}

void MainWindow::on_new()
{
    if (!checkSave())
    {
        return;
    }

    QString project_path = QFileDialog::getExistingDirectory(this, tr("Select EdGBA Game Directory"));
    if(project_path.size())
    {
        newGame(project_path);
    }
}

void MainWindow::on_open()
{
    if (!checkSave())
    {
        return;
    }
    QString project_file = QFileDialog::getOpenFileName(this, tr("Open EdGBA Game"), "", tr("EdGBA Game(*." EDGBA_FILE_EXT ")"));
    if(project_file.size())
    {
        openGame(project_file);
    }
}

void MainWindow::on_save()
{
    if (project_dirname_valid)
    {
        saveGame("");
    }
    else
    {
        on_saveAs();
    }
}

void MainWindow::on_saveAs()
{
    QString project_path = QFileDialog::getExistingDirectory(this, tr("Save EdGBA Game"));
    if(project_path.size())
    {
        saveGame(project_path);
    }}

void MainWindow::on_quit()
{
    if (checkSave())
    {
        app->exit();
    }
}

void MainWindow::on_undo()
{
    EditorInterface* editor = activeEditor();
    if(editor)
    {
        editor->undo();
    }
}

void MainWindow::on_redo()
{
    EditorInterface* editor = activeEditor();
    if(editor)
    {
        editor->redo();
    }
}

void MainWindow::on_zoomIn()
{
    EditorInterface* editor = activeEditor();
    if(editor)
    {
        editor->zoomIn();
    }
}

void MainWindow::on_zoomOut()
{
    EditorInterface* editor = activeEditor();
    if(editor)
    {
        editor->zoomOut();
    }
}

void MainWindow::on_run()
{
    // TODO:log emulator process
    on_messageClear(true);

    if(emuprocess)
    {
        emuprocess->terminate();
        emuprocess->waitForFinished();
    }

    delete rom_compiler;
    rom_compiler = nullptr;

    rom_compiler = new RomCompiler();
    QObject::connect(rom_compiler, SIGNAL(finished(bool,QString)), this, SLOT(on_romCompileFinished(bool,QString)));

    game->save();
    rom_compiler->build(game);
}

void MainWindow::on_romCompileFinished(bool success, QString rom_file)
{
    delete rom_compiler;
    rom_compiler = nullptr;

    if(!success)
    {
        Utils::popupWarning("Failed to build ROM! Check message log for details.");
        return;
    }

    QString emu_path = Config::get(CONFIG_KEY_GBA_EMU_PATH);
    if(!Common::fileExists(emu_path))
    {
        Utils::popupWarning("No GBA Emulator found!\nPlease check that `Config->Set GBA Emulator` is set to the correct executable.");
        return;
    }

    if(emuprocess == nullptr)
    {
        emuprocess = new QProcess();
        QObject::connect(emuprocess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(on_emuFinished(int,QProcess::ExitStatus)));
    }
    emuprocess->start(emu_path, { rom_file });
}

void MainWindow::on_emuFinished(int exit_code, QProcess::ExitStatus exit_status)
{
    if(exit_status == QProcess::CrashExit)
    {
        msgWarn("ROM") << "Emulator crashed!\n";
    }
    else
    {
        msgLog("ROM") << "Emulator exited code " << exit_code << "\n";
    }
}

void MainWindow::on_messageClear(bool)
{
    message_log_view->clear();
}

void MainWindow::on_setGBAEmulator()
{
    QString filter =
    #ifdef _WIN32
        "Executable (*.exe)"
    #else
        "Executable (*)"
    #endif
    ;
    QString filepath = QFileDialog::getOpenFileName(this, "Choose GBA Emulator", "", filter);
    Config::set(CONFIG_KEY_GBA_EMU_PATH, filepath);
    Config::save();
}

void MainWindow::on_setDevKitProPath()
{         
    QString dirname = QFileDialog::getExistingDirectory(this, "DevkitARM Directory");
    Config::set(CONFIG_KEY_DEVKITPRO_PATH, dirname);
    Config::save();
}

void MainWindow::on_setDarkMode(bool is_dark_mode)
{
    if(!is_dark_mode)
    {
        app->setStyleSheet("");
        return;
    }

    static const QString darkmode_style = ":/edgba/style.qss";
    QFile file(darkmode_style);
    if(file.exists())
    {
        file.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&file);
        app->setStyleSheet(ts.readAll());
    }
    Config::set(CONFIG_KEY_DARK_MODE, QString::number(is_dark_mode ? 1 : 0));
    Config::save();
}

void MainWindow::on_openBuildSettings()
{
    ConfigMenu* config_menu = new ConfigMenu();
    config_menu->show();
}
