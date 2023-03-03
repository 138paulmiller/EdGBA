#ifndef TILEDIMAGEVIEW_H
#define TILEDIMAGEVIEW_H

#include <gba/tiledimage.h>

#include <QGraphicsView>
#include <QThread>

struct TiledImageDrawTask
{
    class TiledImageView* view;
    QImage& image;
};

class TiledImageDrawThread : public QObject
{
    Q_OBJECT
signals:
    void finished();
};

class TiledImageModel : public QObject
{
    Q_OBJECT

protected:
    TiledImage* image;
    friend class TiledImageView;
    class TiledImageView* view;

public:
    TiledImageModel(QObject* parent);
    void setTiledImage(TiledImage* image);
    TiledImage* getTiledImage() const;

    virtual void render(QImage& out_image);
};

class TiledImageView : public QGraphicsView
{
    Q_OBJECT
private:
    TiledImageModel* model;
    float zoom;
    QPixmap cached_pixmap;
    QImage cached_image;

    QColor grid_color;
    QColor text_fg_color;
    QColor text_bg_color;
    QColor highlight_color;
    bool grid_enabled;
    bool indices_enabled;
    int grid_size_x, grid_size_y;
    int cell_highlight_x, cell_highlight_y;
    bool control_key;

public:
    TiledImageView(QWidget* parent);

    void setModel(TiledImageModel* model);
    TiledImageModel* getModel();

    void clear();
    void draw(const QImage& image, int x = 0, int y = 0);
    void redraw();

    void getXY(QMouseEvent* event, int& x, int& y);
    void getCellXY(QMouseEvent* event, int& cellx, int& celly);

    int getZoom();
    void zoomBy(int delta);

    int getGridSizeX();
    int getGridSizeY();
    void setGrid(QColor grid_color, int grid_size_x, int grid_size_y);
    void setGridEnabled(bool is_enabled);
    void toggleGrid();

    void setIndicesEnabled(bool is_enabled);
    void toggleIndices();

    void clearCellHighlight();
    void setCellHighlight(int cellx, int celly);

    // Begin QGraphicsView
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
    // End QGraphicsView
};

#endif // TILEDIMAGEVIEW_H
