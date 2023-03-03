#include "tiledimageeditdialog.h"
#include "mainwindow.h"

#include <ui/utils.h>

#include <QFileDialog>

TiledImageEditDialog::TiledImageEditDialog(QWidget *parent, TiledImageModel* model)
    : QDialog(parent)
    , ui(new Ui_TiledImageEditDialog)
    , model(model)
{
    setWindowTitle("Tileset Edit");

    ui->setupUi(this);

    // setup tileset ui
    ui->view->setModel(model);
    ui->view->setGridEnabled(false);

    QObject::connect(ui->lineedit_name, SIGNAL(textChanged(QString)), this, SLOT(on_rename(QString)));
    QObject::connect(ui->button_import, SIGNAL(clicked(bool)), this, SLOT(on_load()));

    // Setup styles
    Utils::setupIconButton(ui->button_import, ":/icons/add-image.png");

    reload();
}

TiledImageEditDialog::~TiledImageEditDialog()
{
    delete ui;
}

void TiledImageEditDialog::reload()
{
    if(ui == nullptr)
    {
        return;
    }

    TiledImage* tiled_image = getTiledImage();
    if(tiled_image == nullptr)
    {
        return;
    }

    if(ui->lineedit_name)
    {
        ui->lineedit_name->setText(tiled_image->getName());
    }
    ui->view->redraw();
}

void TiledImageEditDialog::on_zoomIn()
{}

void TiledImageEditDialog::on_zoomOut()
{}

void TiledImageEditDialog::on_toggleGrid()
{}

void TiledImageEditDialog::on_load()
{
    QString image_filename = QFileDialog::getOpenFileName(this, tr("Load Image"), "", tr("Image Files (*.png)"));
    QImage image;
    if(!image.load(image_filename))
    {
        //TODO: Warn user of failure
        return;
    }

    if(image.height() * image.width() > GBA_TILESET_WIDTH * GBA_TILESET_HEIGHT)
    {
        //Warn user via message prompt!!!!
        qDebug() << "[EdGBA] Image is too large!"
                 << "Max is image size is (" << GBA_TILESET_HEIGHT  << "x" << GBA_TILESET_HEIGHT << ")";
    }

    TiledImage* tiled_image = getTiledImage();
    if(tiled_image)
    {
        tiled_image->loadFromImage(image);
    }
    reload();
    emit changed();
}

void TiledImageEditDialog::on_rename(QString value)
{
    TiledImage* tiled_image = getTiledImage();
    if(tiled_image)
    {
        tiled_image->setName(value);
    }
    emit metadataChanged();
}

TiledImage* TiledImageEditDialog::getTiledImage()
{
    if(model == nullptr)
    {
        return nullptr;
    }
    return model->getTiledImage();
}
