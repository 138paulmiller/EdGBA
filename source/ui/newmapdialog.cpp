#include "newmapdialog.h"

#include <mainwindow.h>

#include <stdio.h>
#include <QMouseEvent>
#include <QMessageBox>
#include <QFileDialog>

/* set initial values */
NewMapDialog::NewMapDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui_NewMapDialog)
{
    success = false;
    option = 0;

    ui->setupUi(this);

    /* the new map dialog */
    QObject::connect(ui->actionOK, SIGNAL(triggered()), this, SLOT(on_ok()));
    QObject::connect(ui->actionCancel, SIGNAL(triggered()), this, SLOT(on_cancel()));
    combo = ui->comboBox;

    QObject::connect(ui->actionRegular, SIGNAL(triggered()), this, SLOT(on_regular()));
    QObject::connect(ui->actionAffine, SIGNAL(triggered()), this, SLOT(on_affine()));

    on_regular();
}

void NewMapDialog::on_regular()
{
    combo->clear();
    combo->addItem("32x32");
    combo->addItem("32x64");
    combo->addItem("64x32");
    combo->addItem("64x64");
    affine = false;
}

void NewMapDialog::on_affine()
{
    combo->clear();
    combo->addItem("16x16");
    combo->addItem("32x32");
    combo->addItem("64x64");
    combo->addItem("128x128");
    affine = true;
}

bool NewMapDialog::getMapSelection(bool& out_affine, int& out_width, int& out_height)
{
    if(!success)
    {
        return false;
    }
    out_width = 0;
    out_height = 0;
    out_affine = affine;
    switch (option)
    {
        case 0: if(affine) { out_width =  16; out_height =  16; } else { out_width = 32; out_height = 32; } break;
        case 1: if(affine) { out_width =  32; out_height =  32; } else { out_width = 32; out_height = 64; } break;
        case 2: if(affine) { out_width =  64; out_height =  64; } else { out_width = 64; out_height = 32; } break;
        case 3: if(affine) { out_width = 128; out_height = 128; } else { out_width = 64; out_height = 64; } break;
    }
    return true;
}

void NewMapDialog::on_ok()
{
    success = true;
    option = combo->currentIndex();
    close();
}

void NewMapDialog::on_cancel()
{
    success = false;
    close();
}
