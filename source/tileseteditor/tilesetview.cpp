/* paletteview.cpp
 * implementation of the palette view */

#include <stdio.h>
#include <QMouseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include "tilesetview.h"

TilesetModel::TilesetModel(QObject* parent)
    : TiledImageModel(parent)
{}

void TilesetModel::setTileset(Tileset* new_tileset)
{
    TiledImageModel::setTiledImage(new_tileset);
}

Tileset* TilesetModel::getTileset() const
{
    TiledImage* image = TiledImageModel::getTiledImage();
    return dynamic_cast<Tileset*>(image);
}

TilesetView::TilesetView(QWidget* parent):
    TiledImageView(parent)
{}

void TilesetView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
    {
        return;
    }
    int x,y;
    TiledImageView::getCellXY(event, x,y);
    emit tileClicked(x, y);
}
