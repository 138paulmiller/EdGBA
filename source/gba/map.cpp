/* map.cpp
 * the actual map model which stores the tile information */

#include "map.h"
#include "gba.h"
#include "palette.h"
#include "game.h"
#include <compiler/cgen.h>

#include <QPainter>
#include <QFileInfo>

static const char* map_mode_labels[GBA_MAP_SIZE_COUNT] =
{
    "MODE 0",
    "MODE 1",
    "MODE 2",
};

static int map_sizes[GBA_MAP_SIZE_COUNT][2] =
{
    {32,32},
    {32,64},
    {64,32},
    {64,64},
    {16,16},
    {32,32},
    {64,64},
    {128,128},
};

static bool map_size_affines[GBA_MAP_SIZE_COUNT] =
{
    0,
    0,
    0,
    0,
    1,
    1,
    1,
    1,
};

static const char* map_size_labels[GBA_MAP_SIZE_COUNT] =
{
    "32x32",
    "32x64",
    "64x32",
    "64x64",
    "16x16_AFFINE",
    "32x32_AFFINE",
    "64x64_AFFINE",
    "128x128_AFFINE",
};

static int map_size_flags[GBA_MAP_SIZE_COUNT] =
{
    GBA_MAP_SIZE_32x32,
    GBA_MAP_SIZE_32x64,
    GBA_MAP_SIZE_64x32,
    GBA_MAP_SIZE_64x64,
    GBA_MAP_SIZE_16x16_AFFINE,
    GBA_MAP_SIZE_32x32_AFFINE,
    GBA_MAP_SIZE_64x64_AFFINE,
    GBA_MAP_SIZE_128x128_AFFINE,
};

/*
Display Modes
    Mode:	BG0 	BG1 	BG2 	BG3
    -----------------------------------
    0 		reg 	reg 	reg 	reg
    1 		reg 	reg 	aff 	-
    2 		- 		- 		aff 	aff
*/

static bool map_mode_bgs[GBA_TILED_MODE_COUNT][GBA_BG_COUNT] =
{
    { 1, 1, 1, 1},
    { 1, 1, 1, 0},
    { 0, 0, 1, 1},
};


static bool map_mode_affines[GBA_TILED_MODE_COUNT][GBA_BG_COUNT] =
{
    { 0, 0, 0, 0},
    { 0, 0, 1, 0},
    { 0, 0, 1, 1},
};

static inline QString getBackgroundTypePrefix(int bg_index)
{
    switch(bg_index)
    {
    case GBA_BG0: return "bg0";
    case GBA_BG1: return "bg1";
    case GBA_BG2: return "bg2";
    case GBA_BG3: return "bg3";
    }
    return "";
}

void Background::resize(int new_size_flag)
{
    const int prev_width = Map::getBackgroundSizeFlagWidth(size_flag);
    const int prev_height = Map:: getBackgroundSizeFlagHeight(size_flag);
    const QVector<int> prev_tiles = tiles;
    const QVector<bool> prev_vflips = vflips;
    const QVector<bool> prev_hflips = hflips;

    size_flag = new_size_flag;

    const int width = Map::getBackgroundSizeFlagWidth(size_flag);
    const int height = Map:: getBackgroundSizeFlagHeight(size_flag);

    tiles.fill(0, width * height);
    vflips.fill(0, width * height);
    hflips.fill(0, width * height);

    if(prev_tiles.size() && prev_width && prev_height)
    {
        for(int py = 0; py < qMin(height, prev_height); ++py)
        {
            for(int px = 0; px < qMin(width, prev_width); ++px)
            {
                tiles[py * width + px] = prev_tiles[py * prev_width + px];
                vflips[py * width + px] = prev_vflips[py * prev_width + px];
                hflips[py * width + px] = prev_hflips[py * prev_width + px];
            }
        }
    }
}

void Background::reset()
{
    map = nullptr;
    priority = 0;
    scroll_x = 0;
    scroll_y = 0;
    tileset = nullptr;
    bg_index = 0;
    resize(0);
}

void Background::forEachTileIndex(std::function<void(int)> callback)
{
    const int width = Map::getBackgroundSizeFlagWidth(size_flag);
    const int height = Map:: getBackgroundSizeFlagHeight(size_flag);
    const int affine = Map:: getBackgroundSizeFlagAffine(size_flag);

    int block = 0;
    int above = 0;
    int left = 0;
    int row = 0;
    int col = 0;

    while(row != height)
    {
        if (affine)
        {
            /* we just go tile by tile */
            if (row == height)
            {
                return;
            }

            int tile_index = row * width + col;
            callback(tile_index);

            /* update counters */
            col++;
            if (col == width)
            {
                col = 0;
                row++;
            }
            continue;
        }

        int num_blocks = (width * height) / 1024;

        if (block == num_blocks)
        {
            return;
        }

        // get the next one based off of the current indices
        int tile_row = row + 32 * above;
        int tile_col = col + 32 * left;

        // update the column
        col++;

        // if the column is out, move to the next row
        if (col == 32)
        {
            row++;
            col = 0;
        }

        // if the row is out, move to next screen block
        if (row == 32)
        {
            // if we just did the last screen block in a row of screen blocks
            int last;
            switch (num_blocks)
            {
                case 1:
                    last = 1;
                    break;
                case 2:
                    if (width == 32)
                        last = 1;
                    else
                        last = 0;
                    break;
                case 4:
                    if (block == 1 || block == 3)
                        last = 1;
                    else
                        last = 0;
                    break;
                case 16:
                    if ((block + 1) % 4 == 0)
                        last = 1;
                    else
                        last = 0;
                    break;
            }

            /* if it was the last in a row, none are left and one more is above
             * otherwise, one more is to the left */
            if (last)
            {
                left = 0;
                above++;
            }
            else
            {
                left++;
            }

            /* now update to the next block */
            block++;
            row = 0;
        }

        int tile_index = tile_row * width + tile_col;
        callback(tile_index);
    }
}


void Background::render(QImage& out_image) const
{
    if(tileset == nullptr)
    {
        return;
    }

    const int width = Map:: getBackgroundSizeFlagWidth(size_flag);
    const int height = Map:: getBackgroundSizeFlagHeight(size_flag);

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            renderTile(out_image, x, y);
        }
    }
}

void Background::renderTile(QImage& out_image, int x, int y, int tile_width, int tile_height) const
{
    const int mapx = x * GBA_TILE_SIZE;
    const int mapy = y * GBA_TILE_SIZE;
    const int width = Map:: getBackgroundSizeFlagWidth(size_flag);
    const int height = Map:: getBackgroundSizeFlagHeight(size_flag);

    if(y < 0 || y >= height || x < 0 || x >= width )
    {
        return;
    }

    if(tileset == nullptr)
    {
        return;
    }

    int index = y * width + x;
    if(index >= tiles.size())
    {
        return;
    }

    const int tile_index = tiles[index];
    const bool hflip = hflips[index];
    const bool vflip = vflips[index];
    tileset->renderTile(out_image, tile_index, hflip, vflip, tile_width, tile_height, mapx, mapy);
}

QString Background::getTilesId() const
{
    return map->getName() + "_" + getBackgroundTypePrefix(bg_index) + GBA_TILES_SUFFIX;
}

QString Background::getTilesetId() const
{
    return tileset ? "&" + tileset->getName() : "0";
}

QString Background::getTypeName() const
{
    return GBA_LAYER_TYPE;
}

void Background::getStructFields(QList<QPair<CGen::Type, QString>>& out_fields) const
{
    const QString prefix = getBackgroundTypePrefix(bg_index) + "_";
    const QString tileset_type = Tileset().getTypeName();

    out_fields.push_back(qMakePair(CGen::Type::CONST_CHAR,               prefix + QString("priority : 2")));
    out_fields.push_back(qMakePair(CGen::Type::CONST_CHAR,               prefix + QString("size_flag : 2")));
    out_fields.push_back(qMakePair(CGen::Type::CONST_SHORT,              prefix + QString("scroll_x")));
    out_fields.push_back(qMakePair(CGen::Type::CONST_SHORT,              prefix + QString("scroll_y")));
    out_fields.push_back(qMakePair(CGen::Type::CONST_PTR_UNSIGNED_CHAR, prefix + QString("tiles")));
    out_fields.push_back(qMakePair(CGen::Type::CONST_STRUCT,             tileset_type + "* " + prefix + "tileset"));
}

void Background::writeStructData(QList<QString>& out_field_data)
{
    int gba_size_flag = size_flag;
    // Map Size is shifted down if the map bg is affine
    if(map && Map::getBackgroundAffine(map->getMode(), bg_index))
    {
        gba_size_flag = gba_size_flag - GBA_MAP_SIZE_16x16_AFFINE;
    }

    out_field_data.append(QString::number(priority));
    out_field_data.append(QString::number(gba_size_flag));
    out_field_data.append(QString::number(scroll_x));
    out_field_data.append(QString::number(scroll_y));
    out_field_data.append(getTilesId());
    out_field_data.append(getTilesetId());
}

void Background::readStructData(QList<QString>& in_field_data)
{
    // Note: this must match the field order
    if(in_field_data.size() == 0)
        return;

    priority = in_field_data.front().toInt();
    in_field_data.pop_front();
    if(in_field_data.size() == 0)
        return;

    size_flag = in_field_data.front().toInt();
    in_field_data.pop_front();
    if(in_field_data.size() == 0)
        return;

    scroll_x = in_field_data.front().toInt();
    in_field_data.pop_front();
    if(in_field_data.size() == 0)
        return;

    scroll_y = in_field_data.front().toInt();
    in_field_data.pop_front();
    if(in_field_data.size() == 0)
        return;

    // setTiles(in_field_data.front()); // skip tiles. this is loaded in data
    in_field_data.pop_front();
    if(in_field_data.size() == 0)
        return;

    tileset_name = in_field_data.front();
    in_field_data.pop_front();


    // Map Size is shifted down if the map bg is affine
    if(map && Map::getBackgroundAffine(map->getMode(), bg_index))
    {
        size_flag = GBA_MAP_SIZE_16x16_AFFINE + size_flag;
    }

    const int width = Map::getBackgroundSizeFlagWidth(size_flag);
    const int height = Map::getBackgroundSizeFlagHeight(size_flag);
    tiles.clear();

    tiles.fill(0, width * height);
    vflips.fill(0, width * height);
    hflips.fill(0, width * height);
}

void Background::gatherAssets(Game* game)
{
    tileset_name.remove("&");
    tileset = game->findAsset<Tileset>(tileset_name);
}

void Background::writeDecls(QTextStream& out)
{
    CGen::writeArrayDecl(out, CGen::CONST_UNSIGNED_CHAR, getTilesId());
}

bool Background::readDecls(QTextStream& in)
{
    QString type, tiles_id;
    return CGen::readArrayDecl(in, type, tiles_id);
}

void Background::writeData(QTextStream& out)
{
    // Write tiles
    CGen::ArrayWriter array_writer(out);
    array_writer.begin(CGen::CONST_UNSIGNED_CHAR, getTilesId() );

    // Only write data if there is a tileset present. otherwise this data is unused

    forEachTileIndex([this, &array_writer](int tile_index)
    {
        if(tile_index < tiles.size())
        {
            int tile = tiles[tile_index];

            if(!Map::getBackgroundSizeFlagAffine(size_flag))
            {
                // write the tile and flip information
                if(vflips[tile_index])
                {
                    tile |= (1 << GBA_TILE_VFLIP_BIT);
                }
                if(hflips[tile_index])
                {
                    tile |= (1 << GBA_TILE_HFLIP_BIT);
                }

                array_writer.writeValue(tile & 0x00FF);
                array_writer.writeValue(tile >> 8);
            }
            else
            {
                array_writer.writeValue(tile & 0x00FF);
            }
        }
    });
    array_writer.end();
}

bool Background::readData(QTextStream& in)
{
    // Read tiles
    CGen::ArrayReader array_reader(in);

    QString id;
    if(!array_reader.begin(CGen::CONST_UNSIGNED_CHAR, id))
    {
        return false;
    }

    forEachTileIndex([this, &array_reader](int tile_index)
    {        
        if(!Map::getBackgroundSizeFlagAffine(size_flag))
        {
            bool success;
            int tile = array_reader.readValue(success);
            if(success)
            {
                // write in flip info
                tile |= array_reader.readValue(success) << 8;
            }
            if(success && tile_index < tiles.size())
            {
                if(tile & (1 << GBA_TILE_VFLIP_BIT))
                {
                    tile &= ~(1 << GBA_TILE_VFLIP_BIT);
                    vflips[tile_index] = 1;
                }
                if(tile & (1 << GBA_TILE_HFLIP_BIT))
                {
                    tile &= ~(1 << GBA_TILE_HFLIP_BIT);
                    hflips[tile_index] = 1;
                }

                tiles[tile_index] = tile;
            }
        }
        else
        {
            bool success;
            int tile = array_reader.readValue(success);
            if(success && tile_index < tiles.size())
            {
                tiles[tile_index] = tile;
            }

        }

    });

    return array_reader.end();
}

// ------------------------ Map --------------------------------//

Map::Map()
{
    reset();
}

Map::~Map()
{
    undo_stack.clear();
    redo_stack.clear();
}

QStringList Map::getMapModeNames()
{
    QStringList mode_names;
    for(int i = 0; i < GBA_TILED_MODE_COUNT; ++i)
    {
        mode_names << map_mode_labels[i];
    }
    return mode_names;
}

int Map::getMapMode(QString mode_name)
{
    for(int i = 0; i < GBA_TILED_MODE_COUNT; ++i)
    {
        if(mode_name == map_mode_labels[i])
            return i;
    }
    return -1;
}

QString Map::getMapModeName(int mode)
{
    if(mode >= 0 && mode < GBA_TILED_MODE_COUNT)
        return map_mode_labels[mode];
    return "";
}

bool Map::getBackgroundEnabled(int mode, int bg_index)
{
    return map_mode_bgs[mode][bg_index];
}

bool Map::getBackgroundAffine(int mode, int bg_index)
{
    return map_mode_affines[mode][bg_index];
}

QStringList Map::getBackgroundSizeNames(int mode, int bg_index)
{
    QStringList size_names;
    if(!getBackgroundEnabled(mode, bg_index))
        return size_names;

    for(int i = 0; i < GBA_MAP_SIZE_COUNT; ++i)
    {
        if(map_size_affines[i] == getBackgroundAffine(mode, bg_index))
        {
            size_names << map_size_labels[i];
        }
    }
    return size_names;
}

QVector<int> Map::getBackgroundSizeFlags(int mode, int bg_index)
{
    QVector<int> size_flags;
    if(!getBackgroundEnabled(mode, bg_index))
        return size_flags;

    for(int i = 0; i < GBA_MAP_SIZE_COUNT; ++i)
    {
        if(getBackgroundAffine(mode, bg_index) == map_size_affines[i])
        {
            size_flags << map_size_flags[i];
        }
    }
    return size_flags;
}

QString Map::getBackgroundSizeName(int size_flag)
{
    for(int i = 0; i < GBA_MAP_SIZE_COUNT; ++i)
    {
        if(map_size_flags[i] == size_flag)
            return map_size_labels[i];
    }
    return "";
}

int Map::getBackgroundSizeFlag(QString name)
{
    for(int i = 0; i < GBA_MAP_SIZE_COUNT; ++i)
    {
        if(map_size_labels[i] == name)
            return map_size_flags[i];
    }
    return -1;
}

int Map::getBackgroundSizeFlag(int width, int height, bool affine)
{
    if(affine)
    {
        if(width == 16 && height == 16)
            return GBA_MAP_SIZE_16x16_AFFINE;
        if(width == 32 && height == 32)
            return GBA_MAP_SIZE_32x32_AFFINE;
        if(width == 64 && height == 64)
            return GBA_MAP_SIZE_64x64_AFFINE;
        if(width == 128 && height == 128)
            return GBA_MAP_SIZE_128x128_AFFINE;
    }
    else
    {
        if(width == 32 && height == 32)
            return GBA_MAP_SIZE_32x32;
        if(width == 32 && height == 64)
            return GBA_MAP_SIZE_32x64;
        if(width == 64 && height == 32)
            return GBA_MAP_SIZE_64x32;
        if(width == 64 && height == 64)
            return GBA_MAP_SIZE_64x64;
    }
    return -1;
}

int Map::getBackgroundSizeFlagWidth(int size_flag)
{
    for(int i = 0; i < GBA_MAP_SIZE_COUNT; ++i)
    {
        if(map_size_flags[i] == size_flag)
            return map_sizes[i][0];
    }
    return 0;
}

int Map::getBackgroundSizeFlagHeight(int size_flag)
{
    for(int i = 0; i < GBA_MAP_SIZE_COUNT; ++i)
    {
        if(map_size_flags[i] == size_flag)
            return map_sizes[i][1];
    }
    return 0;
}

bool Map::getBackgroundSizeFlagAffine(int size_flag)
{
    for(int i = 0; i < GBA_MAP_SIZE_COUNT; ++i)
    {
        if(map_size_flags[i] == size_flag)
            return map_size_affines[i];
    }
    return 0;
}

int Map::getBackgroundSize(int bg_index) const
{
    if(bg_index >= 0 && bg_index < GBA_BG_COUNT)
    {
        const Background& background = backgrounds[bg_index];
        return background.size_flag;
        //Map::getBackgroundSizeFlag(background.width, background.height, background.affine);
    }
    return -1;
}

void Map::reset()
{
    for(int bg_index = 0; bg_index < GBA_BG_COUNT; ++bg_index)
    {
        Background& background = backgrounds[bg_index];
        background.reset();
        background.bg_index = bg_index;
        background.map = this;
    }

    setMode(GBA_TILED_MODE0);
    setName(GBA_DEFAULT_MAP_NAME);
}

Tileset* Map::getTileset(int bg_index)
{
    if(bg_index >= 0 && bg_index < GBA_BG_COUNT)
        return backgrounds[bg_index].tileset;
    return nullptr;
}

void Map::setTileset(int bg_index, Tileset* new_tileset)
{
    if(bg_index >= 0 && bg_index < GBA_BG_COUNT)
    {
        Background& background = backgrounds[bg_index];
        background.tileset = new_tileset;
    }
    syncBackgrounds();
}

QString Map::getTilesetName(int bg_index)
{
    Tileset* tileset = getTileset(bg_index);
    if(tileset)
    {
        return tileset->getName();
    }
    return "";
}

bool Map::getAffine(int bg_index) const
{
    if(bg_index >= 0 && bg_index < GBA_BG_COUNT)
    {
        const Background& background = backgrounds[bg_index];
        return Map::getBackgroundSizeFlagAffine(background.size_flag);
    }
    return false;
}

int Map::getPriority(int bg_index)
{
    if(bg_index >= 0 && bg_index < GBA_BG_COUNT)
    {
        Background& background = backgrounds[bg_index];
        return background.priority;
    }
    return -1;
}

void Map::setPriority(int bg_index, int priority)
{
    if(bg_index >= 0 && bg_index < GBA_BG_COUNT)
    {
        Background& background = backgrounds[bg_index];
        background.priority = priority;
    }
}

int Map::getPixelWidth() const
{
    int width = 0;
    for(int bg_index = 0; bg_index < GBA_BG_COUNT; ++bg_index)
    {
        const Background& background = backgrounds[bg_index];
        const int bg_width = Map::getBackgroundSizeFlagWidth(background.size_flag);

        width = qMax(width, bg_width * GBA_TILE_SIZE + background.scroll_x);
    }
    return width;
}

int Map::getPixelHeight() const
{
    int height = 0;
    for(int bg_index = 0; bg_index < GBA_BG_COUNT; ++bg_index)
    {
        const Background& background = backgrounds[bg_index];
        const int bg_height = Map::getBackgroundSizeFlagHeight(background.size_flag);
        height = qMax(height, bg_height * GBA_TILE_SIZE + background.scroll_y);
    }
    return height;
}

void Map::setMode(int mode)
{
    metadata.insert("mode", QString::number(mode));
    syncBackgrounds();
}

int Map::getMode() const
{
    return metadata["mode"].toInt();
}

void Map::syncBackgrounds()
{
    QVector<TiledImage*> tilesets;
    for(int bg_index= 0; bg_index < GBA_BG_COUNT; ++bg_index)
    {
        Background& background = backgrounds[bg_index];
        background.bg_index = bg_index;
        background.map = this;
        if(background.tileset)
        {
            background.tileset_name = background.tileset->getName();
            tilesets.append(background.tileset);
        }
    }
}

QString Map::getPath() const
{
    return GBA_MAPS_PATH;
}

QString Map::getTypeName() const
{
    return GBA_MAP_TYPE;
}

QString Map::getDefaultName() const
{
    return GBA_DEFAULT_MAP_NAME;
}

void Map::getStructFields(QList<QPair<CGen::Type, QString>>& out_fields) const
{
    out_fields.push_back(qMakePair(CGen::Type::CONST_CHAR,     QString("mode : 2")));
    for(int bg_index= 0; bg_index < GBA_BG_COUNT; ++bg_index)
    {
        const Background& background = backgrounds[bg_index];
        background.getStructFields(out_fields);
    }
}

void Map::writeStructData(QList<QString>& out_field_data)
{
    out_field_data.append(QString::number(getMode()));

    for(int bg_index= 0; bg_index < GBA_BG_COUNT; ++bg_index)
    {
        Background& background = backgrounds[bg_index];
        background.writeStructData(out_field_data);
    }
}

void Map::readStructData(QList<QString>& in_field_data)
{
    if(in_field_data.size() == 0)
        return;

    setMode(in_field_data.front().toInt());
    in_field_data.pop_front();
    if(in_field_data.size() == 0)
        return;

    for(int bg_index= 0; bg_index < GBA_BG_COUNT; ++bg_index)
    {
        Background& background = backgrounds[bg_index];
        background.readStructData(in_field_data);
    }
}

void Map::writeDecls(QTextStream& out)
{
    for(int bg_index= 0; bg_index < GBA_BG_COUNT; ++bg_index)
    {
        Background& background = backgrounds[bg_index];
        background.writeDecls(out);
    }
}

bool Map::readDecls(QTextStream& in)
{
    for(int bg_index= 0; bg_index < GBA_BG_COUNT; ++bg_index)
    {
        Background& background = backgrounds[bg_index];
        if(!background.readDecls(in))
            return false;
    }
    return true;
}

void Map::writeData(QTextStream& out)
{
    for(int bg_index= 0; bg_index < GBA_BG_COUNT; ++bg_index)
    {
        Background& background = backgrounds[bg_index];
        background.writeData(out);
    }
}

bool Map::readData(QTextStream& in)
{
    for(int bg_index= 0; bg_index < GBA_BG_COUNT; ++bg_index)
    {
        Background& background = backgrounds[bg_index];
        if(!background.readData(in))
            return false;
    }
    return true;
}

void Map::gatherAssets(Game* game)
{
    for(int bg_index= 0; bg_index < GBA_BG_COUNT; ++bg_index)
    {
        Background& background = backgrounds[bg_index];
        background.gatherAssets(game);
    }
    syncBackgrounds();
}

void Map::resizeBackground(int bg_index, int size_flag)
{
    if(bg_index >= 0 && bg_index < GBA_BG_COUNT)
    {
        Background& background = backgrounds[bg_index];
        background.bg_index = bg_index;
        background.resize(size_flag);
    }
}

Background* Map::getBackground(int bg_index)
{
    if(bg_index >= 0 && bg_index < GBA_BG_COUNT)
    {
        return &backgrounds[bg_index];
    }
    return nullptr;
}

void Map::setTile(int bg_index, int index, int tile_index, bool hflip, bool vflip)
{
    if(bg_index >= 0 && bg_index < GBA_BG_COUNT)
    {
        Background& background = backgrounds[bg_index];
        if(index >= 0 && index < background.tiles.size())
        {
            background.tiles[index] = tile_index;
            background.hflips[index] = hflip;
            background.vflips[index] = vflip;
        }
    }
}

void Map::render(QImage& out_image) const
{
    int image_width = getPixelWidth();
    int image_height = getPixelHeight();
    if(out_image.width() != image_width || out_image.height() != image_height)
    {
        out_image = QImage(image_width, image_height, QImage::Format_Indexed8);
        out_image.fill(Qt::transparent);
    }

    Palette* tileset_palette = game->getTilesetPalette();
    out_image.setColorTable(*tileset_palette);

    for(int priority = GBA_PRIORITY_COUNT-1; priority >= 0; --priority)
    {
        for(int bg_index = 0; bg_index < GBA_BG_COUNT; ++bg_index)
        {
            const Background& background = backgrounds[bg_index];
            if(background.priority == priority)
            {
                background.render(out_image);
            }
        }
    }
}

void Map::renderTile(QImage& out_image, int x, int y, int tile_width, int tile_height) const
{
    int image_width = getPixelWidth();
    int image_height = getPixelHeight();
    if(out_image.width() < image_width || out_image.height() < image_height)
    {
        out_image = QImage(image_width, image_height, QImage::Format_Indexed8);
        out_image.fill(Qt::transparent);
    }

    Palette* tileset_palette = game->getTilesetPalette();
    out_image.setColorTable(*tileset_palette);

    for(int priority = GBA_PRIORITY_COUNT-1; priority >= 0; --priority)
    {
        for(int bg_index = 0; bg_index < GBA_BG_COUNT; ++bg_index)
        {
            const Background& background = backgrounds[bg_index];
            if(background.priority == priority)
            {
                background.renderTile(out_image, x, y, tile_width, tile_height);
            }
        }
    }
}

void Map::undo()
{
    if (undo_stack.empty())
    {
        return;
    }

    QVector<Background> restore = undo_stack.back();
    undo_stack.pop_back();

    QVector<Background> current;
    for(int bg_index = 0; bg_index < GBA_BG_COUNT; ++bg_index)
        current.append(backgrounds[bg_index]);

    for(int bg_index = 0; bg_index < GBA_BG_COUNT; ++bg_index)
        backgrounds[bg_index] = restore[bg_index];

    redo_stack.push_back(current);
}

void Map::redo()
{
    if (redo_stack.empty())
    {
        return;
    }

    QVector<Background> restore = redo_stack.back();
    redo_stack.pop_back();

    QVector<Background> current;
    for(int bg_index = 0; bg_index < GBA_BG_COUNT; ++bg_index)
        current.append(backgrounds[bg_index]);

    for(int bg_index = 0; bg_index < GBA_BG_COUNT; ++bg_index)
        backgrounds[bg_index] = restore[bg_index];

    undo_stack.push_back(current);
}
