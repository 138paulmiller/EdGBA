#include "spriteeditor.h"
#include "ui_spriteeditor.h"

#include <mainwindow.h>
#include <ui/utils.h>
#include <ui/misc/newnamedialog.h>
#include <defines.h>

#include <QFileDialog>
#include <QMessageBox>

SpriteEditor::SpriteEditor(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui_SpriteEditor)
{
    grid_color = QColor(255,0,0);
    frame_time_msec = 1000;
    skip_sync = true;
    preview_enabled = true;

    ui->setupUi(this);

    editname_dialog = nullptr;
}

SpriteEditor::~SpriteEditor()
{
    delete ui;
}

void SpriteEditor::setup(MainWindow* window)
{
    // Main window
    main_window = window;
    QObject::connect(window->ui->action_show_grid, SIGNAL(triggered()), this, SLOT(on_toggleGrid()));

    // TiledImage View/Models
    sprite_model = new SpriteModel(this);
    sprite_view = ui->sprite_view;
    sprite_view->setModel(sprite_model);
    sprite_view->setMouseHighlightEnabled(false);

    spritesheet_model = new SpriteSheetModel(this);
    spritesheet_view = ui->spritesheet_view;
    spritesheet_view->setModel(spritesheet_model);
    spritesheet_view->setMouseHighlightEnabled(false);

    // Spritesheet controls
    spritesheet_names_model = new QStringListModel(this);
    spritesheet_names_combo = ui->spritesheet_names;
    spritesheet_names_combo->setModel(spritesheet_names_model);
    QObject::connect(spritesheet_names_combo, SIGNAL(currentTextChanged(QString)), this, SLOT(on_spriteSheetSelectionChange(QString)));

    QObject::connect(ui->spritesheet_controls->add, SIGNAL(clicked(bool)), this, SLOT(on_spriteSheetAdd(bool)));
    QObject::connect(ui->spritesheet_controls->remove, SIGNAL(clicked(bool)), this, SLOT(on_spriteSheetRemove(bool)));
    QObject::connect(ui->spritesheet_controls->edit, SIGNAL(clicked(bool)), this, SLOT(on_spriteSheetEditName(bool)));
    QObject::connect(ui->spritesheet_controls->load, SIGNAL(clicked(bool)), this, SLOT(on_spriteSheetLoad(bool)));

    // Spritesheet sizes
    QStringListModel* sizes_model = new QStringListModel();
    sizes_model->setStringList(SpriteSheet::getSpriteSizeNames());
    ui->sprite_sizes_combo->setModel(sizes_model);
    QObject::connect(ui->sprite_sizes_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(on_spriteSheetSizeChanged(int)));

    // Preview toggle
    QObject::connect(ui->preview_checkbox, SIGNAL(stateChanged(int)), this, SLOT(on_previewAnimationChanged(int)));
    ui->preview_checkbox->setChecked(preview_enabled);

    // Sprite Anim controls
    QObject::connect(ui->anim_controls->add, SIGNAL(clicked(bool)), this, SLOT(on_spriteAnimAdd(bool)));
    QObject::connect(ui->anim_controls->remove, SIGNAL(clicked(bool)), this, SLOT(on_spriteAnimRemove(bool)));
    QObject::connect(ui->anim_controls->edit, SIGNAL(clicked(bool)), this, SLOT(on_spriteAnimEditName(bool)));
    ui->anim_controls->load->setVisible(false);

    // Anim Names
    spriteanim_names_model = new QStringListModel(this);
    spriteanim_names_combo = ui->anim_names;
    spriteanim_names_combo->setModel(spriteanim_names_model);
    QObject::connect(spriteanim_names_combo, SIGNAL(currentTextChanged(QString)), this, SLOT(on_spriteAnimSelectionChange(QString)));

    // Animation frames
    QObject::connect(ui->anim_frames_edit, SIGNAL(textEdited(QString)), this, SLOT(on_animationFramesChanged(QString)));
    QObject::connect(ui->anim_frames_edit, SIGNAL(editingFinished()), this, SLOT(on_animationFramesCommit()));

    // Frame speed
    QObject::connect(ui->anim_frame_speed_spinbox, SIGNAL(valueChanged(int)), this, SLOT(on_animationFrameSpeedChange(int)));

    // Flipped
    QObject::connect(ui->anim_hflip_checkbox, SIGNAL(toggled(bool)), this, SLOT(on_animationHFlipChange(bool)));
    QObject::connect(ui->anim_vflip_checkbox, SIGNAL(toggled(bool)), this, SLOT(on_animationVFlipChange(bool)));

}

void SpriteEditor::reset()
{
    skip_sync = false;
    sprite_model->setSpriteSheet(nullptr);
    spritesheet_model->setSpriteSheet(nullptr);
}

void SpriteEditor::reload()
{
    skip_sync = false;
    syncUI();
}

void SpriteEditor::syncUI()
{
    if(skip_sync) return;
    // prevent reentry, as some ui calls will route back to syncUI callback

    skip_sync = true;

    syncLabels();
    syncViews();

    skip_sync = false;
}

void SpriteEditor::syncLabels()
{
    // Grab previous data. Reseting the asset name models clear the values
    SpriteSheet* spritesheet = edit_context->getSpriteSheet();
    SpriteAnim* spriteanim = edit_context->getSpriteAnim();

    // Sync Sprite sheet combo
    QStringList spritesheet_names;
    edit_context->getSpriteSheetNames(spritesheet_names);
    spritesheet_names_model->setStringList(spritesheet_names);

    if(spritesheet)
    {
        spritesheet_names_combo->setCurrentText(spritesheet->getName());

        // Sync Sprite size combo
        QString sprite_size_label = SpriteSheet::getSpriteSizeName(spritesheet->getSpriteSize());
        ui->sprite_sizes_combo->setCurrentText(sprite_size_label);
    }

    // Sync Anim combo
    QStringList spriteanim_names;
    edit_context->getSpriteAnimNames(spriteanim_names);
    spriteanim_names_model->setStringList(spriteanim_names);

    if(spriteanim)
    {
        spriteanim_names_combo->setCurrentText(spriteanim->getName());

        // Sync Anim frames edit
        QString anim_frames_text = "";
        for(int i = 0; i < spriteanim->getFrameCount(); ++i)
        {
            int frame =  spriteanim->getFrame(i);
            anim_frames_text += QString::number(frame);
            if(i+1 < spriteanim->getFrameCount())
            {
                anim_frames_text += ",";
            }
        }
        ui->anim_frames_edit->setText(anim_frames_text);
    }

    // Frames input
    ui->anim_frames_edit->setEnabled(spriteanim != nullptr);

    // Frames duration
    int frame_duration = 0;
    if(spriteanim)
    {
        frame_duration = spriteanim->getFrameDuration();
    }
    ui->anim_frame_speed_spinbox->setValue(frame_duration);

    // Flipped check
    bool hflip = false;
    bool vflip = false;
    if(spriteanim)
    {
        hflip = spriteanim->getHFlip();
        vflip = spriteanim->getVFlip();
    }

    ui->anim_hflip_checkbox->setChecked(hflip);
    ui->anim_vflip_checkbox->setChecked(vflip);
}

void SpriteEditor::syncViews()
{
    sprite_view->invalidate();
    spritesheet_view->invalidate();

    SpriteSheet* spritesheet = edit_context->getSpriteSheet();
    //sprite_anim->setSpriteSheet(spritesheet);
    if(spritesheet)
    {
        spritesheet_view->setGrid(grid_color, spritesheet->getSpriteWidth(), spritesheet->getSpriteHeight());
    }

    // Sync models

    // Sprite view update
    SpriteAnim* spriteanim = edit_context->getSpriteAnim();
    if(spriteanim)
    {
        int frame_duration = spriteanim->getFrameDuration();
        if(preview_enabled)
            sprite_model->start(frame_duration);
        else
            sprite_model->stop();
    }

    sprite_model->setSpriteAnim(spriteanim);
    sprite_model->setSpriteSheet(spritesheet);
    sprite_view->redraw();

    // Sprite sheet update
    spritesheet_model->setSpriteSheet(spritesheet);
    spritesheet_view->redraw();
}

void SpriteEditor::undo()
{}

void SpriteEditor::redo()
{}

void SpriteEditor::zoomIn()
{
    ui->sprite_view->zoomBy(1);
    ui->spritesheet_view->zoomBy(1);
    ui->sprite_view->redraw();
    spritesheet_view->redraw();
}

void SpriteEditor::zoomOut()
{
    ui->sprite_view->zoomBy(-1);
    ui->spritesheet_view->zoomBy(-1);
    ui->sprite_view->redraw();
    spritesheet_view->redraw();
}

void SpriteEditor::on_toggleGrid()
{
    spritesheet_view->toggleGrid();
    spritesheet_view->toggleIndices();

    syncUI();
}

void SpriteEditor::on_spriteSheetAdd(bool /*enabled*/)
{
    edit_context->newSpriteSheet();
    syncUI();
    main_window->markDirty();
}

void SpriteEditor::on_spriteSheetRemove(bool /*enabled*/)
{
    edit_context->removeSpriteSheet();
    syncUI();
    main_window->markDirty();
}

void SpriteEditor::on_spriteSheetSelectionChange(QString name)
{
    if(skip_sync)
    {
        return; //currently being editted
    }
    //TODO: Gracefully handle null maps
    SpriteSheet* spritesheet = edit_context->findSpriteSheet(name);
    edit_context->setSpriteSheet(spritesheet);

    syncUI();
    main_window->markDirty();
}

void SpriteEditor::on_spriteSheetEditName(bool /*enabled*/)
{
    SpriteSheet* spritesheet = edit_context->getSpriteSheet();
    if(spritesheet == nullptr)
    {
        return;
    }

    editname_dialog = new NewNameDialog(main_window, spritesheet->getName());
    QObject::connect(editname_dialog, SIGNAL(checkName(QString,bool&)), this, SLOT(on_spriteSheetCheckName(QString,bool&)));

    editname_dialog->exec();
    if(editname_dialog->accepted())
    {
        QString name = editname_dialog->getName();
        spritesheet->setName(name);
        edit_context->setSpriteSheet(spritesheet);

        syncLabels();
        main_window->markDirty();
    }

    delete editname_dialog;
    editname_dialog = nullptr;
}

void SpriteEditor::on_spriteSheetCheckName(QString name, bool& ok)
{
    QStringList names;
    edit_context->getSpriteSheetNames(names);
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

void SpriteEditor::on_spriteSheetSizeChanged(int index)
{
    edit_context->setSpriteSheetSize(index);
    syncUI();
    main_window->markDirty();
}

void SpriteEditor::on_spriteSheetLoad(bool /*enabled*/)
{
    QString image_filename = QFileDialog::getOpenFileName(this, tr("Load SpriteSheet Image"), "", tr("Image Files (*.png)"));

    edit_context->newSpriteSheetFromImage(image_filename);

    syncUI();
    main_window->markDirty();
}

void SpriteEditor::on_spriteAnimAdd(bool /*enabled*/)
{
    edit_context->newSpriteAnim();
    syncUI();
    main_window->markDirty();
}

void SpriteEditor::on_spriteAnimRemove(bool /*enabled*/)
{
    edit_context->removeSpriteAnim();
    syncUI();
    main_window->markDirty();
}

void SpriteEditor::on_spriteAnimSelectionChange(QString name)
{
    if(skip_sync)
    {
        return; //currently being editted
    }

    SpriteAnim* spriteanim = edit_context->findSpriteAnim(name);
    edit_context->setSpriteAnim(spriteanim);

    syncUI();
    main_window->markDirty();
}

void SpriteEditor::on_spriteAnimEditName(bool /*enabled*/)
{
    SpriteAnim* spriteanim = edit_context->getSpriteAnim();
    if(spriteanim == nullptr)
    {
        return;
    }

    editname_dialog = new NewNameDialog(main_window, spriteanim->getName());
    QObject::connect(editname_dialog, SIGNAL(checkName(QString,bool&)), this, SLOT(on_spriteAnimCheckName(QString,bool&)));

    editname_dialog->exec();
    if(editname_dialog->accepted())
    {
        QString name = editname_dialog->getName();
        spriteanim->setName(name);
        edit_context->setSpriteAnim(spriteanim);

        syncLabels();
        main_window->markDirty();
    }

    delete editname_dialog;
    editname_dialog = nullptr;
}

void SpriteEditor::on_spriteAnimCheckName(QString name, bool& ok)
{
    QStringList names;
    edit_context->getSpriteAnimNames(names);
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

void SpriteEditor::on_previewAnimationChanged(int enabled)
{
    preview_enabled = enabled;
    syncUI();
}

void SpriteEditor::on_animationFramesChanged(QString anim_frames_text)
{
    QVector<int> frames;
    foreach(QString frame, anim_frames_text.split(","))
    {
        bool ok;
        int value = frame.toInt(&ok);
        if(ok)
        {
            frames.push_back(value);
        }
    }

    SpriteAnim* spriteanim = edit_context->getSpriteAnim();
    if(spriteanim)
    {
        spriteanim->setFrames(frames);
    }
}

void SpriteEditor::on_animationFramesCommit()
{
   syncUI();
}

void SpriteEditor::on_animationFrameSpeedChange(int value)
{
    SpriteAnim* spriteanim = edit_context->getSpriteAnim();
    if(spriteanim)
    {
        spriteanim->setFrameDuration(value);
    }

    syncUI();
}

void SpriteEditor::on_animationHFlipChange(bool value)
{
    SpriteAnim* spriteanim = edit_context->getSpriteAnim();
    if(spriteanim)
    {
        spriteanim->setHFlip(value);
    }

    syncUI();
}


void SpriteEditor::on_animationVFlipChange(bool value)
{
    SpriteAnim* spriteanim = edit_context->getSpriteAnim();
    if(spriteanim)
    {
        spriteanim->setVFlip(value);
    }

    syncUI();
}
