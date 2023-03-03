#ifndef ASSET_H
#define ASSET_H

#include <QMap>
#include <QList>
#include <QString>
#include <QTextStream>

#include <compiler/cgen.h>

class Game;

class CStructInterface
{
public:
    virtual QString getTypeName() const = 0;
    virtual void getStructFields(QList<QPair<CGen::Type, QString>>& /*out_fields*/) const {}

    virtual void writeStructData(QList<QString>& /*out_field_data*/) {}
    virtual void readStructData(QList<QString>& /*in_field_data*/) {}

    virtual void writeDecls(QTextStream& /*out*/) {}
    virtual bool readDecls(QTextStream& /*in*/) { return true; }

    virtual void writeData(QTextStream& /*out*/) {}
    virtual bool readData(QTextStream& /*in*/) { return true; }
};

class Asset : public CStructInterface
{
protected:
    // Serialized/deserialized information about map name, width,height, affine, tileset used
    QMap<QString, QString> metadata;

    class Game* game;
public:
    virtual ~Asset() = default;

    virtual void reset();
    virtual QString getPath() const = 0;
    virtual QString getDefaultName() const = 0;
    virtual void gatherAssets(Game* game);

    void serialize(QTextStream& out);
    bool deserialize(QTextStream& in);

    void setName(QString name);
    QString getName() const;

    void setGame(Game* game);
    Game* getGame() const;

};

#endif // ASSET_H
