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

class MapEditorContext
{
private:
    Game* game;

    // Actively edited assets
    Map* map;
    int selected_bg_index;

    struct BackgroundSelection
    {
        //width and height of tiles in selection rect in tileset image
        int size;

        bool hflip, vflip;

        //selected tiles in the selection box tilexy=[y*size+x]
        QVector<int> tiles;
    };

    BackgroundSelection selections[GBA_BG_COUNT];

public:
    MapEditorContext();
    void reset();

    void selectBackground(int bg_index);
    int getSelectedBackground() const;

    void resizeSelectedBackground(int size_flag);
    void getBackgroundSizeNames(QStringList& names) const;
    QString getBackgroundSizeName(int bg_index) const;

    int getTileSelectionSize() const; // size of tile in pixels
    void setTileSelectionSize(int new_tile_selection_size);

    int getSelectionSize() const;
    void setSelectionSize(int selection_size);
    void selectTiles(int tilex, int tiley);
    bool getCornerSelectedTileXY(int& tilex, int& tiley) const;

    bool getSelectionHFlip() const;
    void setSelectionHFlip(bool is_flipped);
    bool getSelectionVFlip() const;
    void setSelectionVFlip(bool is_flipped);

    void setGame(Game* new_game);

    bool newMap(QString name);
    bool removeMap();
    void setMap(Map* new_map);
    Map* findMap(const QString& name);
    Map* getMap();
    void getMapNames(QStringList& names);

    bool newTilesetFromImage(QString image_filename);
    bool newTileset(QString name);
    bool removeSelectedTileset();
    void setSelectedTileset(Tileset* new_tileset);
    Tileset* findTileset(const QString& name);

    Tileset* getTileset(int bg_index) const;
    QString getTilesetName(int bg_index) const;

    Tileset* getSelectedTileset() const;
    void getTilesetNames(QStringList& names) const;

    bool renderSelectedTiles(QImage& image, int target_size) const;

    void handleTilesetTileClick(int tilex, int tiley);
    void handleMapTileClick(int tilex, int tiley);

    // TODO: add redo/undo buffers here
    void undo();
    void redo();

private:
    void refreshSelection();
    const BackgroundSelection& getSelection() const;
    BackgroundSelection& getSelection();
};

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

    MapEditorContext edit_context;

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
