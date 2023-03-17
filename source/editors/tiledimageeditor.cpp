#include "tiledimageeditor.h"
#include "ui_tiledimageeditor.h"

#include <mainwindow.h>
#include <ui/utils.h>
#include <ui/misc/newnamedialog.h>
#include <defines.h>

#include <QFileDialog>
#include <QMessageBox>

TiledImageEditor::TiledImageEditor(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui_TiledImageEditor)
{
    grid_color = QColor(255,0,0);
    skip_sync = true;

    ui->setupUi(this);

    editname_dialog = nullptr;
}

TiledImageEditor::~TiledImageEditor()
{
    delete ui;
}

void TiledImageEditor::setup(MainWindow* window)
{
    // Main window
    main_window = window;
}

void TiledImageEditor::reset()
{
}

void TiledImageEditor::reload()
{
    skip_sync = false;
}

void TiledImageEditor::undo()
{}

void TiledImageEditor::redo()
{}

void TiledImageEditor::zoomIn()
{
}

void TiledImageEditor::zoomOut()
{
}

