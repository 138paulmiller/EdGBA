#ifndef TILESET_H
#define TILESET_H

#include "tiledimage.h"

class Tileset : public TiledImage
{
public:
    void reset() override;

    static void syncPalettes(QList<Tileset*> tilesets, Palette* out_shared_palette);

    QString getPath() const override;
    QString getDefaultName() const override;
    QString getTypeName() const override;

    void getTileXY(int tile_index, int& tilex, int& tiley) const;
    void getTileImageXY(int tile_index, int& tilex, int& tiley) const;

    void renderTile(QImage &out_image, int tile_index, bool hflip, bool vflip, int tile_width, int tile_height, int offset_x = 0, int offset_y = 0);
};

#endif // TILESET_H
