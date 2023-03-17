#include "tiledimageview.h"
#include <QMouseEvent>
#include <QScrollBar>
#include <QGraphicsPixmapItem>
#include <QPainter>

#define GRID_LINE_WIDTH 1

static void renderGrid(QImage& image, int grid_size_x, int grid_size_y, QColor grid_color)
{
    if(grid_size_x < 0 || grid_size_y < 0)
        return;

    QPainter painter( &image);
    painter.setPen(QPen(grid_color, GRID_LINE_WIDTH));

    QVector<QLine> lines;
    lines.reserve(image.width() / grid_size_x + image.height() / grid_size_y);

    for (int y = 0; y < image.height(); y += grid_size_y)
    {
        QLine line = QLine(0, y, image.width(), y);
        lines.push_back(line);
    }

    for (int x = 0; x < image.width(); x += grid_size_x)
    {
        QLine line = QLine(x, 0, x, image.height());
        lines.push_back(line);
    }
    painter.drawLines(lines);
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
        QPainter painter( &image);
        painter.setPen(QPen(highlight_color, GRID_LINE_WIDTH));
        painter.drawRect(highlight_x * grid_size_x, highlight_y * grid_size_y, grid_size_x, grid_size_y);
        return;

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

TileImageViewDrawProperties::TileImageViewDrawProperties():
    zoom(2),
    grid_color(QColor(128, 32, 32, 128)),
    text_fg_color(QColor(225, 255, 225)),
    text_bg_color(QColor(0, 0, 0)),
    highlight_color(QColor(255,255,0)),
    grid_enabled(false),
    indices_enabled(false),
    grid_size_x(8),
    grid_size_y(8),
    mouse_cell_highlight(true),
    mouse_cell_x(0),
    mouse_cell_y(0)
{}

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
            view->invalidate();
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
    control_key(false)
{    
    source_width = 0;
    source_height = 0;

    QGraphicsScene* scene = new QGraphicsScene(this);
    scene->setBackgroundBrush(Qt::transparent);

    setScene(scene);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);

    setMouseTracking(true);
    if(viewport())
        viewport()->setMouseTracking(true);

    clearCellHighlight();

    invalidate();
    invalidateOverlay();
}

TiledImageView::~TiledImageView()
{
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
    cached_pixmap = QPixmap(0, 0);
    scene()->clear();
}

void TiledImageView::invalidate()
{
    cached_image = QImage(0, 0, QImage::Format_ARGB32);
}

void TiledImageView::invalidateOverlay()
{
    cached_overlay = QImage(0, 0, QImage::Format_ARGB32);
}

void TiledImageView::redraw()
{
    if(model == nullptr)
    {
        clear();
        return;
    }

    cached_pixmap = QPixmap(0, 0);   // force realloc
    if(cached_image.width() == 0 || cached_image.height() == 0)
    {
        render();
    }

    const int scaled_width = source_width * properties.zoom;
    const int scaled_height = source_height * properties.zoom;
    if(scaled_width != cached_image.width() && scaled_height != cached_image.height())
    {
        cached_image = cached_image.scaled(scaled_width, scaled_height, Qt::IgnoreAspectRatio, Qt::FastTransformation);
        invalidateOverlay();
    }

    const int target_pixmap_width = cached_image.width() + GRID_LINE_WIDTH;
    const int target_pixmap_height = cached_image.height() + GRID_LINE_WIDTH;
    if(cached_pixmap.width() != target_pixmap_width || cached_pixmap.height() != target_pixmap_height)
    {
        cached_pixmap = QPixmap(target_pixmap_width, target_pixmap_height);
        cached_pixmap.fill(Qt::transparent); // force alpha channel
    }

    draw(cached_image, GRID_LINE_WIDTH, GRID_LINE_WIDTH);

    if(cached_overlay.width() != cached_image.width() && cached_overlay.height() != cached_image.width())
    {
        renderOverlay();
    }

    draw(cached_overlay);

    scene()->clear();
    scene()->addPixmap(cached_pixmap);
}

void TiledImageView::zoomBy(int delta)
{
    const float zoom_factor = 1.05;
    properties.zoom += delta * zoom_factor;
    if(properties.zoom < 1)
    {
        properties.zoom = 1;
    }
    if(properties.zoom > 5)
    {
        properties.zoom = 5;
    }

    redraw();
}

void TiledImageView::getXY(QMouseEvent* event, int& x, int& y)
{
    int scroll_x = horizontalScrollBar()->value();
    int scroll_y = verticalScrollBar()->value();

    x = (event->x() + scroll_x) / properties.zoom - GRID_LINE_WIDTH;
    y = (event->y() + scroll_y) / properties.zoom - GRID_LINE_WIDTH;
}

void TiledImageView::getCellXY(QMouseEvent* event, int& cellx, int& celly)
{
    getXY(event, cellx, celly);
    cellx /= properties.grid_size_x;
    celly /= properties.grid_size_y;
}

int TiledImageView::getZoom()
{
    return properties.zoom;
}

int TiledImageView::getGridSizeX()
{
    return properties.grid_size_x;
}

int TiledImageView::getGridSizeY()
{
    return properties.grid_size_y;
}

void TiledImageView::setGrid(QColor grid_color, int grid_size_x, int grid_size_y)
{
    properties.grid_color = grid_color;
    properties.grid_size_x = grid_size_x;
    properties.grid_size_y = grid_size_y;
}

void TiledImageView::setGridEnabled(bool is_enabled)
{
    properties.grid_enabled = is_enabled;
}

void TiledImageView::setMouseHighlightEnabled(bool is_enabled)
{
    properties.mouse_cell_highlight = is_enabled;
}

void TiledImageView::toggleGrid()
{
    properties.grid_enabled = !properties.grid_enabled;
}

void TiledImageView::setIndicesEnabled(bool is_enabled)
{
    properties.indices_enabled = is_enabled;
}

void TiledImageView::toggleIndices()
{
    properties.indices_enabled = !properties.indices_enabled;
}

void TiledImageView::clearCellHighlight()
{
    properties.cell_highlight_x = -1;
    properties.cell_highlight_y = -1;
    invalidateOverlay();
}

void TiledImageView::setCellHighlight(int cellx, int celly)
{
    properties.cell_highlight_x = cellx;
    properties.cell_highlight_y = celly;
    invalidateOverlay();
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

void TiledImageView::mouseMoveEvent( QMouseEvent *event )
{
    TiledImageView::getCellXY(event, properties.mouse_cell_x,properties.mouse_cell_y);
    if(properties.mouse_cell_highlight)
    {
        invalidateOverlay();
        redraw();
    }
    QWidget::mouseMoveEvent( event );
}

void TiledImageView::draw(const QImage& image, int x, int y)
{
    QPainter painter(&cached_pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(x, y, image);
}

void TiledImageView::render()
{
    model->render(cached_image);

    source_width = cached_image.width();
    source_height = cached_image.height();
}

void TiledImageView::renderOverlay()
{
    const int width = cached_image.width() + GRID_LINE_WIDTH;
    const int height = cached_image.height() + GRID_LINE_WIDTH;

    // draw overlay
    cached_overlay = QImage(width, height, QImage::Format_ARGB32);
    cached_overlay.fill(Qt::transparent);

    int grid_size_x_scale = properties.grid_size_x * properties.zoom;
    int grid_size_y_scale = properties.grid_size_y * properties.zoom;

    if (properties.grid_enabled)
        renderGrid(cached_overlay, grid_size_x_scale, grid_size_y_scale, properties.grid_color);

    if(properties.indices_enabled)
        renderTileIndices(cached_overlay, grid_size_x_scale, grid_size_y_scale, properties.text_fg_color, properties.text_bg_color);

    renderCellHighlight(cached_overlay, grid_size_x_scale, grid_size_y_scale, properties.highlight_color, properties.cell_highlight_x, properties.cell_highlight_y);

    if(properties.mouse_cell_highlight)
        renderCellHighlight(cached_overlay, grid_size_x_scale, grid_size_y_scale, properties.highlight_color, properties.mouse_cell_x, properties.mouse_cell_y);

    cached_overlay = cached_overlay.scaled(cached_overlay.width(), cached_overlay.height(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

