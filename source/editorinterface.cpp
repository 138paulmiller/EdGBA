#include "editorinterface.h"
#include "mainwindow.h"

#include <QMessageBox>

EditContext::EditContext()
{
    game = nullptr;
    reset();
}

EditContext::~EditContext()
{}

void EditContext::reset()
{
    if(game == nullptr)
    {
        tileset = nullptr;
        spriteanim = nullptr;
        spritesheet = nullptr;
        map = nullptr;
        return;
    }

    const int bg_index = 0;
    QList<Tileset*> tilesets = game->getAssets<Tileset>();
    if(tilesets.size())
        setTileset(bg_index, tilesets[0]);
    else
        setTileset(bg_index, nullptr);

    QList<Map*> maps = game->getAssets<Map>();
    if(maps.size())
        setMap(maps[0]);
    else
        setMap(nullptr);

    QList<SpriteSheet*> spritesheets = game->getAssets<SpriteSheet>();
    if(spritesheets.size())
        setSpriteSheet(spritesheets[0]);
    else
        setSpriteSheet(nullptr);

    QList<SpriteAnim*> spriteanims  = game->getAssets<SpriteAnim>();
    if(spriteanims.size())
        setSpriteAnim(spriteanims[0]);
    else
        setSpriteAnim(nullptr);
}

Game* EditContext::getGame()
{
    return game;
}

void EditContext::setGame(Game* new_game)
{
    game = new_game;
    if(game == nullptr)
    {
        return;
    }
    reset();
}

bool EditContext::newMap(QString name)
{
    Map* new_map = game->addAsset<Map>();
    if(new_map)
        new_map->setName(name);
    setMap(new_map);
    return getMap() != nullptr;
}

bool EditContext::removeMap()
{
    game->removeAsset<Map>(map);

    Map* map = nullptr;
    QList<Map*> maps = game->getAssets<Map>();
    if(maps.size())
    {
        map = maps[0];
    }
    setMap(map);
    return true;
}

bool EditContext::loadTilesetFromImage(Tileset* tileset, QString image_filename)
{
    if(image_filename.isNull() || image_filename.isEmpty())
    {
        return false;
    }

    QImage image;
    if(!image.load(image_filename))
    {
        //TODO: Warn user of failure
        return false;
    }


    if(image.height() * image.width() > GBA_TILESET_MAX_SIZE)
    {
        QString msg = "Tilesets are limited to " + QString::number(GBA_TILE_MAX) + " 8x8 tiles.\nThe max file dimension is "  + QString::number(GBA_TILESET_WIDTH) + "x" + QString::number(GBA_TILESET_HEIGHT);
        QMessageBox msgBox;
        msgBox.setWindowTitle(EDGBA_TITLE);
        msgBox.setText("Tileset too large!");
        msgBox.setInformativeText(msg);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
    }

    bool rename = false;
    if(tileset == nullptr)
    {
        rename = true;
        tileset = game->addAsset<Tileset>();
    }

    if(tileset->loadFromImage(image))
    {
        if(rename)
        {
            QFileInfo fileInfo(image_filename);
            tileset->setName(fileInfo.baseName());
        }

        game->rebuildPalettes();
        return true;
    }
    return false;
}

Tileset* EditContext::newTileset(QString name)
{
    if(game == nullptr)
        return nullptr;

    Tileset* tileset = game->addAsset<Tileset>();
    if(tileset)
        tileset->setName(name);

    return tileset;
}

void EditContext::removeTileset(Tileset *tileset)
{
    if(map)
        map->removeTileset(tileset);
    if(game)
        game->removeAsset<Tileset>(tileset);
}

void EditContext::replaceTileset(Tileset *tileset, Tileset *new_tileset)
{
    foreach(Map* map, game->getAssets<Map>())
    {
        map->replaceTileset(tileset, new_tileset);
    }
    removeTileset(tileset);
}

void EditContext::setMap(Map* new_map)
{
    map = new_map;
}

Map* EditContext::findMap(const QString& name)
{
    return game->findAsset<Map>(name);
}

Map* EditContext::getMap()
{
    return map;
}

void EditContext::getMapNames(QStringList& names)
{
    foreach(Map* map, game->getAssets<Map>())
    {
        if(map)
        {
            names << map->getName();
        }
    }
}

Tileset* EditContext::findTileset(const QString& name)
{
    return game->findAsset<Tileset>(name);
}

Tileset* EditContext::getTileset(int bg_index) const
{
    if(map == nullptr)
    {
        return nullptr;
    }
    return map->getTileset(bg_index);
}

void EditContext::setTileset(int bg_index, Tileset* tileset) const
{
    if(map != nullptr)
    {
        map->setTileset(bg_index, tileset);
    }
}

QString EditContext::getTilesetName(int bg_index) const
{
    Tileset* tileset = getTileset(bg_index);
    if(tileset == nullptr)
    {
        return EMPTY_TILESET_NAME;
    }
    return tileset->getName();
}

void EditContext::getTilesetNames(QStringList& names) const
{
    foreach(Tileset* tileset, game->getAssets<Tileset>())
    {
        if(tileset)
        {
            names << tileset->getName();
        }
    }
    names << EMPTY_TILESET_NAME;
}

bool EditContext::newSpriteSheet()
{
    SpriteSheet* spritesheet = game->addAsset<SpriteSheet>();
    setSpriteSheet(spritesheet);
    return getSpriteSheet() != nullptr;
}

bool EditContext::newSpriteSheetFromImage(QString image_filename)
{
    if(image_filename.isNull() || image_filename.isEmpty())
    {
        return false;
    }

    QImage image;
    if(!image.load(image_filename))
    {
        //TODO: Warn user of failure
        return false;
    }


    if(image.height() * image.width() > GBA_TILESET_MAX_SIZE)
    {
        QString msg = "SpriteSheets are limited to " + QString::number(GBA_TILE_MAX) + " 8x8 tiles.\nThe max file dimension is "  + QString::number(GBA_TILESET_WIDTH) + "x" + QString::number(GBA_TILESET_HEIGHT);
        qDebug() << "[EdGBA] Image is too large!" << msg;

        QMessageBox msgBox;
        msgBox.setWindowTitle(EDGBA_TITLE);
        msgBox.setText("SpriteSheet too large!");
        msgBox.setInformativeText(msg);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
    }

    bool rename = false;
    SpriteSheet* spritesheet = getSpriteSheet();
    if(spritesheet == nullptr)
    {
        rename = true;
        spritesheet = game->addAsset<SpriteSheet>();
    }

    if(spritesheet->loadFromImage(image))
    {
        if(rename)
        {
            QFileInfo fileInfo(image_filename);
            spritesheet->setName(fileInfo.baseName());
        }

        setSpriteSheet(spritesheet);
        game->rebuildPalettes();

        return true;
    }
    return false;
}


bool EditContext::removeSpriteSheet()
{
    game->removeAsset<SpriteSheet>(spritesheet);

    SpriteSheet* spritesheet = nullptr;
    QList<SpriteSheet*> spritesheets = game->getAssets<SpriteSheet>();
    if(spritesheets.size())
    {
        spritesheet = spritesheets[0];
    }
    setSpriteSheet(spritesheet);
    return true;
}

void EditContext::setSpriteSheet(SpriteSheet* new_spritesheet)
{
    spritesheet = new_spritesheet;
}

SpriteSheet* EditContext::findSpriteSheet(const QString& name)
{
    return game->findAsset<SpriteSheet>(name);
}

SpriteSheet* EditContext::getSpriteSheet()
{
    return spritesheet;
}

void EditContext::getSpriteSheetNames(QStringList& names) const
{
    foreach(SpriteSheet* spritesheet, game->getAssets<SpriteSheet>())
    {
        if(spritesheet)
        {
            names << spritesheet->getName();
        }
    }
}

bool EditContext::newSpriteAnim()
{
    SpriteAnim* spriteanim = game->addAsset<SpriteAnim>();
    setSpriteAnim(spriteanim);
    return getSpriteAnim() != nullptr;
}

bool EditContext::removeSpriteAnim()
{
    game->removeAsset<SpriteAnim>(spriteanim);

    SpriteAnim* spriteanim = nullptr;
    QList<SpriteAnim*> spriteanims = game->getAssets<SpriteAnim>();
    if(spriteanims.size())
    {
        spriteanim = spriteanims[0];
    }
    setSpriteAnim(spriteanim);
    return true;
}

SpriteAnim* EditContext::getSpriteAnim()
{
    return spriteanim;
}

void EditContext::setSpriteAnim(SpriteAnim* spriteanim)
{
    this->spriteanim = spriteanim;
}

void EditContext::getSpriteAnimNames(QStringList& names) const
{
    foreach(SpriteAnim* spriteanim, game->getAssets<SpriteAnim>())
    {
        if(spriteanim)
        {
            names << spriteanim->getName();
        }
    }
}

SpriteAnim* EditContext::findSpriteAnim(const QString& name)
{
    return game->findAsset<SpriteAnim>(name);
}

void EditContext::setSpriteSheetSize(int size_index)
{
    if(spritesheet)
    {
        QVector<int> sprite_size_flags = SpriteSheet::getSpriteSizeFlags();
        if(size_index >= 0 && size_index < sprite_size_flags.size())
        {
            spritesheet->setSpriteSize(sprite_size_flags[size_index]);
        }
    }
}


void EditContext::undo()
{
    //TODO:!
}

void EditContext::redo()
{

}
