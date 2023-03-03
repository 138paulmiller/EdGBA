#ifndef SPRITESHEET_H
#define SPRITESHEET_H

#include "spriteanim.h"
#include "tiledimage.h"

class SpriteSheet : public TiledImage
{
public:
    void reset() override;

    static void syncPalettes(QList<SpriteSheet*> spritesheets, Palette* out_shared_palette);

    QString getPath() const override;
    QString getDefaultName() const override;
    QString getTypeName() const override;
    void getStructFields(QList<QPair<CGen::Type, QString>>& out_fields) const override;
    void writeStructData(QList<QString>& out_field_data) override;
    void readStructData(QList<QString>& in_field_data) override;

    static QStringList getSpriteSizeNames();
    static QString getSpriteSizeName(int size_flag);
    static QVector<int> getSpriteSizeFlags();

    int getSpriteSize() const;
    void setSpriteSize(int size);

    int getSpriteWidth() const;
    int getSpriteHeight() const;

    void getTileXY(int tile_index, int& tilex, int& tiley) const;
    void getTileImageXY(int tile_index, int& imagex, int& imagey) const;

    void renderFrame(QImage &image, int frame_index, bool hflip = false, bool vflip = false);
};

#endif // SPRITESHEET_H
