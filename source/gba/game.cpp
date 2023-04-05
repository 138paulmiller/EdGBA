#include "game.h"
#include "gba.h"
#include <msglog.h>
#include <compiler/cgen.h>
#include <common.h>

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QSettings>

#define HEADER_TAG  QString("/*** Generated by EdGBA ***/")
#define MESSAGE_TAG QString("/*** ! Do not modify !  ***/")

#define TILESET_TAG QString("/*** Tileset ***/")
#define MAP_TAG     QString("/*** Map ***/")

// TODO: Add undo redo buffer. Push/Pop assets off stack with a action (add, remove, etc..)

Game::~Game()
{
    reset();
}

void Game::newGame(QString project_path)
{
    reset();
    SourceFile* main_source = addSourceFile("main.c");
    main_source->setContent("");

    QFileInfo info(project_path);
    name = info.baseName();
}

void Game::reset()
{
    is_dirty = false;
    project_file= "";
    name = GBA_DEFAULT_GAME_NAME;

    foreach(SourceFile* source_file, source_files)
    {
        delete source_file;
    }
    source_files.clear();

    foreach(QString key, asset_table.keys())
    {
        foreach(Asset* asset, asset_table[key])
        {
            delete asset;
        }
    }
    asset_table.clear();
}

bool Game::isValid() const
{
    return project_file.size() > 0;
}

bool Game::isDirty() const
{
    return is_dirty;
}

void Game::markDirty()
{
    is_dirty = true;
}

QString Game::getName() const
{
    return name;
}

QString Game::getAbsoluteProjectPath() const
{
    return Common::absolutePath(project_file);
}

QString Game::getAbsoluteProjectFile() const
{
    return Common::absoluteFile(project_file);
}

QString Game::getAbsoluteCodePath() const
{
    return getAbsoluteProjectPath() + GBA_CODE_PATH;
}

QString Game::getAbsoluteGeneratedPath() const
{
    return getAbsoluteProjectPath() + GBA_GENERATED_PATH;
}

QSettings* Game::getSettings()
{
    QFile f(project_file);
    if(!f.exists())
    {
        f.open(QIODevice::ReadWrite);
        f.close();
    }
    return new QSettings(project_file, QSettings::IniFormat);
}

void Game::reloadSourceFiles()
{
    foreach(SourceFile* source_file, source_files)
    {
        source_file->save();
        delete source_file;
    }
    source_files.clear();

    QStringList filters;
    filters << "*.s" << "*.c" << "*.h";
    QString dir = getAbsoluteCodePath();
    QDirIterator it(dir, filters, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        addSourceFile(it.next());
    }
}

QStringList Game::getHeaderFiles() const
{
    QStringList files;
    foreach(const SourceFile* source, source_files)
    {
        if(source->isHeader())
            files << source->getFilePath();
    }
    return files;
}

QStringList Game::getSourceFiles() const
{
    QStringList files;
    foreach(const SourceFile* source, source_files)
    {
        if(!source->isHeader())
            files << source->getFilePath();
    }
    return files;
}

void Game::saveAs(const QString& in_project_file)
{
    project_file = in_project_file;
    // TODO: copy over all other file data ?
    save();
}

void Game::save()
{
    is_dirty = false;
    QSettings* settings = getSettings();
    if(settings == nullptr)
    {
        return;
    }
    settings->setValue("name", name);

    // TODO: write to temp file. THen on complete,  remove existing maps and tilesets then copy over
    QDir(getAbsoluteGeneratedPath()).removeRecursively();
    QDir(getAbsoluteGeneratedPath()).mkpath(".");

    foreach(SourceFile* source_file, source_files)
    {
        source_file->save();
    }

    // Save assets
    foreach(QString key, asset_table.keys())
    {
        foreach(Asset* asset, asset_table[key])
        {
            QString abs_path = getAbsoluteGeneratedPath() + asset->getPath();
            if(abs_path.size() == 0 || abs_path[abs_path.size()-1] != '/')
            {
                abs_path += '/';
            }

            QDir(abs_path).mkpath(".");

            QString asset_source = abs_path + asset->getName() + ".c";
            QTextStream* stream = openOutputStream(asset_source);
            if(stream)
            {
                asset->serialize(*stream);
                closeStream(stream);
            }
        }
    }

    // Create the Assets.h API
    saveAssetsHeader();
}

void Game::saveAssetsHeader()
{
    QString asset_header = getAbsoluteGeneratedPath() + GBA_ASSETS_HEADER;
    QTextStream* stream = openOutputStream(asset_header);

    if(stream)
    {
        *stream << "#ifndef __ASSETS_H__" << endl;
        *stream << "#define __ASSETS_H__" << endl << endl;

        foreach(const QList<Asset*>& assets, asset_table)
        {
            bool write_struct_def = true;
            foreach(Asset* asset, assets)
            {
                if(write_struct_def)
                {
                    QString type = asset->getTypeName();

                    QList<QPair<CGen::Type, QString>> fields;
                    asset->getStructFields(fields);

                    CGen::writeStructDef(*stream, type, fields);

                    write_struct_def = false;
                    *stream << endl;
                }

                CGen::writeStructDecl(*stream, asset->getTypeName(), asset->getName());
            }
            *stream << endl;
        }

        *stream << "#endif //__ASSETS_H__";
        closeStream(stream);
    }
}

void Game::rebuildPalettes()
{
    QList<Tileset*> tilesets = getAssets<Tileset>();
    Tileset::syncPalettes(tilesets, getTilesetPalette());

    QList<SpriteSheet*> spritesheets = getAssets<SpriteSheet>();
    SpriteSheet::syncPalettes(spritesheets, getSpritePalette());
}

bool Game::load(const QString& in_project_file)
{
    reset();
    project_file = in_project_file;

    QSettings* settings = getSettings();
    if(settings == nullptr)
    {
        return false;
    }

    foreach (QString key, settings->childKeys())
    {
        QString value = settings->value(key).toString();
        if(key == "name")
        {
            if(value.size() > 0)
            {
                name = value;
            }
        }
    }
    delete settings;

    // Gather code source and header files
    {
        QStringList filters;
        filters << "*.c" << "*.h";
        QString dir = getAbsoluteCodePath();
        QDirIterator it(dir, filters, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            addSourceFile(it.next());
        }
    }

    const QString asset_dir = getAbsoluteGeneratedPath();
    // Spawn assets according to path
    QStringList filters;
    filters << "*.c";
    QDirIterator file_it(asset_dir, filters, QDir::Files, QDirIterator::Subdirectories);

    QList<Asset*> new_assets;
    while (file_it.hasNext())
    {
        Asset* new_asset = nullptr;
        QString asset_source = file_it.next();
        QString asset_path = QFileInfo(asset_source).path();
        if(asset_path.size() == 0 || asset_path[asset_path.size()-1] != '/')
        {
            asset_path += '/';
        }

        QTextStream* stream = openInputStream(asset_source);
        if(!stream)
        {
            continue;
        }

        // TODO: automate this
        if(asset_path == asset_dir + Map().getPath())
        {
            new_asset = addAsset<Map>();
        }
        if(asset_path == asset_dir + Tileset().getPath())
        {
            new_asset = addAsset<Tileset>();
        }
        if(asset_path == asset_dir + SpriteSheet().getPath())
        {
            new_asset = addAsset<SpriteSheet>();
        }
        if(asset_path == asset_dir + SpriteAnim().getPath())
        {
            new_asset = addAsset<SpriteAnim>();
        }
        if(asset_path == asset_dir + Palette().getPath())
        {
            new_asset = addAsset<Palette>();
        }

        if(new_asset && !new_asset->deserialize(*stream))
        {
            removeAsset(new_asset);
        }
        else
        {
            new_assets.push_back(new_asset);
        }

        closeStream(stream);
    }

    foreach(Asset* asset, new_assets)
    {
        asset->gatherAssets(this);
    }

    return true;
}

void Game::checkNames()
{
    // Check all tileset name against spritesheets and vice versa
    //foreach(Tileset* spritesheet, spritesheets)
    //{
    //    if(new_tileset->getName() == spritesheet->getName())
    //    {
    //        new_tileset->setName(name + QString::number(++counter));
    //    }
    //}
}

Palette* Game::getTilesetPalette()
{
    Palette* palette = findAsset<Palette>(GBA_SHARED_TILESET_PALETTE_NAME);
    if(palette == nullptr)
    {
        palette = addAsset<Palette>();
        palette->setName(GBA_SHARED_TILESET_PALETTE_NAME);
    }
    return palette;
}
Palette* Game::getSpritePalette()
{
    Palette* palette = findAsset<Palette>(GBA_SHARED_SPRITE_PALETTE_NAME);
    if(palette == nullptr)
    {
        palette = addAsset<Palette>();
        palette->setName(GBA_SHARED_SPRITE_PALETTE_NAME);
    }
    return palette;
}

QTextStream* Game::openInputStream(const QString& file_path)
{
    QFile* file = new QFile(file_path);
    if(file == nullptr)
    {
        return nullptr;
    }
    file->open(QIODevice::ReadOnly);

    QTextStream* stream = new QTextStream(file);
    if(stream == nullptr)
    {
        file->close();
        delete file;
        return nullptr;
    }
    QString line = stream->readLine();
    if (line != HEADER_TAG)
    {
        closeStream(stream);
        return nullptr;
    }
    return stream;
}

QTextStream* Game::openOutputStream(const QString& file_path)
{
    QFile* file = new QFile(file_path);
    if(file == nullptr)
    {
        return nullptr;
    }
    file->open(QIODevice::WriteOnly);

    QTextStream* stream = new QTextStream(file);
    if(stream == nullptr)
    {
        file->close();
        delete file;
        return nullptr;
    }
    *stream << HEADER_TAG << endl;
    return stream;
}

void Game::closeStream(QTextStream* stream)
{
    if(stream)
    {
        if(stream->device())
        {
            stream->device()->close();
            delete stream->device();
        }
        delete stream;
    }
}

SourceFile* Game::addSourceFile(const QString& file_path)
{
    SourceFile* source_file = new SourceFile(file_path);
    source_files.push_back(source_file);
    return source_file;
}

void Game::removeSourceFile(SourceFile* source_file)
{
    auto it = source_files.begin();
    while(it != source_files.end())
    {
        if(*it == source_file)
        {
            source_files.erase(it);
            delete source_file;
            return;
        }
        it++;
    }
}

SourceFile* Game::findSourceFile(const QString& file_path)
{
    QFileInfo info(file_path);
    QString full_file_path = info.absoluteFilePath();

    foreach(SourceFile* source_file, source_files)
    {
        if(full_file_path == source_file->getFilePath())
        {
            return source_file;
        }
    }
    return nullptr;
}
