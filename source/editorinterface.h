#ifndef EDITORINTERFACE_H
#define EDITORINTERFACE_H

#include "defines.h"
#include <gba/game.h>
#include <QObject>

class MainWindow;
class EditorInterface
{
public:
    class EditContext* edit_context;

    virtual ~EditorInterface() = default;
    virtual void setup(MainWindow* window) = 0;
    virtual void reset() = 0;
    virtual void reload() = 0;
    virtual void undo() = 0;
    virtual void redo() = 0;
    virtual void zoomIn() = 0;
    virtual void zoomOut() = 0;
};

class EditContext : public QObject
{
    Q_OBJECT

private:
    class Game* game;

    // assets actively being editted
    class SpriteAnim* spriteanim;
    class SpriteSheet* spritesheet;
    class Tileset* tileset;

    class Map* map;
    int selected_bg_index; //See GBA_BG<N>

    struct TilesetSelection
    {
        //width and height of tiles in selection rect in tileset image
        int size;

        // if selected rect should be flipped
        bool hflip, vflip;

        //selected tiles in the selection box tilexy=[y*size+x]
        QVector<int> tiles;
    };

    // tileset selection per background
    TilesetSelection selections[GBA_BG_COUNT];

public:
    EditContext();
    virtual ~EditContext();
    void reset();

    Game* getGame();
    void setGame(Game* new_game);

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


    // Move to Map Editor
    bool renderSelectedTiles(QImage& image, int target_size) const;

    // Move to Map Editor
    void handleTilesetTileClick(int tilex, int tiley);

    // Move to Map Editor
    void handleMapTileClick(int tilex, int tiley);

    bool newSpriteSheet();
    bool newSpriteSheetFromImage(QString image_filename);
    bool removeSpriteSheet();
    void setSpriteSheet(SpriteSheet* new_spritesheet);
    SpriteSheet* findSpriteSheet(const QString& name);
    SpriteSheet* getSpriteSheet();
    void getSpriteSheetNames(QStringList& names) const;

    bool newSpriteAnim();
    bool removeSpriteAnim();
    SpriteAnim* getSpriteAnim();
    void setSpriteAnim(SpriteAnim* spriteanim);
    void getSpriteAnimNames(QStringList& names) const;
    SpriteAnim* findSpriteAnim(const QString& name);

    void setSpriteSheetSize(int size);

    // TODO: add redo/undo buffers here
    void undo();
    void redo();

private:
    void refreshSelection();
    const TilesetSelection& getSelection() const;
    TilesetSelection& getSelection();
};

#endif // EDITORINTERFACE_H
