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

    tiledimage_model = new TiledImageModel(this);
    tiledimage_view = ui->tiledimage_view;
    tiledimage_view->setModel(tiledimage_model);

    // tiledimage controls
    tiledimage_names_model = new QStringListModel(this);
    tiledimage_names_combo = ui->tiledimage_names;
    tiledimage_names_combo->setModel(tiledimage_names_model);
    QObject::connect(tiledimage_names_combo, SIGNAL(currentTextChanged(QString)), this, SLOT(on_tiledimageSelectionChange(QString)));

    QObject::connect(ui->tiledimage_controls->add, SIGNAL(clicked(bool)), this, SLOT(on_tiledimageAdd(bool)));
    QObject::connect(ui->tiledimage_controls->remove, SIGNAL(clicked(bool)), this, SLOT(on_tiledimageRemove(bool)));
    QObject::connect(ui->tiledimage_controls->edit, SIGNAL(clicked(bool)), this, SLOT(on_tiledimageEditName(bool)));
    QObject::connect(ui->tiledimage_controls->load, SIGNAL(clicked(bool)), this, SLOT(on_tiledimageLoad(bool)));
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
    tiledimage_view->zoomBy(1);
}

void TiledImageEditor::zoomOut()
{
    tiledimage_view->zoomBy(-1);
}

void TiledImageEditor::on_tiledimageAdd(bool)
{
    // Open dialog to pick spritesheet of tileset
}

void TiledImageEditor::on_tiledimageRemove(bool)
{

}

void TiledImageEditor::on_tiledimageEditName(bool)
{

}

void TiledImageEditor::on_tiledimageLoad(bool)
{

}
