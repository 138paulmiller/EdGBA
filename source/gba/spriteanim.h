#ifndef SPRITEANIM_H
#define SPRITEANIM_H

#include <QVector>

#include "asset.h"

/*
 //TODO Create additional frame metadata
struct SpriteFrame
{
    int tile; //tile indices in the sprite sheet
    bool flipped;

    QString getTypeName();
};
*/

class SpriteAnim : public Asset
{
private:
    QVector<int> frames;

public:
    SpriteAnim();

    void reset() override;
    QString getPath() const override;
    QString getTypeName() const override;
    QString getDefaultName() const override;
    void getStructFields(QList<QPair<CGen::Type, QString>>& out_fields) const override;
    void writeStructData(QList<QString>& out_field_data) override;
    void readStructData(QList<QString>& in_field_data) override;
    void writeDecls(QTextStream& out) override;
    bool readDecls(QTextStream& in) override;
    void writeData(QTextStream& out) override;
    bool readData(QTextStream& in) override;

    QString getFramesId() const;

    bool getHFlip() const;
    void setHFlip(bool flipped);
    bool getVFlip() const;
    void setVFlip(bool flipped);

    int getFrameDuration() const;
    void setFrameDuration(int duration);

    void fillFrames(int count);
    int getFrameCount() const;
    int getFrame(int index) const;
    void setFrame(int index, const int frame);
    void setFrames(const QVector<int>& frames);
};

#endif // SPRITEANIM_H
