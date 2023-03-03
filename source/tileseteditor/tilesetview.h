#ifndef TILESETVIEW_H
#define TILESETVIEW_H

#include <ui/tiledimageview.h>
#include <gba/tileset.h>

class TilesetModel : public TiledImageModel
{
public:
    TilesetModel(QObject* parent);
    void setTileset(Tileset* tileset);
    Tileset* getTileset() const;
};

class TilesetView : public TiledImageView
{
Q_OBJECT

public:
    TilesetView(QWidget* parent);
    void mousePressEvent (QMouseEvent* event);

signals:
    void tileClicked(int tilex, int tiley);
};

#endif

