#include "mapeditor.h"

#include <defines.h>
#include <gba/gba.h>

#include "ui_mapeditor.h"

#include <mainwindow.h>
#include <ui/newmapdialog.h>
#include <ui/newnamedialog.h>
#include <ui/tiledimageeditdialog.h>

#include <QDialog>
#include <QMessageBox>
#include <QDebug>
#include <QFileInfo>
#include <QFileDialog>
#include <QLineEdit>
#include <QGraphicsView>

MapEditor::MapEditor(QWidget *parent)
    : QWidget(parent),
    ui(new Ui_MapEditor)
{
    ui->setupUi(this);

    skip_sync = 0;
    grid_color = QColor(255,0,0);
    editname_dialog = nullptr;
}

MapEditor::~MapEditor()
{
    delete ui;
}

void MapEditor::setup(MainWindow* window)
{
    main_window = window;

    // actions
    QObject::connect(window->ui->action_show_grid, SIGNAL(triggered()), this, SLOT(on_toggleGrid()));


    // set map ui
    map_model = new MapModel(this);
    ui->map_view->setModel(map_model);
    QObject::connect(ui->map_view, SIGNAL(clicked(int,int)), this, SLOT(on_mapTileClick(int,int)));

    map_names_model = new QStringListModel(this);
    ui->map_names->setModel(map_names_model);
    QObject::connect(ui->map_names, SIGNAL(currentTextChanged(QString)), this, SLOT(on_mapSelectionChange(QString)));


    map_modes_model = new QStringListModel(this);
    ui->map_modes->setModel(map_modes_model);
    QObject::connect(ui->map_modes, SIGNAL(currentTextChanged(QString)), this, SLOT(on_mapModeChange(QString)));

    QObject::connect(ui->map_controls->add, SIGNAL(clicked(bool)), this, SLOT(on_mapAdd(bool)));
    QObject::connect(ui->map_controls->remove, SIGNAL(clicked(bool)), this, SLOT(on_mapRemove(bool)));
    QObject::connect(ui->map_controls->edit, SIGNAL(clicked(bool)), this, SLOT(on_mapEditName(bool)));
    ui->map_controls->load->setVisible(false);

    // setup background tileset ui
    tileset_model = new TilesetModel(this);
    tileset_names_model = new QStringListModel(this);
    background_sizes_model = new QStringListModel(this);

    ui->tileset_view->setModel(tileset_model);
    ui->tileset_names->setModel(tileset_names_model);
    ui->background_sizes->setModel(background_sizes_model);

    QObject::connect(ui->tileset_view, SIGNAL(tileClicked(int,int)), this, SLOT(on_tilesetTileClick(int,int)));
    QObject::connect(ui->tileset_names, SIGNAL(currentTextChanged(QString)), this, SLOT(on_tilesetSelectionChange(QString)));
    QObject::connect(ui->tileset_controls->add, SIGNAL(clicked(bool)), this, SLOT(on_tilesetAdd(bool)));
    QObject::connect(ui->tileset_controls->remove, SIGNAL(clicked(bool)), this, SLOT(on_tilesetRemove(bool)));
    QObject::connect(ui->tileset_controls->edit, SIGNAL(clicked(bool)), this, SLOT(on_tilesetEditName(bool)));
    QObject::connect(ui->tileset_controls->load, SIGNAL(clicked(bool)), this, SLOT(on_tilesetLoad(bool)));
    QObject::connect(ui->tile_size, SIGNAL(valueChanged(int)), this, SLOT(on_tileResize(int)));
    QObject::connect(ui->background_priority, SIGNAL(valueChanged(int)), this, SLOT(on_backgroundPriorityChange(int)));
    QObject::connect(ui->background_sizes, SIGNAL(currentTextChanged(QString)), this, SLOT(on_backgroundResize(QString)));

    QObject::connect(ui->tile_hflip, SIGNAL(toggled(bool)), this, SLOT(on_tileHFlip(bool)));
    QObject::connect(ui->tile_vflip, SIGNAL(toggled(bool)), this, SLOT(on_tileVFlip(bool)));

    background_buttons[GBA_BG0] = ui->bg0_button;
    background_buttons[GBA_BG1] = ui->bg1_button;
    background_buttons[GBA_BG2] = ui->bg2_button;
    background_buttons[GBA_BG3] = ui->bg3_button;

    QObject::connect(ui->bg0_button, SIGNAL(clicked(bool)), this, SLOT(on_clickBackground0(bool)));
    QObject::connect(ui->bg1_button, SIGNAL(clicked(bool)), this, SLOT(on_clickBackground1(bool)));
    QObject::connect(ui->bg2_button, SIGNAL(clicked(bool)), this, SLOT(on_clickBackground2(bool)));
    QObject::connect(ui->bg3_button, SIGNAL(clicked(bool)), this, SLOT(on_clickBackground3(bool)));
}

void MapEditor::reset()
{
    skip_sync = 0;
    edit_context->reset();

    tileset_model->setTileset(nullptr);
    map_model->setMap(nullptr);
}

void MapEditor::reload()
{
    skip_sync = 0;
    syncUI();
}

void MapEditor::syncUI()
{
    if(edit_context->getMap() == nullptr)
    {
        // do not allow tab editting
        ui->map_tab->setEnabled(false);
        return;
    }

    skip_sync++;

    syncLabels();
    syncViews();

    skip_sync--;
}

void MapEditor::syncViews()
{
    ui->map_tab->setEnabled(true);

    // Sync models
    Map* map = edit_context->getMap();
    ui->map_view->setModel(map_model);
    map_model->setMap(map);

    Tileset* tileset = edit_context->getSelectedTileset();
    tileset_model->setTileset(tileset);

    // Redraw views
    refreshTilesetView();
    refreshMapView();
}

void MapEditor::syncLabels()
{
    // Must grab current tileset and map before change the stringlists for each model. This will fire a reselection event
    Map* map = edit_context->getMap();

    int mode = map->getMode();
    bool select_new_bg = false;
    for(int bg_index = 0; bg_index < GBA_BG_COUNT; ++bg_index)
    {
        bool enabled = Map::getBackgroundEnabled(mode, bg_index);
        if(!enabled && edit_context->getSelectedBackground() == bg_index)
        {
            select_new_bg = true;
        }
        background_buttons[bg_index]->setEnabled(enabled);
    }

    if(select_new_bg)
    {
        //find first enabled
        for(int new_bg_index = 0; new_bg_index < GBA_BG_COUNT; ++new_bg_index)
        {
            if(Map::getBackgroundEnabled(mode, new_bg_index))
            {
                edit_context->selectBackground(new_bg_index);
                break;
            }
        }
    }

    // update toggle button
    for(int bg_index = 0; bg_index < GBA_BG_COUNT; ++bg_index)
    {
        background_buttons[bg_index]->setChecked(edit_context->getSelectedBackground() == bg_index);
    }

    // Sync map mode combo
    QStringList map_modes = Map::getMapModeNames();
    map_modes_model->setStringList(map_modes);
    ui->map_modes->setCurrentText(Map::getMapModeName(mode));

    // Sync background combos
    QStringList tileset_names;
    edit_context->getTilesetNames(tileset_names);

    // Sync tileset names combos
    int bg_index = edit_context->getSelectedBackground();
    QString current_tileset_name = edit_context->getTilesetName(bg_index);
    QString background_size_label = edit_context->getBackgroundSizeName(bg_index);

    tileset_names_model->setStringList(tileset_names);
    ui->tileset_names->setCurrentText(current_tileset_name);

    // Sync background size combos
    QStringList background_sizes = Map::getBackgroundSizeNames(mode, bg_index);
    // changed mode
    if(!background_sizes.contains(background_size_label))
    {
        // if bg was affine, shift down
        if(Map::getBackgroundAffine(mode, bg_index))
        {
            map->resizeBackground(bg_index, map->getBackgroundSize(bg_index) + GBA_MAP_SIZE_32x32_AFFINE);
        }
        else // else shift up
        {
            map->resizeBackground(bg_index, map->getBackgroundSize(bg_index) - GBA_MAP_SIZE_16x16_AFFINE);
        }
        // select new bg_size
        background_sizes = Map::getBackgroundSizeNames(mode, bg_index);
        background_size_label = edit_context->getBackgroundSizeName(bg_index);
    }

    background_sizes_model->setStringList(background_sizes);
    ui->background_sizes->setCurrentText(background_size_label);

    // Sync map combos
    QStringList map_names;
    edit_context->getMapNames(map_names);
    map_names_model->setStringList(map_names);
    ui->map_names->setCurrentText(map->getName());

    // Sync tile size
    int tile_size = edit_context->getTileSelectionSize();
    ui->tile_size->setValue(tile_size);

    // Sync Priority
    int priority = map->getPriority(bg_index);
    ui->background_priority->setValue(priority);

    // Flips
    bool flip_enabled = !map->getAffine(bg_index);
    ui->tile_hflip->setEnabled(flip_enabled);
    ui->tile_vflip->setEnabled(flip_enabled);

    ui->tile_hflip->setChecked(edit_context->getSelectionHFlip());
    ui->tile_vflip->setChecked(edit_context->getSelectionVFlip());
}

void MapEditor::setTileSize(int tile_size)
{
    if(tile_size < GBA_TILE_SIZE)
    {
        tile_size = GBA_TILE_SIZE;
    }

    edit_context->setTileSelectionSize(tile_size);
}

void MapEditor::refreshTilesetView()
{
    int tile_size = edit_context->getTileSelectionSize();
    ui->tileset_view->setGrid(grid_color, tile_size, tile_size);

    int tilex, tiley;
    if(edit_context->getCornerSelectedTileXY(tilex, tiley))
    {
        int cellx = tilex / tile_size;
        int celly = tiley / tile_size;

        ui->tileset_view->setCellHighlight(cellx, celly);
    }

    ui->tileset_view->redraw();
}

void MapEditor::refreshMapView()
{
    int tile_size = edit_context->getTileSelectionSize();
    ui->map_view->setGrid(grid_color, tile_size, tile_size);
    ui->map_view->redraw();
}

void MapEditor::on_mapTileClick(int tilex, int tiley)
{
    edit_context->handleMapTileClick(tilex, tiley);

    int selection_size = edit_context->getSelectionSize();
    for(int dy = 0; dy < selection_size; ++dy)
    {
        for(int dx = 0; dx < selection_size; ++dx)
        {
            map_model->markTileDirty(tilex * selection_size + dx, tiley * selection_size + dy);
        }
    }

    ui->map_view->invalidate();
    main_window->markDirty();
}

void MapEditor::on_tilesetTileClick(int tilex, int tiley)
{
    edit_context->handleTilesetTileClick(tilex, tiley);
    ui->tileset_view->invalidateOverlay();
    refreshTilesetView();
}

void MapEditor::on_mapAdd(bool)
{
    editname_dialog = new NewNameDialog(main_window, Map().getDefaultName());
    QObject::connect(editname_dialog, SIGNAL(checkName(QString,bool&)), this, SLOT(on_mapCheckName(QString,bool&)));

    editname_dialog->exec();
    if(editname_dialog->accepted())
    {
        QString name = editname_dialog->getName();
        edit_context->newMap(name);
        main_window->markDirty();
        syncUI();
    }

    delete editname_dialog;
    editname_dialog = nullptr;
}

void MapEditor::on_mapRemove(bool)
{
    edit_context->removeMap();
    syncUI();
    main_window->markDirty();
}

void MapEditor::on_mapEditName(bool /*enabled*/)
{
    Map* map = edit_context->getMap();
    if(map == nullptr)
    {
        return;
    }

    editname_dialog = new NewNameDialog(main_window, map->getName());
    QObject::connect(editname_dialog, SIGNAL(checkName(QString,bool&)), this, SLOT(on_mapCheckName(QString,bool&)));

    editname_dialog->exec();
    if(editname_dialog->accepted())
    {
        QString name = editname_dialog->getName();
        map->setName(name);
        edit_context->setMap(map);

        syncLabels();
        main_window->markDirty();
    }

    delete editname_dialog;
    editname_dialog = nullptr;
}

void MapEditor::on_mapCheckName(QString name, bool& ok)
{
    QStringList names;
    edit_context->getMapNames(names);
    ok = !names.contains(name);

    QString base = name;
    int count = 1;
    while(names.contains(name))
    {
        name = base + QString::number(count);
        count++;
    }

    if(editname_dialog)
    {
        editname_dialog->setName(name);
    }
}

void MapEditor::on_mapSelectionChange(QString name)
{
    if(skip_sync) return;

    //TODO: Gracefully handle null maps
    Map* map = edit_context->findMap(name);
    edit_context->setMap(map);

    syncUI();
    main_window->markDirty();
}

void MapEditor::on_mapModeChange(QString name)
{
    if(skip_sync) return;

    //TODO: Gracefully handle null maps
    if(Map* map = edit_context->getMap())
    {
        map->setMode(Map::getMapMode(name));
    }
    syncUI();
    main_window->markDirty();
}

void MapEditor::on_toggleGrid()
{
    ui->tileset_view->toggleGrid();
    ui->map_view->toggleGrid();
    ui->tileset_view->invalidateOverlay();
    ui->map_view->invalidateOverlay();
    refreshMapView();
    refreshTilesetView();
}

void MapEditor::on_tileResize(int tile_size)
{
    setTileSize(tile_size);
    ui->tileset_view->invalidateOverlay();
    ui->map_view->invalidateOverlay();
    refreshTilesetView();
    refreshMapView();
}

void MapEditor::on_tileHFlip(bool is_flipped)
{
    edit_context->setSelectionHFlip(is_flipped);
    refreshTilesetView();
    refreshMapView();
}

void MapEditor::on_tileVFlip(bool is_flipped)
{
    edit_context->setSelectionVFlip(is_flipped);
    refreshTilesetView();
    refreshMapView();
}

void MapEditor::on_backgroundChange(int index)
{
    edit_context->selectBackground(index);
    syncUI();
}

void MapEditor::on_backgroundResize(QString value)
{
    if(skip_sync) return;

    int size_flag = Map::getBackgroundSizeFlag(value);
    if(size_flag != -1)
        edit_context->resizeSelectedBackground(size_flag);

    ui->map_view->invalidate();
    syncUI();
}

void MapEditor::on_backgroundPriorityChange(int priority)
{
    if(Map* map = edit_context->getMap())
    {
        int bg_index = edit_context->getSelectedBackground();
        map->setPriority(bg_index, priority);
    }

    ui->map_view->invalidate();
    // redraw
    syncUI();
}

void MapEditor::on_clickBackground0(bool /**/)
{
    edit_context->selectBackground(GBA_BG0);
    syncUI();
}

void MapEditor::on_clickBackground1(bool /**/)
{
    edit_context->selectBackground(GBA_BG1);
    syncUI();

}

void MapEditor::on_clickBackground2(bool /**/)
{
    edit_context->selectBackground(GBA_BG2);
    syncUI();

}

void MapEditor::on_clickBackground3(bool /**/)
{
    edit_context->selectBackground(GBA_BG3);
    syncUI();
}

void MapEditor::on_tilesetSelectionChange(QString name)
{
    if(skip_sync) return;

    //TODO: Gracefully handle null maps
    Tileset* tileset = edit_context->findTileset(name);
    edit_context->setSelectedTileset(tileset);

    ui->map_view->invalidate();

    syncUI();
    main_window->markDirty();
}

void MapEditor::on_tilesetEditName(bool /*enabled*/)
{
    Tileset* tileset = edit_context->getSelectedTileset();
    if(tileset == nullptr)
    {
        return;
    }

    // Open Tileset Editor
    editname_dialog = new NewNameDialog(main_window, tileset->getName());
    QObject::connect(editname_dialog, SIGNAL(checkName(QString,bool&)), this, SLOT(on_tilesetCheckName(QString,bool&)));

    editname_dialog->exec();
    if(editname_dialog->accepted())
    {
        QString name = editname_dialog->getName();
        tileset->setName(name);
        edit_context->setSelectedTileset(tileset);

        syncLabels();
        main_window->markDirty();
    }

    delete editname_dialog;
    editname_dialog = nullptr;
}

void MapEditor::on_tilesetCheckName(QString name, bool& ok)
{
    QStringList names;
    edit_context->getTilesetNames(names);
    ok = !names.contains(name);

    QString base = name;
    int count = 1;
    while(names.contains(name))
    {
        name = base + QString::number(count);
        count++;
    }

    if(editname_dialog)
    {
        editname_dialog->setName(name);
    }
}

void MapEditor::on_tilesetLoad(bool /*enabled*/)
{
    QString image_filename = QFileDialog::getOpenFileName(this, tr("Load Tileset Image"), "", tr("Image Files (*.png)"));

    edit_context->newTilesetFromImage(image_filename);

    ui->map_view->invalidateOverlay();
    syncUI();
    main_window->markDirty();
}

void MapEditor::on_tilesetAdd(bool)
{
    editname_dialog = new NewNameDialog(main_window, Tileset().getDefaultName());
    QObject::connect(editname_dialog, SIGNAL(checkName(QString,bool&)), this, SLOT(on_tilesetCheckName(QString,bool&)));

    editname_dialog->exec();
    if(editname_dialog->accepted())
    {
        QString name = editname_dialog->getName();
        edit_context->newTileset(name);
        main_window->markDirty();
        syncUI();
    }

    delete editname_dialog;
    editname_dialog = nullptr;
}

void MapEditor::on_tilesetRemove(bool)
{
    edit_context->removeSelectedTileset();

    ui->map_view->invalidateOverlay();
    syncUI();
    main_window->markDirty();
}

void MapEditor::undo()
{
    edit_context->undo();

    ui->map_view->invalidateOverlay();
    syncUI();
    main_window->markDirty();
}

void MapEditor::redo()
{
    edit_context->redo();
    ui->map_view->invalidateOverlay();
    syncUI();
    main_window->markDirty();
}

void MapEditor::zoomIn()
{
    ui->tileset_view->zoomBy(1);
    ui->map_view->zoomBy(1);

    refreshMapView();
    refreshTilesetView();
}

void MapEditor::zoomOut()
{
    ui->tileset_view->zoomBy(-1);
    ui->map_view->zoomBy(-1);
    refreshMapView();
    refreshTilesetView();
}
