#include "tileset.h"
#include "gba.h"

void Tileset::reset()
{
    setName(GBA_DEFAULT_TILESET_NAME);
    setWidth(GBA_TILESET_WIDTH);
    setHeight(GBA_TILESET_HEIGHT);
    setTileWidth(GBA_TILE_SIZE);
    setTileHeight(GBA_TILE_SIZE);
    palette.clear();
    pixels.fill(0, getWidth()*getHeight());
}

void Tileset::syncPalettes(QList<Tileset*> tilesets, Palette* out_shared_palette)
{
    QList<TiledImage*> images;
    foreach(Tileset* tileset, tilesets)
    {
        images.append(tileset);
    }
    TiledImage::syncPalettes(images, out_shared_palette);
}

QString Tileset::getPath() const
{
    return GBA_TILESETS_PATH;
}

QString Tileset::getDefaultName() const
{
    return GBA_DEFAULT_TILESET_NAME;
}

QString Tileset::getTypeName() const
{
    return GBA_TILESET_TYPE;
}

void Tileset::getTileXY(int tile_index, int& tilex, int& tiley) const
{
    getTileImageXY(tile_index, tilex, tiley);
    tilex /= GBA_TILE_SIZE;
    tiley /= GBA_TILE_SIZE;
}

void Tileset::getTileImageXY(int tile_index, int& imagex, int& imagey) const
{
    const int width = getWidth();
    if(width == 0) return;
    imagex = (tile_index * GBA_TILE_SIZE) % width;
    imagey = ((tile_index * GBA_TILE_SIZE) / width) * GBA_TILE_SIZE;
}

void Tileset::renderTile(QImage &out_image, int tile_index, bool hflip, bool vflip, int tile_width, int tile_height, int offset_x, int offset_y)
{
    int tileset_width = getWidth();
    if(tileset_width == 0) return;

    int tilex = (tile_index * GBA_TILE_SIZE) % tileset_width;
    int tiley = ((tile_index * GBA_TILE_SIZE) / tileset_width) * GBA_TILE_SIZE;

    for (int i = 0; i < tile_width; i++)
    {
        for (int j = 0; j < tile_height; j++)
        {
            const int u = hflip ? tile_width - 1 - i : i;
            const int v = vflip ? tile_height - 1 - j : j;

            int color_index = getColorIndex(tilex + i, tiley + j);
            QRgb color = getColor(color_index);
            if(qAlpha(color) != 0)
            {
                out_image.setPixel(offset_x + u, offset_y + v, color_index);
            }
        }
    }
}
