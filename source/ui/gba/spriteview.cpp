/* paletteview.cpp
 * implementation of the palette view */

#include <stdio.h>
#include <QMouseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include "spriteview.h"

SpriteModel::SpriteModel(QObject* parent)
    : TiledImageModel(parent)
{
    frame = -1;
    connect(&timer, SIGNAL(timeout()), this, SLOT(on_nextFrame()));
}

void SpriteModel::start(int msec)
{
    if(timer.isActive() && timer.interval() != msec)
    {
        timer.stop();
        frame = -1;
    }

    if(frame == -1)
    {
        if(msec > 0)
            timer.start(msec);
        else
        {
            view->invalidate();
            view->redraw();
        }
    }
}

void SpriteModel::stop()
{
    frame = -1;
    timer.stop();
}

void SpriteModel::setSpriteAnim(SpriteAnim* spriteanim)
{
    this->spriteanim = spriteanim;
}

SpriteAnim* SpriteModel::getSpriteAnim() const
{
    return spriteanim;
}

void SpriteModel::setSpriteSheet(SpriteSheet* new_SpriteSheet)
{
    TiledImageModel::setTiledImage(new_SpriteSheet);
}

SpriteSheet* SpriteModel::getSpriteSheet() const
{
    TiledImage* image = TiledImageModel::getTiledImage();
    return dynamic_cast<SpriteSheet*>(image);
}

void SpriteModel::render(QImage& out_image)
{
    SpriteSheet* spritesheet = getSpriteSheet();
    if(spriteanim == nullptr || spritesheet == nullptr)
    {
        view->clear();
        return;
    }

    if(frame == -1)
    {
        if(spriteanim->getFrameCount())
        {
            const int default_frame = spriteanim->getFrame(0);
            const bool hflip = spriteanim->getHFlip();
            const bool vflip = spriteanim->getVFlip();
            spritesheet->renderFrame(out_image, default_frame, hflip, vflip);
        }

        view->clear();
        return;
    }

    if(frame < spriteanim->getFrameCount())
    {
        int current_frame_index = spriteanim->getFrame(frame);
        const bool hflip = spriteanim->getHFlip();
        const bool vflip = spriteanim->getVFlip();
        spritesheet->renderFrame(out_image, current_frame_index, hflip, vflip);
    }
}

void SpriteModel::on_nextFrame()
{
    // not visible
    if(this->view->visibleRegion().isEmpty()) return;

    if(spriteanim != nullptr && spriteanim->getFrameCount() > 0)
    {
        frame = (frame + 1) % spriteanim->getFrameCount();
    }
    else
    {
        frame = -1;
    }
    view->invalidate();
    view->redraw();
}

SpriteView::SpriteView(QWidget* parent)
    : TiledImageView(parent)
{}

