/* mapview.cpp
 * the map view implementation file */

#include "mapview.h"

#include <QMouseEvent>
#include <QMessageBox>

MapModel::MapModel(QObject* parent)
    : TiledImageModel(parent)
    , map(nullptr)
    , tileset(nullptr)
    , view(nullptr)
{
}
void MapModel::setMap(Map* new_map)
{
    if(map != new_map)
    {
        map = new_map;
        //if(map)
        //{
        //    tileset = map->getTileset();
        //}
        if(view)
        {
            view->redraw();
        }
        return;
    }

    //if(map && map->getTileset() != tileset)
    //{
    //    tileset = map->getTileset();
    //    if(view)
    //    {
    //        view->redraw();
    //    }
    //}
}

Map* MapModel::getMap() const
{
    return map;
}

void MapModel::markTileDirty(int tile_x, int tile_y)
{
    if(tile_x * GBA_TILE_SIZE >= map->getPixelWidth()
    || tile_y * GBA_TILE_SIZE >= map->getPixelHeight())
        return;
    TileChange change;
    change.tile_x = tile_x;
    change.tile_y = tile_y;
    tile_changes.push_back(change);
}

void MapModel::render(QImage& out_image)
{
    // render map to image, then draw to view
    if(map)
    {
        if(tile_changes.size())
        {
            while(tile_changes.size())
            {
                TileChange change = tile_changes.back();
                tile_changes.pop_back();
                map->renderTile(cached_image, change.tile_x, change.tile_y);
            }
            out_image = cached_image;
        }
        else
        {
            map->render(out_image);
            cached_image = out_image;
        }
    }
}

MapView::MapView(QWidget* parent)
    : TiledImageView(parent)
    , model( nullptr)
    , dragging(false)
{
    QGraphicsScene* scene = new QGraphicsScene(parent);
    scene->setBackgroundBrush(Qt::transparent);

    setScene(scene);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
}

void MapView::mouseMoveEvent(QMouseEvent* event)
{
    if(!dragging)
    {
        return;
    }

    static int px = 0, py = 0;
    int x,y;
    TiledImageView::getCellXY(event, x,y);

    if(px != x || py != y)
    {
        placeTileAt(x, y);
        px = x; py = y;
    }
}

void MapView::placeTileAt(int x, int y)
{
    emit clicked(x,y);
    redraw();
}

void MapView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        dragging = false;
    }
}

void MapView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        dragging = true;
    }
    int x,y;
    TiledImageView::getCellXY(event, x,y);
    placeTileAt(x, y);
}
