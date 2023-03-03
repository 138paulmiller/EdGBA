#ifndef SPRITESHEETVIEW_H
#define SPRITESHEETVIEW_H

#include <ui/tiledimageview.h>
#include <gba/spritesheet.h>

class SpriteSheetModel : public TiledImageModel
{
    Q_OBJECT

public:
    SpriteSheetModel(QObject* parent);
    void setSpriteSheet(SpriteSheet* SpriteSheet);
    SpriteSheet* getSpriteSheet() const;
};

class SpriteSheetView : public TiledImageView
{
    Q_OBJECT

public:
    SpriteSheetView(QWidget* parent);
    void mousePressEvent (QMouseEvent* event);

signals:
    void frameClicked(int tilex, int tiley);
};

#endif

