#include "assetcontrols.h"
#include "ui_assetcontrols.h"
#include <ui/utils.h>

AssetControls::AssetControls(QWidget* parent):
    QWidget(parent),
    ui(new Ui_AssetControls),
    add(nullptr),
    remove(nullptr),
    edit(nullptr)
{
    ui->setupUi(this);

    add = ui->add;
    remove = ui->remove;
    edit = ui->edit;
    load = ui->load;

    Utils::setupIconButton(add,     ":/edgba/icons/add.png");
    Utils::setupIconButton(remove,  ":/edgba/icons/delete.png");
    Utils::setupIconButton(edit,    ":/edgba/icons/edit.png");
    Utils::setupIconButton(load,    ":/edgba/icons/load.png");
}

AssetControls::~AssetControls()
{
    delete ui;
}
