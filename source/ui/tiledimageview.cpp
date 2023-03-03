#include "tiledimageview.h"
#include <QMouseEvent>
#include <QScrollBar>
#include <QGraphicsPixmapItem>

static void renderGrid(QImage& image, int grid_size_x, int grid_size_y, QColor grid_color)
{
    if(grid_size_x < 0 || grid_size_y < 0)
        return;
    for (int y = grid_size_y; y < image.height(); y += grid_size_y)
    {
        for (int x = 0; x < image.width(); x++)
        {
            image.setPixel(x, y, grid_color.rgba());
        }
    }

    for (int x = grid_size_x; x < image.width(); x += grid_size_x)
    {
        for (int y = 0; y < image.height(); y++)
        {
            image.setPixel(x, y, grid_color.rgba());
            image.setPixel(x, y, grid_color.rgba());
        }
    }
}

static void renderTileIndices(QImage& image, int grid_size_x, int grid_size_y, QColor text_fg_color, QColor text_bg_color)
{
    if(grid_size_x < 0 || grid_size_y < 0)
        return;

    QFont font = QFont("Arial");
    font.setPixelSize(9);

    QPainter painter( &image);
    painter.setFont( font );
    painter.setPen(text_fg_color);

    int tiles_w =  image.width() / grid_size_x;
    int tiles_h =  image.height() / grid_size_y;
    int tile_count = tiles_w * tiles_h;

    QRect rect = QFontMetrics(font).boundingRect(QString::number(tile_count-1));

    for (int tx = 0; tx < tiles_w; tx++)
    {
        for (int ty = 0; ty < tiles_h; ty++)
        {
            int tile_index = ty * tiles_w + tx;
            QString text = QString::number(tile_index);

            int x = tx * grid_size_x;
            int y = ty * grid_size_y;
            rect.moveTo(x+1,y+1);

            painter.fillRect(rect, text_bg_color);
            painter.drawText(rect, Qt::AlignLeft, text );
        }
    }
}

static void renderCellHighlight(QImage& image, int grid_size_x, int grid_size_y, QColor highlight_color, int highlight_x, int highlight_y)
{
    if(highlight_x != -1 && highlight_y != -1)
    {
        int x1 = highlight_x*grid_size_x;
        int y1 = highlight_y*grid_size_y;
        int x2 = x1 + grid_size_x;
        int y2 = y1 + grid_size_y;

        if(x2 > image.width()-1)
        {
            --grid_size_x;
        }

        if(y2 > image.height()-1)
        {
            --grid_size_y;
        }

        for (int y = y1; y <= y2; y += grid_size_y)
        {
            for (int x = x1; x <= x2; x++)
            {
                image.setPixel(x, y, highlight_color.rgba());
            }
        }

        for (int x = x1; x <= x2; x += grid_size_x)
        {
            for (int y = y1; y <= y2; y++)
            {
                image.setPixel(x, y, highlight_color.rgba());
            }
        }
    }
}

TiledImageModel::TiledImageModel(QObject* parent)
    : QObject(parent)
    , image(nullptr)
    , view(nullptr)
{
}

void TiledImageModel::setTiledImage(TiledImage* new_image)
{
    if(this->image != new_image)
    {
        this->image = new_image;
        if(view)
        {
            view->redraw();
        }
    }
}

TiledImage* TiledImageModel::getTiledImage() const
{
    return image;
}

void TiledImageModel::render(QImage& out_image)
{
    if(image)
    {
        image->render(out_image);
    }
}

TiledImageView::TiledImageView(QWidget* parent):
    QGraphicsView(parent),
    model(nullptr),
    zoom(2),
    grid_color(QColor(255, 0, 0)),
    text_fg_color(QColor(225, 255, 225)),
    text_bg_color(QColor(0, 0, 0)),
    highlight_color(QColor(255,255,0)),
    grid_enabled(false),
    indices_enabled(false),
    grid_size_x(8),
    grid_size_y(8),
    control_key(false)
{
    QGraphicsScene* scene = new QGraphicsScene(this);
    scene->setBackgroundBrush(Qt::transparent);

    setScene(scene);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    clearCellHighlight();
}

void TiledImageView::setModel(TiledImageModel* model)
{
    this->model = model;
    this->model->view = this;
}

TiledImageModel* TiledImageView::getModel()
{
    return model;
}

void TiledImageView::clear()
{
    cached_pixmap.fill(Qt::transparent);
    scene()->clear();
}

void TiledImageView::draw(const QImage& image, int x, int y)
{
    QPainter painter(&cached_pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(x, y, image);
}

void TiledImageView::redraw()
{
    clear();

    if(model)
    {
        QImage image;
        model->render(image);

        const int width = image.width() * zoom;
        const int height = image.height() * zoom;

        if(width == 0 || height == 0)
            return;

        // reset the pixmap
        cached_pixmap = QPixmap(width, height);
        cached_pixmap.fill(Qt::transparent); // force alpha channel

        // draw image to pixmap
        image = image.scaled(width, height, Qt::IgnoreAspectRatio, Qt::FastTransformation);
        draw(image);

        // draw overlay
        int border_size = 2;
        QImage overlay = QImage(width / border_size, height / border_size, QImage::Format_ARGB32);
        overlay.fill(Qt::transparent);

        int grid_size_x_scale = grid_size_x * zoom / border_size;
        int grid_size_y_scale = grid_size_y * zoom / border_size;

        if (grid_enabled)
        {
            renderGrid(overlay, grid_size_x_scale, grid_size_y_scale, grid_color);
        }

        if(indices_enabled)
        {
            renderTileIndices(overlay, grid_size_x_scale, grid_size_y_scale, text_fg_color, text_bg_color);
        }

        renderCellHighlight(overlay, grid_size_x_scale, grid_size_y_scale, highlight_color, cell_highlight_x, cell_highlight_y);

        overlay = overlay.scaled(overlay.width() * border_size, overlay.height() * border_size, Qt::IgnoreAspectRatio, Qt::FastTransformation);
        draw(overlay);
    }

    scene()->addPixmap(cached_pixmap);
}

void TiledImageView::zoomBy(int delta)
{
    const float zoom_factor = 1.25;
    zoom += delta * zoom_factor;
    if(zoom < 1)
    {
        zoom = 1;
    }
    if(zoom > 5)
    {
        zoom = 5;
    }
    redraw();
}

void TiledImageView::getXY(QMouseEvent* event, int& x, int& y)
{
    int scroll_x = horizontalScrollBar()->value();
    int scroll_y = verticalScrollBar()->value();

    x = (event->x() + scroll_x) / zoom;
    y = (event->y() + scroll_y) / zoom;
}

void TiledImageView::getCellXY(QMouseEvent* event, int& cellx, int& celly)
{
    getXY(event, cellx, celly);
    cellx /= grid_size_x;
    celly /= grid_size_y;
}

int TiledImageView::getZoom()
{
    return zoom;
}

int TiledImageView::getGridSizeX()
{
    return grid_size_x;
}

int TiledImageView::getGridSizeY()
{
    return grid_size_y;
}

void TiledImageView::setGrid(QColor grid_color, int grid_size_x, int grid_size_y)
{
    this->grid_color = grid_color;
    this->grid_size_x = grid_size_x;
    this->grid_size_y = grid_size_y;
}

void TiledImageView::setGridEnabled(bool is_enabled)
{
    grid_enabled = is_enabled;
}

void TiledImageView::toggleGrid()
{
    grid_enabled = !grid_enabled;
}

void TiledImageView::setIndicesEnabled(bool is_enabled)
{
    indices_enabled = is_enabled;
}

void TiledImageView::toggleIndices()
{
    indices_enabled = !indices_enabled;
}

void TiledImageView::clearCellHighlight()
{
    cell_highlight_x = -1;
    cell_highlight_y = -1;
}

void TiledImageView::setCellHighlight(int cellx, int celly)
{
    cell_highlight_x = cellx;
    cell_highlight_y = celly;
}

void TiledImageView::keyPressEvent(QKeyEvent *event)
{
    if(event && event->key() == Qt::Key_Control)
    {
        control_key = true;
    }
    QGraphicsView::keyPressEvent(event);
}

void TiledImageView::keyReleaseEvent(QKeyEvent *event)
{
    if(event && event->key() == Qt::Key_Control)
    {
        control_key = false;
    }
    QGraphicsView::keyReleaseEvent(event);
}

void TiledImageView::wheelEvent(QWheelEvent *event)
{
    bool handled = false;
    if(control_key)
    {
        if(event->delta() > 0)
        {
            handled = true;
            zoomBy(1);
        }
        else if(event->delta() < 0)
        {
            handled = true;
            zoomBy(-1);
        }
    }
    if(!handled)
    {
        QGraphicsView::wheelEvent(event);
    }
}
