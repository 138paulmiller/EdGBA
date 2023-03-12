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

public:
    EditContext();
    virtual ~EditContext();
    void reset();

    Game* getGame();
    void setGame(Game* new_game);

    bool newMap(QString name);
    bool removeMap();
    void setMap(Map* new_map);
    Map* findMap(const QString& name);
    Map* getMap();
    void getMapNames(QStringList& names);

    bool loadTilesetFromImage(Tileset* tileset, QString image_filename);
    Tileset* newTileset(QString name);
    void removeTileset(Tileset *tileset);
    void replaceTileset(Tileset *tileset, Tileset *new_tileset);
    Tileset* findTileset(const QString& name);

    Tileset* getTileset(int bg_index) const;
    void setTileset(int bg_index, Tileset* tileset) const;
    QString getTilesetName(int bg_index) const;
    void getTilesetNames(QStringList& names) const;

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
};

#endif // EDITORINTERFACE_H
