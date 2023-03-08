#include "editorinterface.h"
#include "mainwindow.h"

#include <QMessageBox>

EditContext::EditContext()
{
    reset();
}

EditContext::~EditContext()
{}

void EditContext::reset()
{
    selected_bg_index = GBA_BG0;
    for(int i = 0; i < GBA_BG_COUNT; ++i)
    {
        selections[i].tiles = { 0 };
        selections[i].size = 1;
        selections[i].hflip = 0;
        selections[i].vflip = 0;
    }

    if(game == nullptr)
    {
        tileset = nullptr;
        spriteanim = nullptr;
        spritesheet = nullptr;
        map = nullptr;
        return;
    }


    QList<Tileset*> tilesets = game->getAssets<Tileset>();
    if(tilesets.size())
    {
        setSelectedTileset(tilesets[0]);
    }
    else
    {
        setSelectedTileset(nullptr);
    }

    QList<Map*> maps = game->getAssets<Map>();
    if(maps.size())
    {
        setMap(maps[0]);
    }
    else
    {
        setMap(nullptr);
    }

    QList<SpriteSheet*> spritesheets = game->getAssets<SpriteSheet>();
    if(spritesheets.size())
    {
        setSpriteSheet(spritesheets[0]);
    }
    else
    {
        setSpriteSheet(nullptr);
    }

    QList<SpriteAnim*> spriteanims  = game->getAssets<SpriteAnim>();
    if(spriteanims.size())
    {
        setSpriteAnim(spriteanims[0]);
    }
    else
    {
        setSpriteAnim(nullptr);
    }
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

void EditContext::selectBackground(int bg_index)
{
    selected_bg_index = bg_index;
}

int EditContext::getSelectedBackground() const
{
    return selected_bg_index;
}

void EditContext::resizeSelectedBackground(int size_flag)
{
    if(map)
    {
        map->resizeBackground(selected_bg_index, size_flag);
    }
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

bool EditContext::newTilesetFromImage(QString image_filename)
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
    Tileset* tileset = getSelectedTileset();
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

        setSelectedTileset(tileset);
        game->rebuildPalettes();
        return true;
    }
    return false;
}

bool EditContext::newTileset(QString name)
{
    if(game == nullptr || map == nullptr)
    {
        return false;
    }

    Tileset* tileset = game->addAsset<Tileset>();
    if(tileset)
        tileset->setName(name);
    setSelectedTileset(tileset);
    return getTileset(selected_bg_index) != nullptr;
}

bool EditContext::removeSelectedTileset()
{
    if(game == nullptr)
    {
        return false;
    }

    Tileset* tileset = getSelectedTileset();
    game->removeAsset<Tileset>(tileset);

    Tileset* new_tileset = nullptr;
    QList<Tileset*> tilesets = game->getAssets<Tileset>();
    if(tilesets.size())
        new_tileset = tilesets.first();

    foreach(Map* map, game->getAssets<Map>())
    {
        for(int bg_index = 0; bg_index < GBA_BG_COUNT; ++bg_index)
        {
            if(map->getTileset(bg_index) == tileset)
            {
                map->setTileset(bg_index, new_tileset);
            }
        }
    }

    tileset = nullptr;
    if(tilesets.size())
    {
        tileset = tilesets[0];
    }
    setSelectedTileset(tileset);
    return true;
}

void EditContext::selectTiles(int tilex, int tiley)
{
    if(game == nullptr || map == nullptr)
    {
        return;
    }

    Tileset* tileset = getSelectedTileset();
    if(tileset == nullptr)
    {
        return;
    }

    int tiles_width = tileset->getWidth()/ GBA_TILE_SIZE;
    int tiles_height = tileset->getHeight()/ GBA_TILE_SIZE;
    if(tilex >= tiles_width || tiley >= tiles_height)
    {
        return;
    }

    TilesetSelection& selection = getSelection();
    selection.tiles.clear();
    for(int dy = 0; dy < selection.size; ++dy)
    {
        for(int dx = 0; dx < selection.size; ++dx)
        {
            int tile = (tiley + dy) * tiles_width + (tilex + dx);
            selection.tiles.push_back(tile);
        }
    }
}

bool EditContext::getCornerSelectedTileXY(int& tilex, int& tiley) const
{
    if(game == nullptr || map == nullptr)
    {
        return false;
    }


    Tileset* tileset = getSelectedTileset();
    if(tileset == nullptr)
    {
        return false;
    }

    const TilesetSelection& selection = getSelection();
    if(selection.tiles.size())
    {
        tileset->getTileImageXY(selection.tiles[0], tilex, tiley);
        return true;
    }
    return false;
}

void EditContext::setMap(Map* new_map)
{
    map = new_map;
    if(map)
    {
        const QString tileset_name = map->getTilesetName(selected_bg_index);
        Tileset* tileset = findTileset(tileset_name);
        setSelectedTileset(tileset);
    }
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

void EditContext::setSelectedTileset(Tileset* new_tileset)
{
    if(map)
        map->setTileset(selected_bg_index, new_tileset);
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

QString EditContext::getTilesetName(int bg_index) const
{
    Tileset* tileset = getTileset(bg_index);
    if(tileset == nullptr)
    {
        return EMPTY_TILESET_NAME;
    }
    return tileset->getName();
}

QString EditContext::getBackgroundSizeName(int bg_index) const
{
    if(map == nullptr)
    {
        return "";
    }

    return Map::getBackgroundSizeName(map->getBackgroundSize(bg_index));
}

Tileset* EditContext::getSelectedTileset() const
{
    return getTileset(selected_bg_index);
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

void EditContext::getBackgroundSizeNames(QStringList& names) const
{
    foreach(const Tileset* tileset, game->getAssets<Tileset>())
    {
        if(tileset)
        {
            names << tileset->getName();
        }
    }
}

int EditContext::getTileSelectionSize() const
{
    const TilesetSelection& selection = getSelection();
    return selection.size * GBA_TILE_SIZE;
}

void EditContext::setTileSelectionSize(int new_tile_selection_size)
{
    setSelectionSize(new_tile_selection_size / GBA_TILE_SIZE);
}

int EditContext::getSelectionSize() const
{
    const TilesetSelection& selection = getSelection();
    return selection.size;
}

bool EditContext::getSelectionHFlip() const
{
    const TilesetSelection& selection = getSelection();
    return selection.hflip;
}

void EditContext::setSelectionHFlip(bool is_flipped)
{
    TilesetSelection& selection = getSelection();
    selection.hflip = is_flipped;
    refreshSelection();
}

bool EditContext::getSelectionVFlip() const
{
    const TilesetSelection& selection = getSelection();
    return selection.vflip;
}

void EditContext::setSelectionVFlip(bool is_flipped)
{
    TilesetSelection& selection = getSelection();
    selection.vflip = is_flipped;
    refreshSelection();
}

void EditContext::setSelectionSize(int new_selection_size)
{
    TilesetSelection& selection = getSelection();
    selection.size = new_selection_size;
    refreshSelection();
}

bool EditContext::renderSelectedTiles(QImage& image, int target_size) const
{
    Tileset* tileset = getSelectedTileset();
    if(tileset == nullptr)
    {
        return false;
    }

    int imagex, imagey;
    if(!getCornerSelectedTileXY(imagex, imagey))
    {
        return false;
    }

    QRect rect;
    rect.setX(imagex);
    rect.setY(imagey);
    rect.setWidth(getTileSelectionSize());
    rect.setHeight(getTileSelectionSize());

    tileset->renderRegion(rect, image);

    float scale = target_size;
    image = image.scaled(scale, scale, Qt::KeepAspectRatio);
    return true;
}

void EditContext::refreshSelection()
{
    Tileset* tileset = getSelectedTileset();
    if(tileset == nullptr)
    {
        return;
    }

    TilesetSelection& selection = getSelection();

    int tilex, tiley;
    tileset->getTileXY(selection.tiles[0], tilex, tiley);
    selectTiles(tilex, tiley);
}

EditContext::TilesetSelection& EditContext::getSelection()
{
    return selections[(int)selected_bg_index];
}

const EditContext::TilesetSelection& EditContext::getSelection() const
{
    return selections[(int)selected_bg_index];
}

void EditContext::handleTilesetTileClick(int tilex, int tiley)
{
    TilesetSelection& selection = getSelection();
    tilex *= selection.size;
    tiley *= selection.size;
    selectTiles(tilex, tiley);
}

void EditContext::handleMapTileClick(int tilex, int tiley)
{
    if(map == nullptr)
        return;

    Background* background = map->getBackground(selected_bg_index);
    if(background == nullptr)
    {
        return;
    }

    TilesetSelection& selection = getSelection();
    tilex *= selection.size;
    tiley *= selection.size;

    const bool hflip = selection.hflip;
    const bool vflip = selection.vflip;

    for(int dy = 0; dy < selection.size; ++dy)
    {
        for(int dx = 0; dx < selection.size; ++dx)
        {
            // If selection is flipped, grab tileset tiles in reverse order
            const int du = hflip ? selection.size - 1 - dx : dx;
            const int dv = vflip ? selection.size - 1 - dy : dy;
            int tileset_tile_index = dv * selection.size + du;

            int bg_width = Map::getBackgroundSizeFlagWidth(background->size_flag);
            int tile_index = (tiley + dy) * bg_width + (tilex + dx);

            if(tileset_tile_index < selection.tiles.size())
            {
                int tile = selection.tiles[tileset_tile_index];
                map->setTile(selected_bg_index, tile_index, tile, hflip, vflip);
            }
        }
    }
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
