#ifndef GAME_H
#define GAME_H

#include "gba.h"
#include "asset.h"
#include "map.h"
#include "tileset.h"
#include "spriteanim.h"
#include "spritesheet.h"
#include "sourcefile.h"
#include "palette.h"

#include <QList>
#include <QSettings>

class Game
{
public:
    ~Game();

    bool is_dirty;
    QString name;
    QString project_file;

    QList<SourceFile*> source_files;
    // Maps from asset type name to instances
    QMap<QString, QList<Asset*>> asset_table;

public:
    // create default game
    void newGame(QString project_path);
    void reset();
    bool isValid() const;
    bool isDirty() const;
    void markDirty();

    void saveAs(const QString& project_dir);
    void save();
    bool load(const QString& project_file);

    void saveAssetsHeader();

    // Utils to sync game data
    void rebuildPalettes();
    void reloadSourceFiles();
    void checkNames();

    Palette* getTilesetPalette();
    Palette* getSpritePalette();

    QString getName() const;
    QString getAbsoluteProjectPath() const;
    QString getAbsoluteProjectFile() const;
    QString getAbsoluteCodePath() const;
    QString getAbsoluteGeneratedPath() const;

    QTextStream* openInputStream(const QString& file_path);
    QTextStream* openOutputStream(const QString& file_path);
    void closeStream(QTextStream* stream);

    SourceFile* addSourceFile(const QString& file_name);
    void removeSourceFile(SourceFile* source_file);
    SourceFile* findSourceFile(const QString& file_name);

    QStringList getHeaderFiles() const;
    QStringList getSourceFiles() const;

    template<typename AssetType, typename... Args>
    AssetType* addAsset(Args&&...args);

    template<typename AssetType>
    AssetType* findAsset(const QString& name);

    template<typename AssetType>
    void removeAsset(AssetType* asset);

    template<typename Type>
    QList<Type*> getAssets() const;

private:
    QSettings* getSettings();
};

template<typename AssetType, typename... Args>
AssetType* Game::addAsset(Args&&...args)
{
    static_assert(std::is_base_of<Asset, AssetType>::value, "AssetType must derive from Asset");

    AssetType* new_asset = new AssetType(args...);
    new_asset->reset();
    new_asset->setGame(this);

    const QString name =  new_asset->getName();
    const QString type =  new_asset->getTypeName();

    if(asset_table.contains(type))
    {
        int counter = 0;
        foreach(Asset* check_asset, asset_table[type])
        {
            if(name == check_asset->getName())
            {
                new_asset->setName(name + QString::number(++counter));
            }
        }
    }
    asset_table[type].push_back(new_asset);
    return new_asset;
}

template<typename AssetType>
AssetType* Game::findAsset(const QString& name)
{
    static_assert(std::is_base_of<Asset, AssetType>::value, "AssetType must derive from Asset");

    const QString type = AssetType().getTypeName();
    if(asset_table.contains(type))
    {
        foreach(Asset* check_asset, asset_table[type])
        {
            if(name == check_asset->getName())
            {
               return dynamic_cast<AssetType*>(check_asset);
            }
        }
    }
    return nullptr;
}

template<typename AssetType>
void Game::removeAsset(AssetType* asset)
{
    static_assert(std::is_base_of<Asset, AssetType>::value, "AssetType must derive from Asset");

    if(asset == nullptr) return;

    const QString type = asset->getTypeName();
    if(asset_table.contains(type))
    {
        QList<Asset*>& assets = asset_table[type];
        assets.removeAll(asset);
        delete asset;
    }
}

template<typename AssetType>
QList<AssetType*> Game::getAssets() const
{
    static_assert(std::is_base_of<Asset, AssetType>::value, "AssetType must derive from Asset");

    QList<AssetType*> out_assets;

    const QString type = AssetType().getTypeName();
    if(asset_table.contains(type))
    {
        foreach(Asset* asset, asset_table[type])
        {
            if(AssetType* cast_asset = dynamic_cast<AssetType*>(asset))
            {
                out_assets.push_back(cast_asset);
            }
        }
    }
    return out_assets;
}

#endif // GAME_H
