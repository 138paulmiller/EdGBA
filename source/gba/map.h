#ifndef MAP_H
#define MAP_H

#include "gba.h"
#include "tileset.h"
#include <QStack>
#include <functional>


//TODO: cache the Map image for Overworld rendering ?

class Map;
class Background : public CStructInterface
{
public:
    friend class Map;
    Map* map;
    int bg_index;
    QVector<int> tiles;
    QVector<bool> vflips;
    QVector<bool> hflips;
    Tileset* tileset;
    QString tileset_name; //cached to setup association after load

    int size_flag;
    int scroll_x, scroll_y;
    int priority;

    void resize(int new_size_flag);
    void reset();

    void forEachTileIndex(std::function<void(int)> callback);

    int getWidth();
    int getHeight();

    void render(QImage& out_image) const;
    void renderTile(QImage& out_image, int x, int y, int tile_width = GBA_TILE_SIZE, int tile_height = GBA_TILE_SIZE) const;

    QString getTilesId() const;
    QString getTilesetId() const;
    void gatherAssets(Game* game);

    QString getTypeName() const override;
    void getStructFields(QList<QPair<CGen::Type, QString>>& out_fields) const override;
    void writeStructData(QList<QString>& out_field_data) override;
    void readStructData(QList<QString>& in_field_data) override;
    void writeDecls(QTextStream& out) override;
    bool readDecls(QTextStream& in) override;
    void writeData(QTextStream& out) override;
    bool readData(QTextStream& in) override;
};

class Map : public Asset
{
private:
    Background backgrounds[GBA_BG_COUNT];

    QList<QVector<Background>> undo_stack;
    QList<QVector<Background>> redo_stack;

public:
    Map();
    ~Map();

    static QStringList getMapModeNames();
    static int getMapMode(QString mode_name);
    static QString getMapModeName(int mode);

    static bool getBackgroundEnabled(int mode, int bg_index);
    static bool getBackgroundAffine(int mode, int bg_index);
    static QStringList getBackgroundSizeNames(int mode, int bg_index);
    static QVector<int> getBackgroundSizeFlags(int mode, int bg_index);
    static QString getBackgroundSizeName(int size_flag);
    static int getBackgroundSizeFlag(QString name);
    static int getBackgroundSizeFlag(int width, int height, bool affine);

    static int getBackgroundSizeFlagWidth(int size_flag);
    static int getBackgroundSizeFlagHeight(int size_flag);
    static bool getBackgroundSizeFlagAffine(int size_flag);


    int getPixelWidth() const;
    int getPixelHeight() const;

    void setMode(int mode);
    int getMode() const;

    Background* getBackground(int bg_index);

    void enableBackground(int bg_index);
    void disableBackground(int bg_index);

    void resizeBackground(int bg_index, int size_flag);
    int getBackgroundSize(int bg_index) const;
    void syncBackgrounds();

    void setTile(int bg_index, int index, int tile_index, bool hflip, bool vflip);
    QString getTilesId(int bg_index) const;

    Tileset* getTileset(int bg_index);
    void setTileset(int bg_index, Tileset* new_tileset);
    void setTilesetName(int bg_index, QString name);
    QString getTilesetName(int bg_index);

    bool getAffine(int bg_index) const;

    int getPriority(int bg_index);
    void setPriority(int bg_index, int priority);

    void render(QImage& out_image) const;
    void renderTile(QImage& out_image, int x, int y, int tile_width = GBA_TILE_SIZE, int tile_height = GBA_TILE_SIZE) const;

    void undo();
    void redo();

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
    void gatherAssets(Game* game) override;
};

#endif


