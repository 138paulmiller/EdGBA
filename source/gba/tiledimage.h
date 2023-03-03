#ifndef TILEDIMAGE_H
#define TILEDIMAGE_H

#include <QRgb>
#include <QVector>
#include <QPixmap>

#include <QString>
#include <QTextStream>

#include "palette.h"

class TiledImage : public Asset
{
protected:

    QVector<unsigned char> pixels;
    QVector<QRgb> palette;

public:
    virtual ~TiledImage() = default;

    virtual void reset() override;
    virtual QString getPath() const override;
    virtual QString getDefaultName() const override;
    virtual QString getTypeName() const override;

    virtual void getStructFields(QList<QPair<CGen::Type, QString>>& out_fields) const override;
    virtual void writeStructData(QList<QString>& out_field_data) override;
    virtual void readStructData(QList<QString>& in_field_data) override;
    virtual void writeDecls(QTextStream& out) override;
    virtual bool readDecls(QTextStream& in) override;
    virtual void writeData(QTextStream& out) override;
    virtual bool readData(QTextStream& in) override;
    virtual void gatherAssets(Game* game) override;

    void render(QImage& out_image);
    void renderRegion(const QRect& rect, QImage& image);
    bool loadFromImage(const QImage& image);

    int addOrFindColor(QRgb color);
    int getColorIndex(int x, int y);
    QRgb getColor(int color_index);

    void setColor(int x, int y, QRgb color);
    void setPaletteColor(int color_index, QRgb color);
    void setPaletteColorkey(QRgb colorkey);

    static void syncPalettes(QList<TiledImage*> images, Palette* out_shared_palette);
    void setPalette(const QVector<QRgb>& palette);
    const QVector<QRgb>& getPalette() const;

    int getWidth() const;
    int getHeight() const;
    int getTileWidth() const;
    int getTileHeight() const;
    QString getSharedPalette() const;
    bool usesSharedPalette() const;

    void setWidth(int width);
    void setHeight(int height);
    void setTileWidth(int tile_width);
    void setTileHeight(int tile_height);
    void setSharedPalette(QString shared_palette);

    QString getPaletteId() const;
    QString getPixelsId() const;
};

#endif // TILEDIMAGE_H
