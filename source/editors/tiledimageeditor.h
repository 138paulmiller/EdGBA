#ifndef TILEDIMAGEEDITOR_H
#define TILEDIMAGEEDITOR_H

#include "ui/gba/tiledimageview.h"

#include <QWidget>
#include <QComboBox>
#include <QStringListModel>

#include <editorinterface.h>

class NewNameDialog;

class TiledImageEditor : public QWidget, public EditorInterface
{
    Q_OBJECT

private:
    QColor grid_color;

    bool skip_sync;

    TiledImageModel* tiledimage_model;
    TiledImageView* tiledimage_view;
    QComboBox* tiledimage_names_combo;
    QStringListModel* tiledimage_names_model;

    NewNameDialog* editname_dialog;

public:
    TiledImageEditor(QWidget *parent = nullptr);
    ~TiledImageEditor();

    // Begin EditorInterface
    void setup(MainWindow* window) override;
    void reset() override;
    void reload() override;
    void undo() override;
    void redo() override;
    void zoomIn() override;
    void zoomOut() override;
    // End EditorInterface

protected slots:
    void on_tiledimageAdd(bool);
    void on_tiledimageRemove(bool);
    void on_tiledimageEditName(bool);
    void on_tiledimageLoad(bool);

private:
    class Ui_TiledImageEditor* ui;
    class MainWindow* main_window;
};

#endif // TILEDIMAGEEDITOR_H
