#include "tiledimage.h"
#include "gba.h"
#include "game.h"
#include "palette.h"
#include <msglog.h>
#include <compiler/cgen.h>
#include <functional>

// Iterates the image by each 8x8 tiles from left to right, top to bottom
static void foreachTileAt(int width, int height, std::function<void(int, int)> callback)
{
    int x = 0, y = 0;
    int tile_x = 0, tile_y = 0;

    while(y != height)// we reached the end of the image
    {
        callback(x, y);

        x++;
        tile_x++;

        /* if we hit the end of a tile row, then go to the next row */
        if (tile_x >= GBA_TILE_SIZE)
        {
            y++;
            tile_y++;
            x -= GBA_TILE_SIZE;
            tile_x = 0;

            /* if we hit the end of the tile altogether */
            if (tile_y >= GBA_TILE_SIZE)
            {
                y -= GBA_TILE_SIZE;
                x += GBA_TILE_SIZE;
                tile_y = 0;
            }

            /* if we are now at the end of the actual row, go to next one */
            if (x >= width)
            {
                tile_x = 0;
                tile_y = 0;
                x = 0;
                y += GBA_TILE_SIZE;
            }
        }
    }
}

static void foreachTile(int tile_width, int tile_height, int width, int height, std::function<void(int, int)> callback)
{
    for(int y = 0; y < height; y += tile_height)
    {
        for(int x = 0; x < width; x += tile_width)
        {
            for(int th = 0; th < tile_height; th += GBA_TILE_SIZE)
            {
                for(int tw = 0; tw < tile_width; tw += GBA_TILE_SIZE)
                {
                    auto iterator = [&callback, x, y, tw, th](int tx, int ty)
                    {
                        callback(x + tx + tw, y + ty + th);
                    };
                    foreachTileAt(GBA_TILE_SIZE, GBA_TILE_SIZE, iterator);
                }
            }
        }
    }
}

QVector<unsigned char> translateFromGBAImage(const QVector<unsigned char>& pixels, int tile_width, int tile_height, int width, int height)
{
    QVector<unsigned char> out_pixels;
    out_pixels.fill(0, pixels.size());

    int in_index = 0;
    auto iterator = [&pixels, &out_pixels, &in_index, width](int x, int y)
    {
        const int out_index = y * width + x;
        if(in_index >= pixels.size() || out_index >= out_pixels.size())
        {
           return;
        }
        int color_index = pixels[in_index++];
        if(color_index >= 0 && color_index < GBA_PALETTE_COUNT)
        {
            if(out_index < out_pixels.size())
                out_pixels[out_index] = color_index;
        }
    };
    foreachTile(tile_width, tile_height, width, height, iterator);
    return out_pixels;
}

QVector<unsigned char> translateToGBAImage(const QVector<unsigned char>& pixels, int tile_width, int tile_height, int width, int height)
{
    QVector<unsigned char> out_pixels;
    out_pixels.fill(0, pixels.size());

    int out_index = 0;
    auto iterator = [&pixels, &out_pixels, &out_index, width](int x, int y)
    {
        const int in_index = y * width + x;
        if(in_index >= pixels.size() || out_index >= out_pixels.size())
        {
           return;
        }
        int color_index = pixels[in_index];
        if(color_index >= 0 && color_index < GBA_PALETTE_COUNT)
        {
            out_pixels[out_index++] = color_index;
        }
    };

    foreachTile(tile_width, tile_height, width, height, iterator);
    return out_pixels;
}

void TiledImage::reset()
{
    setWidth(0);
    setHeight(0);
    setTileWidth(GBA_TILE_SIZE);
    setTileHeight(GBA_TILE_SIZE);
    setSharedPalette("");
    palette.clear();
    pixels.clear();
}

QString TiledImage::getPath() const
{
    return GBA_IMAGES_PATH;
}

QString TiledImage::getDefaultName() const
{
    return GBA_DEFAULT_IMAGE_NAME;
}

QString TiledImage::getTypeName() const
{
    return GBA_IMAGE_TYPE;
}

void TiledImage::getStructFields(QList<QPair<CGen::Type, QString>>& out_fields) const
{
    out_fields.push_back(qMakePair(CGen::Type::UNSIGNED_SHORT, QString("width")));
    out_fields.push_back(qMakePair(CGen::Type::UNSIGNED_SHORT, QString("height")));
    out_fields.push_back(qMakePair(CGen::Type::CONST_PTR_UNSIGNED_CHAR, QString("pixels")));
    if(!usesSharedPalette())
    {
        out_fields.push_back(qMakePair(CGen::Type::CONST_PTR_UNSIGNED_SHORT, QString("palette")));
    }
}

void TiledImage::writeStructData(QList<QString>& out_field_data)
{
    // Note: this must match the field order
    out_field_data.append(QString::number(getWidth()));
    out_field_data.append(QString::number(getHeight()));
    out_field_data.append(getPixelsId());
    if(!usesSharedPalette())
    {
        out_field_data.append(getPaletteId());
    }
}

void TiledImage::readStructData(QList<QString>& in_field_data)
{
    // Note: this must match the field order
    if(in_field_data.size() == 0)
        return;

    setWidth(in_field_data.front().toInt());
    in_field_data.pop_front();
    if(in_field_data.size() == 0)
        return;

    setHeight(in_field_data.front().toInt());
    in_field_data.pop_front();
    if(in_field_data.size() == 0)
        return;

    // Image and color data is skipped. These arrays are loaded manually in the source file

    //setTilesId(field_data.at(2));
    in_field_data.pop_front();
    if(in_field_data.size() == 0)
        return;

    if(!usesSharedPalette())
    {
        //setPaletteId(field_data.at(3));
        in_field_data.pop_front();
        if(in_field_data.size() == 0)
            return;
    }
}

void TiledImage::writeDecls(QTextStream& out)
{
    CGen::writeArrayDecl(out, CGen::CONST_UNSIGNED_CHAR, getPixelsId());
    if(!usesSharedPalette())
    {
        CGen::writeArrayDecl(out, CGen::CONST_UNSIGNED_SHORT, getPaletteId());
    }
}

bool TiledImage::readDecls(QTextStream& in)
{
    QString type;
    QString pixels_id;
    if(!CGen::readArrayDecl(in, type, pixels_id))
        return false;

    if(!usesSharedPalette())
    {
        QString palette_id;
        if(!CGen::readArrayDecl(in, type, palette_id))
            return false;
    }

    return true;
}

void TiledImage::writeData(QTextStream& out)
{
    CGen::ArrayWriter array_writer(out);

    int width = getWidth();
    int height = getHeight();
    int tile_width = getTileWidth();
    int tile_height = getTileHeight();
    QVector<unsigned char> pixel_data = translateToGBAImage(pixels, tile_width, tile_height, width, height);
    array_writer.writeAllValues(CGen::CONST_UNSIGNED_CHAR, getPixelsId(), pixel_data);

    if(!usesSharedPalette())
    {
       QVector<int> palette_data = Palette::translateToGBAPalette(palette);
       array_writer.writeAllValues(CGen::CONST_UNSIGNED_SHORT, getPaletteId(), palette_data);
    }
}

bool TiledImage::readData(QTextStream& in)
{
    CGen::ArrayReader array_reader(in);
    QString id;

    QVector<unsigned char> pixel_data;
    if(!array_reader.readAllValues(CGen::CONST_UNSIGNED_CHAR, id, pixel_data))
    {
        return false;
    }

    const int width = getWidth();
    const int height = getHeight();
    const int tile_width = getTileWidth();
    const int tile_height = getTileHeight();
    pixels = translateFromGBAImage(pixel_data, tile_width, tile_height, width, height);
    if(pixels.size() != width * height)
    {
        msgError("Asset") << id << " pixels size does not match image dimensions\n";
    }

    if(!usesSharedPalette())
    {
        QVector<int> palette_data;
        if(!array_reader.readAllValues(CGen::CONST_UNSIGNED_SHORT, id, palette_data))
        {
            return false;
        }
        palette = Palette::translateFromGBAPalette(palette_data);
    }
    return true;
}

void TiledImage::gatherAssets(Game* game)
{
    QList<Palette*> palettes = game->getAssets<Palette>();
    foreach(Palette* palette, palettes)
    {
        if(getSharedPalette() == palette->getName())
        {
            setPalette(*palette);
        }
    }
}

bool TiledImage::loadFromImage(const QImage& image)
{
    setWidth(0);
    setHeight(0);
    palette.clear();
    pixels.clear();

    int width = image.width();
    int height = image.height();
    if(width * height == 0)
    {
        return false;
    }
    setWidth(width);
    setHeight(height);

    pixels.fill(0, image.width() * image.height());
    for(int x = 0; x < image.width(); ++x)
    {
        for(int y = 0; y < image.height(); ++y)
        {
            QRgb color = image.pixel(x, y);
            int color_index = addOrFindColor(color);
            int index = y * image.width() + x;
            if(index < pixels.size())
            {
                pixels[index] = color_index;
            }
        }
    }
    return true;
}

void TiledImage::render(QImage& out_image)
{
    int width = getWidth();
    int height = getHeight();

    if(out_image.width() != width || out_image.height() != height)
    {
        out_image = QImage(width, height, QImage::Format_Indexed8);
        out_image.fill(Qt::transparent);
    }
    out_image.setColorTable(palette);

    for(int x = 0; x < width; ++x)
    {
        for(int y = 0; y < height; ++y)
        {
            out_image.setPixel(x,y, getColorIndex(x, y));
        }
    }
}

void TiledImage::renderRegion(const QRect& rect, QImage& out_image)
{
    int x =  rect.x();
    int y =  rect.y();
    int w =  rect.width();
    int h =  rect.height();
    out_image = QImage(w, h, QImage::Format_Indexed8);
    out_image.setColorTable(palette);

    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < h; j++)
        {
            out_image.setPixel(x,y, getColorIndex(x+i, y+j));
        }
    }
}

int TiledImage::addOrFindColor(QRgb color)
{
    for(int i = 0; i < palette.size(); ++i)
    {
        if(palette[i] == color)
        {
            return i;
        }
    }

    if(palette.size() + 1 < GBA_PALETTE_COUNT)
    {
        palette.append(color);
    }
    return palette.size()-1;
}

int TiledImage::getColorIndex(int x, int y)
{
    int width = getWidth();
    int height = getHeight();
    if(x < 0 || x >= width || y < 0 || y >= height)
    {
        // TODO: Warn user
        return 0;
    }

    return pixels[y * width + x];
}

QRgb TiledImage::getColor(int color_index)
{
    if(color_index <= 0 || color_index >= palette.size())
        return 0;

    return palette[color_index];
}

void TiledImage::setColor(int x, int y, QRgb color)
{
    int width = getWidth();
    int height = getHeight();
    if(x < 0 || x >= width || y < 0 || y >= height)
    {
        return;
    }

    int color_index = addOrFindColor(color);
    pixels[y * width +  x] = color_index;
}

void TiledImage::setPaletteColor(int color_index, QRgb color)
{
    if(color_index < palette.size())
    {
        palette[color_index] = color;
    }
}

void TiledImage::syncPalettes(QList<TiledImage*> images, Palette* out_shared_palette)
{
    images.removeAll(nullptr);

    QVector<QMap<int, int>> color_index_maps;
    QVector<QRgb>& shared_colors = *out_shared_palette;
    shared_colors.clear();

    for(int i = 0; i < images.size(); ++i)
    {
        TiledImage* image = images[i];
        const QVector<QRgb>& palette = image->getPalette();
        QMap<int, int> color_index_map;
        for(int p = 0; p < palette.size(); ++p)
        {
            QRgb color = palette[p];
            int np = -1;
            for(int sp = 0; sp < shared_colors.size(); ++sp)
            {
                if(shared_colors[sp] == color)
                {
                    np = sp;
                    break;
                }
            }

            if(np == -1)
            {
                shared_colors.append(color);
                np = shared_colors.size()-1;
            }

            color_index_map.insert(p, np);
        }
        color_index_maps.append(color_index_map);
    }

    // Set the palettes
    for(int i = 0; i < images.size(); ++i)
    {
        TiledImage* image = images[i];
        image->setPalette(shared_colors);
        image->setSharedPalette(out_shared_palette->getName());
    }

    // Shift the pixels
    for(int i = 0; i < images.size(); ++i)
    {
        TiledImage* image = images[i];
        QMap<int, int> color_index_map = color_index_maps[i];
        for(int j = 0; j < image->pixels.size(); ++j)
        {
            image->pixels[j] = color_index_map[image->pixels[j]];
        }
    }
}

void TiledImage::setPalette(const QVector<QRgb>& palette)
{
    this->palette = palette;
}

const QVector<QRgb>& TiledImage::getPalette() const
{
    return palette;
}

int TiledImage::getWidth() const
{
    return metadata["width"].toInt();
}

int TiledImage::getHeight() const
{
    return metadata["height"].toInt();
}

int TiledImage::getTileWidth() const
{
    return metadata["tile_width"].toInt();
}

int TiledImage::getTileHeight() const
{
    return metadata["tile_height"].toInt();
}

QString TiledImage::getSharedPalette() const
{
    if(usesSharedPalette())
        return metadata["shared_palette"];
    return "";
}

bool TiledImage::usesSharedPalette() const
{
    return metadata.contains("shared_palette");
}

void TiledImage::setWidth(int width)
{
    metadata.insert("width", QString::number(width));
}

void TiledImage::setHeight(int height)
{
    metadata.insert("height", QString::number(height));
}

void TiledImage::setTileWidth(int tile_width)
{
    metadata.insert("tile_width", QString::number(tile_width));
}

void TiledImage::setTileHeight(int tile_height)
{
    metadata.insert("tile_height", QString::number(tile_height));
}

void TiledImage::setSharedPalette(QString shared_palette)
{
    if(shared_palette.size() == 0)
        metadata.remove("shared_palette");
    else
        metadata.insert("shared_palette", shared_palette);
}

QString TiledImage::getPaletteId() const
{
    return getName() + GBA_PALETTE_SUFFIX;
}

QString TiledImage::getPixelsId() const
{
    return getName() + GBA_PIXELS_SUFFIX;
}
