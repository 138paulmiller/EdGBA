#ifndef TILEDIMAGEEDITDIALOG_H
#define TILEDIMAGEEDITDIALOG_H

#include "ui/tiledimageview.h"
#include "ui_tiledimageeditdialog.h"

#include <QDialog>

class TiledImageEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TiledImageEditDialog(QWidget *parentr, TiledImageModel* model);
    ~TiledImageEditDialog();

    void reload();

public slots:

    // Toolbar callbacks
    void on_zoomIn();
    void on_zoomOut();
    void on_toggleGrid();

    void on_load();
    void on_rename(QString value);

signals:
    void changed();
    void metadataChanged();

private:
    TiledImage* getTiledImage();

    Ui_TiledImageEditDialog *ui;
    TiledImageModel* model;
};

#endif // TILEDIMAGEEDITDIALOG_H
