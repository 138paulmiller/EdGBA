#include "spritesheet.h"
#include "gba.h"

static int sprite_sizes[GBA_SPRITE_SIZE_COUNT][2] = {
    {8, 8 },
    {16,16},
    {32,32},
    {64,64},
    {16,8 },
    {32,8 },
    {32,16},
    {64,32},
    {8, 16},
    {8, 32},
    {16,32},
    {32,64},
};

static const char* sprite_size_labels[GBA_SPRITE_SIZE_COUNT] =
{
    "8x8",
    "16x16",
    "32x32",
    "64x64",
    "16x8",
    "32x8",
    "32x16",
    "64x32",
    "8x16",
    "8x32",
    "16x32",
    "32x64",
};

static int sprite_size_flags[GBA_SPRITE_SIZE_COUNT] =
{
    GBA_SPRITE_SIZE_8x8  ,
    GBA_SPRITE_SIZE_16x16,
    GBA_SPRITE_SIZE_32x32,
    GBA_SPRITE_SIZE_64x64,
    GBA_SPRITE_SIZE_16x8 ,
    GBA_SPRITE_SIZE_32x8 ,
    GBA_SPRITE_SIZE_32x16,
    GBA_SPRITE_SIZE_64x32,
    GBA_SPRITE_SIZE_8x16 ,
    GBA_SPRITE_SIZE_8x32 ,
    GBA_SPRITE_SIZE_16x32,
    GBA_SPRITE_SIZE_32x64,
};

void SpriteSheet::reset()
{
    setSpriteSize(GBA_SPRITE_SIZE_8x8);
    setName(GBA_DEFAULT_SPRITESHEET_NAME);
    setWidth(GBA_SPRITESHEET_WIDTH);
    setHeight(GBA_SPRITESHEET_HEIGHT);
    //setTileWidth(GBA_TILE_SIZE);
    //setTileHeight(GBA_TILE_SIZE);

    palette.clear();
    pixels.fill(0, getWidth()*getHeight());
}

void SpriteSheet::syncPalettes(QList<SpriteSheet*> spritesheets, Palette* out_shared_palette)
{
    QList<TiledImage*> images;
    foreach(SpriteSheet* spritesheet, spritesheets)
    {
        images.append(spritesheet);
    }
    TiledImage::syncPalettes(images, out_shared_palette);
}

QString SpriteSheet::getPath() const
{
    return GBA_SPRITESHEETS_PATH;
}

QString SpriteSheet::getDefaultName() const
{
    return GBA_DEFAULT_SPRITESHEET_NAME;
}

QString SpriteSheet::getTypeName() const
{
    return GBA_SPRITESHEET_TYPE;
}

void SpriteSheet::getStructFields(QList<QPair<CGen::Type, QString>>& out_fields) const
{
    TiledImage::getStructFields(out_fields);
    out_fields.push_back(qMakePair(CGen::Type::UNSIGNED_SHORT, QString("sprite_size")));
}

void SpriteSheet::writeStructData(QList<QString>& out_field_data)
{
    // Note: this must match the field order
    TiledImage::writeStructData(out_field_data);
    out_field_data.append(QString::number(getSpriteSize()));
}

void SpriteSheet::readStructData(QList<QString>& in_field_data)
{
    TiledImage::readStructData(in_field_data);
    if(in_field_data.size() == 0)
        return;

    setSpriteSize(in_field_data.front().toInt());
    in_field_data.pop_front();
    if(in_field_data.size() == 0)
        return;
}

void SpriteSheet::getTileXY(int tile_index, int& tilex, int& tiley) const
{
    getTileImageXY(tile_index, tilex, tiley);
    tilex /= GBA_TILE_SIZE;
    tiley /= GBA_TILE_SIZE;
}

void SpriteSheet::getTileImageXY(int tile_index, int& imagex, int& imagey) const
{
    const int width = getWidth();
    if(width == 0) return;
    imagex = (tile_index * GBA_TILE_SIZE) % width;
    imagey = ((tile_index * GBA_TILE_SIZE) / width) * GBA_TILE_SIZE;
}

QStringList SpriteSheet::getSpriteSizeNames()
{
    QStringList list;
    for(int i = 0; i < GBA_SPRITE_SIZE_COUNT; ++i)
    {
        list << sprite_size_labels[i];
    }
    return list;
}

QString SpriteSheet::getSpriteSizeName(int size_flag)
{
    for(int i = 0; i < GBA_SPRITE_SIZE_COUNT; ++i)
    {
        if(sprite_size_flags[i] == size_flag)
            return sprite_size_labels[i];
    }
    return "";
}

QVector<int> SpriteSheet::getSpriteSizeFlags()
{
    QVector<int> vector;
    for(int i = 0; i < GBA_SPRITE_SIZE_COUNT; ++i)
    {
        vector << sprite_size_flags[i];
    }
    return vector;
}

int SpriteSheet::getSpriteSize() const
{
    return metadata["sprite_size"].toInt();
}

void SpriteSheet::setSpriteSize(int size_flag)
{
    metadata["sprite_size"] = QString::number(size_flag);
    setTileWidth(getSpriteWidth());
    setTileHeight(getSpriteHeight());
}

int SpriteSheet::getSpriteWidth() const
{
    int size_flags = getSpriteSize();
    for(int i = 0; i < GBA_SPRITE_SIZE_COUNT; ++i )
    {
        if(sprite_size_flags[i] == size_flags)
        {
            return sprite_sizes[i][0];
        }
    }
    return 0;
}

int SpriteSheet::getSpriteHeight() const
{
    int size_flags = getSpriteSize();
    for(int i = 0; i < GBA_SPRITE_SIZE_COUNT; ++i )
    {
        if(sprite_size_flags[i] == size_flags)
        {
            return sprite_sizes[i][1];
        }
    }
    return 0;
}

void SpriteSheet::renderFrame(QImage &out_image, int frame_index, bool hflip, bool vflip)
{
    const int sprite_width = getSpriteWidth();
    const int sprite_height = getSpriteHeight();
    const int spritesheet_width = getWidth();

    if(spritesheet_width == 0 || sprite_width == 0 || sprite_height == 0)
    {
        return;
    }

    if(out_image.width() < sprite_width || out_image.height() < sprite_height)
    {
        out_image = QImage(sprite_width, sprite_height, QImage::Format_Indexed8);
        out_image.fill(Qt::transparent);
    }

    out_image.setColorTable(getPalette());

    const int tilex = (frame_index * sprite_width) % spritesheet_width;
    const int tiley = ((frame_index * sprite_width) / spritesheet_width) * sprite_height;

    for(int i = 0; i < sprite_width; ++i)
    {
        for(int j = 0; j < sprite_height; ++j)
        {
            const int u = hflip ? sprite_width - 1 - i : i;
            const int v = vflip ? sprite_height - 1 - j : j;

            const int color_index = getColorIndex(tilex + i, tiley + j);
            out_image.setPixel(u, v, color_index);
        }
    }
}
