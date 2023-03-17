#ifndef MAPVIEW_h
#define MAPVIEW_h

#include <gba/map.h>
#include <ui/gba/tiledimageview.h>

class MapModel : public TiledImageModel
{
    Q_OBJECT
private:

    Map* map;
    Tileset* tileset; //cached previous tileset
    QImage cached_image;

    friend class MapView;
    class MapView* view;

    struct TileChange
    {
        int tile_x, tile_y;
    };
    QVector<TileChange> tile_changes;

public:
    MapModel(QObject* parent);
    void setMap(Map* map);
    Map* getMap() const;

    void markTileDirty(int tile_x, int tile_y);

    void render(QImage& out_image);
};

class MapView : public TiledImageView
{
Q_OBJECT

private:
    MapModel* model;

    bool dragging;

    void placeTileAt(int x, int y);
public:
    MapView(QWidget* parent);

    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

signals:
    void clicked(int image_x, int image_y);

};

#endif

