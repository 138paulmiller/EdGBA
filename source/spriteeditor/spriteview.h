#ifndef SPRITEVIEW_H
#define SPRITEVIEW_H

#include <QVector>
#include <QTimer>
#include <ui/tiledimageview.h>
#include <gba/spriteanim.h>
#include <gba/spritesheet.h>

class SpriteModel : public TiledImageModel
{
    Q_OBJECT

private:
    int frame;
    SpriteAnim* spriteanim;
    QTimer timer;

public:
    SpriteModel(QObject* parent);

    void start(int msec);
    void stop();

    void setSpriteAnim(SpriteAnim* sprite_anim);
    SpriteAnim* getSpriteAnim() const;

    void setSpriteSheet(SpriteSheet* SpriteSheet);
    SpriteSheet* getSpriteSheet() const;

    virtual void render(QImage& out_image);

public slots:
    void on_nextFrame();
};

class SpriteView : public TiledImageView
{
    Q_OBJECT

public:
    SpriteView(QWidget* parent);
};

#endif

