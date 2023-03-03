/* paletteview.cpp
 * implementation of the palette view */

#include <stdio.h>
#include <QMouseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include "spritesheetview.h"

SpriteSheetModel::SpriteSheetModel(QObject* parent)
    :TiledImageModel(parent)
{
}

void SpriteSheetModel::setSpriteSheet(SpriteSheet* new_SpriteSheet)
{
    TiledImageModel::setTiledImage(new_SpriteSheet);
}

SpriteSheet* SpriteSheetModel::getSpriteSheet() const
{
    TiledImage* image = TiledImageModel::getTiledImage();
    return dynamic_cast<SpriteSheet*>(image);
}

SpriteSheetView::SpriteSheetView(QWidget* parent):
    TiledImageView(parent)
{}


void SpriteSheetView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
    {
        return;
    }
    int x,y;
    TiledImageView::getCellXY(event, x,y);
    emit frameClicked(x, y);
}
