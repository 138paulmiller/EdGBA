#ifndef COLORS_H
#define COLORS_H

#include <QColor>
#include <QVector>

#include "asset.h"

class Palette : public Asset, public QVector<QRgb>
{
public:
    virtual ~Palette() = default;

    static QVector<QRgb> translateFromGBAPalette(const QVector<int>& in_palette);
    static QVector<int> translateToGBAPalette(const QVector<QRgb>& in_palette);

    QString getPaletteDataId() const;

    void reset() override;
    QString getPath() const override;
    QString getDefaultName() const override;
    QString getTypeName() const override;

    void getStructFields(QList<QPair<CGen::Type, QString>>& out_fields) const override;
    void writeStructData(QList<QString>& out_field_data) override;
    void readStructData(QList<QString>& in_field_data) override;
    void writeDecls(QTextStream& out) override;
    bool readDecls(QTextStream& in) override;
    void writeData(QTextStream& out) override;
    bool readData(QTextStream& in) override;
};

#endif // COLORS_H
