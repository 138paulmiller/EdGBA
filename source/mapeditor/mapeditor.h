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
class QRadioButton;
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

    MapModel* map_model;
    QStringListModel* map_modes_model;
    QStringListModel* map_names_model;

    TilesetModel* tileset_model;
    QStringListModel* tileset_names_model;
    QStringListModel* background_sizes_model;

    QRadioButton* background_buttons[GBA_BG_COUNT];

private:
    // Begin Selection
    struct TilesetSelection
    {
        //width and height of tiles in selection rect in tileset image
        int size;

        // if selected rect should be flipped
        bool hflip, vflip;

        //selected tiles in the selection box tilexy=[y*size+x]
        QVector<int> tiles;
    };

    int selected_bg_index; //See GBA_BG<N>
    // tileset selection per background
    TilesetSelection selections[GBA_BG_COUNT];
    int last_tileset_selection_x, last_tileset_selection_y;

    void selectTiles(int tilex, int tiley);
    bool getCornerSelectedTileXY(int& tilex, int& tiley) const;
    int getTileSelectionSize() const;
    void setTileSelectionSize(int new_tile_selection_size);
    int getSelectionSize() const;
    bool getSelectionHFlip() const;
    void setSelectionHFlip(bool is_flipped);
    bool getSelectionVFlip() const;
    void setSelectionVFlip(bool is_flipped);
    void setSelectionSize(int new_selection_size);
    bool renderSelectedTiles(QImage& image, int target_size) const;
    void refreshSelection();
    TilesetSelection& getSelection();
    const TilesetSelection& getSelection() const;
    void handleTilesetTileClick(int tilex, int tiley);
    void handleMapTileClick(int tilex, int tiley);
    void selectBackground(int bg_index);
    QString getBackgroundSizeName(int bg_index) const;
    int getSelectedBackground() const;
    void resizeSelectedBackground(int size_flag);

    void newTileset(QString name);
    void selectMap(Map* new_map);
    Tileset* getSelectedTileset() const;
    void setSelectedTileset(Tileset* tileset);
    bool removeSelectedTileset();
    // End Selection

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
