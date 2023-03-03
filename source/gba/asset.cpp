#include "asset.h"
#include "gba.h"
#include "compiler/cgen.h"

void Asset::setName(QString name)
{
    metadata.insert("name", name);
}

QString Asset::getName() const
{
    return metadata["name"];
}

void Asset::setGame(Game* game)
{
    this->game = game;
}

Game* Asset::getGame() const
{
    return game;
}

void Asset::reset()
{
    metadata.clear();
}

void Asset::gatherAssets(Game* /*game*/)
{
}

void Asset::serialize(QTextStream& out)
{
    if(!metadata.count("name"))
    {
        setName(getDefaultName());
    }
    CGen::writeCommentMetadata(out, metadata);

    out << "#include " << "<" << GBA_ASSETS_HEADER << ">" << endl;

    writeDecls(out);

    QList<QPair<CGen::Type, QString>> fields;
    getStructFields(fields);
    if(fields.size())
    {
        QList<QString> field_data;
        writeStructData(field_data);
        CGen::writeStruct(out, getTypeName(), getName(), field_data);
    }

    writeData(out);
}

bool Asset::deserialize(QTextStream& in)
{
    reset();
    if(!CGen::readCommentMetadata(in, metadata))
    {
        return false;
    }

    QString include_line = in.readLine();
    if(include_line.size() == 0 && include_line[0] != '#')
        return false;

    if(!readDecls(in))
        return false;

    QList<QPair<CGen::Type, QString>> fields;
    getStructFields(fields);
    if(fields.size())
    {
        QList<QString> field_data;
        QString name;
        if(CGen::readStruct(in, getTypeName(), name, field_data))
        {
            setName(name);
            readStructData(field_data);
        }
    }
    return readData(in);
}

