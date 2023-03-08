#ifndef MapEditorContext_H
#define MapEditorContext_H

#include <gba/game.h>
#include <editorinterface.h>
#include <tileseteditor/tilesetview.h>

#include "mapview.h"

#include <QStringList>
#include <QStringListModel>

#include <QAction>
#include <QComboBox>

class MapEditor;
class TiledImageEditDialog;
class NewNameDialog;

class Ui_MapEditor;
class MapEditor : public QWidget, public EditorInterface
{
    Q_OBJECT

private:
    Ui_MapEditor* ui;
    MainWindow* main_window;

    //TiledImageEditDialog* tileset_dialog;
    NewNameDialog* editname_dialog;

    QColor grid_color;

    int last_tileset_selection_x, last_tileset_selection_y;

    MapModel* map_model;
    QStringListModel* map_modes_model;
    QStringListModel* map_names_model;

    TilesetModel* tileset_model;
    QStringListModel* tileset_names_model;
    QStringListModel* background_sizes_model;

    class QRadioButton* background_buttons[GBA_BG_COUNT];
public:
    MapEditor(QWidget *parent);
    ~MapEditor();

    void refreshTilesetView();
    void refreshMapView();

    void setTileSize(int tile_size);

    // Begin Editor Interface
    void setup(MainWindow* window) override;
    void reset() override;
    void reload() override;
    void undo() override;
    void redo() override;
    void zoomIn() override;
    void zoomOut() override;
    // End Editor Interface

private:
    int skip_sync;
    void syncUI();
    void syncViews();
    void syncLabels();

signals:
    void mapChanged(QString name);
    void tilesetChanged(QString name);

public slots:

    // Toolbar callbacks
    void on_toggleGrid();

    // Background Callbacks
    void on_backgroundChange(int index);
    void on_backgroundResize(QString value);
    void on_backgroundPriorityChange(int priority);
    void on_clickBackground0(bool /**/);
    void on_clickBackground1(bool /**/);
    void on_clickBackground2(bool /**/);
    void on_clickBackground3(bool /**/);

    // Asset callbacks
    void on_mapTileClick(int tilex, int tiley);
    void on_tilesetTileClick(int tilex, int tiley);

    // Widget callbacks
    void on_mapAdd(bool);
    void on_mapRemove(bool);
    void on_mapEditName(bool /*enabled*/);
    void on_mapCheckName(QString name, bool& ok);
    void on_mapSelectionChange(QString name);
    void on_mapModeChange(QString name);

    void on_tilesetAdd(bool);
    void on_tilesetRemove(bool);
    void on_tileResize(int tile_size);
    void on_tileHFlip(bool is_flipped);
    void on_tileVFlip(bool is_flipped);
    void on_tilesetEditName(bool);
    void on_tilesetCheckName(QString name, bool& ok);
    void on_tilesetLoad(bool);
    void on_tilesetSelectionChange(QString name);
};

#endif // MapEditorContext_H
