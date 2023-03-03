#include "configmenu.h"
#include <compiler/romcompiler.h>
#include "config.h"
#include "defines.h"
#include <QPushButton>

ConfigMenu::ConfigMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfigMenu)
{
    setWindowTitle(EDGBA_TITLE " Config");
    ui->setupUi(this);

    build_options[BUILD_CC] = ui->cc;
    build_options[BUILD_LD] = ui->ld;
    build_options[BUILD_OBJCOPY] = ui->objcopy;
    build_options[BUILD_FIX] = ui->fix;
    build_options[BUILD_CFLAGS] = ui->cflags;
    build_options[BUILD_LDFLAGS] = ui->ldflags;
    build_options[BUILD_LIBS] = ui->libs;
    build_options[BUILD_INCLUDES] = ui->includes;
    build_options[BUILD_ARCH]= ui->arch;
    build_options[BUILD_ROM] = ui->rom;
    build_options[BUILD_STEP_COMPILE] = ui->compile_step ;
    build_options[BUILD_STEP_LINK] = ui->link_step;
    build_options[BUILD_STEP_OBJCOPY] = ui->objcopy_step;
    build_options[BUILD_STEP_FIX] = ui->fix_step;
    build_options[BUILD_CUSTOM] = ui->custom;

    foreach(QString key, build_options.keys())
    {
        QLineEdit* line_edit = build_options[key];
        connect(line_edit, SIGNAL(textEdited(QString)), this, SLOT(on_buildChange(QString)));
        line_edit->setText(RomCompiler::getConfig(key));
    }

    connect(ui->reset_defaults, SIGNAL(clicked(bool)), this, SLOT(on_resetBuildDefaults(bool)));
}

ConfigMenu::~ConfigMenu()
{
    delete ui;
}

void ConfigMenu::closeEvent(QCloseEvent* /*event*/)
{
    deleteLater();
}

void ConfigMenu::on_buildChange(QString)
{
    foreach(const QString& key, build_options.keys())
    {
        QLineEdit* line_edit = build_options[key];
        RomCompiler::setConfig(key, line_edit->text());
    }
    RomCompiler::saveConfig();
}

void ConfigMenu::on_resetBuildDefaults(bool)
{
    RomCompiler::resetDefaults();

    foreach(const QString& key, build_options.keys())
    {
        QLineEdit* line_edit = build_options[key];
        line_edit->setText(RomCompiler::getConfig(key));
    }
}
