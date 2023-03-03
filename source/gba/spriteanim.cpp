#include "spriteanim.h"

#include "gba.h"
#include <compiler/cgen.h>

//QString SpriteFrame::getTypeName()
//{
//    return GBA_SPRITEFRAME_TYPE;
//}

SpriteAnim::SpriteAnim()
{
    reset();
}

void SpriteAnim::reset()
{
    setName(GBA_DEFAULT_SPRITEANIM_NAME);
    setFrameDuration(0);
    setHFlip(false);
    setVFlip(false);
    frames.clear();
}

QString SpriteAnim::getPath() const
{
    return GBA_SPRITEANIMS_PATH;
}

QString SpriteAnim::getTypeName() const
{
    return GBA_SPRITEANIM_TYPE;
}

QString SpriteAnim::getDefaultName() const
{
    return GBA_DEFAULT_SPRITEANIM_NAME;
}

void SpriteAnim::getStructFields(QList<QPair<CGen::Type, QString>>& out_fields) const
{
    out_fields.push_back(qMakePair(CGen::Type::UNSIGNED_SHORT, QString("hflip")));
    out_fields.push_back(qMakePair(CGen::Type::UNSIGNED_SHORT, QString("vflip")));
    out_fields.push_back(qMakePair(CGen::Type::UNSIGNED_SHORT, QString("frame_duration")));
    out_fields.push_back(qMakePair(CGen::Type::UNSIGNED_CHAR, QString("frame_count")));
    out_fields.push_back(qMakePair(CGen::Type::CONST_PTR_UNSIGNED_CHAR, QString("frames")));
    //out_fields.push_back(qMakePair(CGen::Type::CONST_STRUCT, SpriteFrame().getTypeName() + " frames[]"));
}

void SpriteAnim::writeStructData(QList<QString>& out_field_data)
{
    out_field_data.append(QString::number(getHFlip()));
    out_field_data.append(QString::number(getVFlip()));
    out_field_data.append(QString::number(getFrameDuration()));
    out_field_data.append(QString::number(getFrameCount()));
    out_field_data.append(getFramesId());
}

void SpriteAnim::readStructData(QList<QString>& in_field_data)
{
    // Note: this must match the field order
    setHFlip(in_field_data[0].toInt() != 0);
    in_field_data.pop_front();
    if(in_field_data.size() == 0)
        return;

    setVFlip(in_field_data[0].toInt() != 0);
    in_field_data.pop_front();
    if(in_field_data.size() == 0)
        return;

    setFrameDuration(in_field_data[0].toInt());
    in_field_data.pop_front();
    if(in_field_data.size() == 0)
        return;

    // in_field_data[0] skip frames, these are loaded in data
}

void SpriteAnim::writeDecls(QTextStream& out)
{
    CGen::writeArrayDecl(out, CGen::Type::CONST_UNSIGNED_CHAR, getFramesId());
    //CGen::writeArrayDecl(out, SpriteFrame().getTypeName(), getFramesId());
}

bool SpriteAnim::readDecls(QTextStream& in)
{
    QString type, tiles_id;
    return CGen::readArrayDecl(in, type, tiles_id);
}

void SpriteAnim::writeData(QTextStream& out)
{
    CGen::ArrayWriter array_writer(out);
    array_writer.writeAllValues(CGen::CONST_UNSIGNED_CHAR, getFramesId(), frames);
}

bool SpriteAnim::readData(QTextStream& in)
{
    frames.clear();

    QString id;
    CGen::ArrayReader array_reader(in);
    return array_reader.readAllValues(CGen::CONST_UNSIGNED_CHAR, id, frames);
    //return true;
}

QString SpriteAnim::getFramesId() const
{
    return getName() + "_frames";
}

int SpriteAnim::getFrameDuration() const
{
    return metadata["frame_duration"].toInt();
}

void SpriteAnim::setFrameDuration(int duration)
{
    metadata["frame_duration"] = QString::number(duration);
}

bool SpriteAnim::getHFlip() const
{
    return metadata["hflip"].toInt();
}

void SpriteAnim::setHFlip(bool flipped)
{
    metadata["hflip"] = QString::number(flipped ? 1 : 0);
}

bool SpriteAnim::getVFlip() const
{
    return metadata["vflip"].toInt();
}

void SpriteAnim::setVFlip(bool flipped)
{
    metadata["vflip"] = QString::number(flipped ? 1 : 0);
}

void SpriteAnim::fillFrames(int count)
{
    frames.fill(0, count);
}

int SpriteAnim::getFrameCount() const
{
    return frames.size();
}

int SpriteAnim::getFrame(int index) const
{
    return frames[index];
}

void SpriteAnim::setFrame(int index, int frame)
{
    frames[index] = frame;
}

void SpriteAnim::setFrames(const QVector<int>& frames)
{
    this->frames = frames;
}

