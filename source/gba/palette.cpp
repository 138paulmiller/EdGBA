#include "palette.h"
#include "game.h"
#include "gba.h"

// 15 bit GBA color to 32 bit RGB format
int GBA2RGBA(unsigned short gba_color)
{
    if(gba_color == GBA_COLORKEY_15BIT)
        return GBA_COLORKEY_RGB;

    int r = (gba_color & 0x1f);
    int g = ((gba_color >> 5) & 0x1f);
    int b = ((gba_color >> 10) & 0x1f);
    //remap from 15 to 16 bit
    r = ((float)r)/0x1f * 0xff;
    g = ((float)g)/0x1f * 0xff;
    b = ((float)b)/0x1f * 0xff;
    return (0xffu << 24) | ((r & 0xffu) << 16) | ((g & 0xffu) << 8) | (b & 0xffu);
}

// 32 bit RGB format to 15 bit GBA color
unsigned short RGBA2GBA(int rgba)
{
    int a = ((rgba >> 24) & 0xff);
    if(a == 0)
        return GBA_COLORKEY_15BIT;

    int r = ((rgba >> 16) & 0xff);
    int g = ((rgba >> 8) & 0xff);
    int b = (rgba & 0xff);
    return (((r >> 3) & 0x1f) | (((g >> 3) & 0x1f) << 5) | (((b >> 3) & 0x1f) << 10));
}

QVector<QRgb> Palette::translateFromGBAPalette(const QVector<int>& in_palette)
{
    QVector<QRgb> out_palette;

    for(int color_index = 0; color_index < in_palette.size(); ++color_index )
    {
        int color_15bit = in_palette[color_index];
        QRgb rgb = GBA2RGBA(color_15bit);

        out_palette.append(rgb);
    }
    return out_palette;
}

QVector<int> Palette::translateToGBAPalette(const QVector<QRgb>& in_palette)
{
    QVector<int> out_palette;
    for(int color_index = 0; color_index < in_palette.size(); ++color_index )
    {
        QRgb color = in_palette[color_index];
        int color_15bit  = RGBA2GBA(color);
        out_palette.append(color_15bit);
    }
    return out_palette;
}

QString Palette::getPaletteDataId() const
{
    return getName() + GBA_COLORS_SUFFIX;
}

void Palette::reset()
{
    setName(getDefaultName());
    this->clear();
}

QString Palette::getPath() const
{
    return GBA_PALETTES_PATH;
}

QString Palette::getDefaultName() const
{
    return GBA_DEFAULT_PALETTE_NAME;
}

QString Palette::getTypeName() const
{
    return GBA_PALETTE_TYPE;
}

void Palette::getStructFields(QList<QPair<CGen::Type, QString>>& out_fields) const
{
    out_fields.push_back(qMakePair(CGen::Type::UNSIGNED_SHORT, QString("size")));
    out_fields.push_back(qMakePair(CGen::Type::CONST_PTR_UNSIGNED_SHORT, QString("colors")));
}

void Palette::writeStructData(QList<QString>& out_field_data)
{
    // Note: this must match the field order
    out_field_data.append(QString::number(size()));
    out_field_data.append(getPaletteDataId());
}

void Palette::readStructData(QList<QString>& in_field_data)
{
    if(in_field_data.size() == 0)
        return;

    //setPaletteSize(in_field_data.front().toInt());
    in_field_data.pop_front();
    if(in_field_data.size() == 0)
        return;

    //setPaletteId(in_field_data.front().toInt());
    in_field_data.pop_front();
    if(in_field_data.size() == 0)
        return;
}


void Palette::writeDecls(QTextStream& out)
{
    CGen::writeArrayDecl(out, CGen::CONST_UNSIGNED_SHORT, getPaletteDataId());
}

bool Palette::readDecls(QTextStream& in)
{
    QString palette_id, type;
    if(!CGen::readArrayDecl(in, type, palette_id))
        return false;
    return true;
}

void Palette::writeData(QTextStream& out)
{
    CGen::ArrayWriter array_writer(out);

    QVector<int> palette_data = translateToGBAPalette(*this);
    array_writer.writeAllValues(CGen::CONST_UNSIGNED_SHORT, getPaletteDataId(), palette_data);
}

bool Palette::readData(QTextStream& in)
{
    CGen::ArrayReader array_reader(in);

    QVector<int> palette_data;
    QString id;
    if(!array_reader.readAllValues(CGen::CONST_UNSIGNED_SHORT, id, palette_data))
    {
        return false;
    }
    *this += translateFromGBAPalette(palette_data);
    return true;
}
