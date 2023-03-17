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

    TiledImageModel* image_model;
    TiledImageView* image_view;
    QComboBox* image_names_combo;
    QStringListModel* image_names_model;

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

private:
    class Ui_TiledImageEditor* ui;
    class MainWindow* main_window;
};

#endif // SPRITEEDITOR_H
