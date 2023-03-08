#ifndef SPRITEEDITOR_H
#define SPRITEEDITOR_H

#include "spriteview.h"
#include "spritesheetview.h"

#include <QWidget>
#include <QComboBox>
#include <QStringListModel>

#include <gba/spritesheet.h>
#include <ui/tiledimageeditdialog.h>
#include <editorinterface.h>

class NewNameDialog;

class SpriteEditor : public QWidget, public EditorInterface
{
    Q_OBJECT

private:
    int frame_time_msec;
    QColor grid_color;
    bool preview_enabled;

    bool skip_sync;

    SpriteModel* sprite_model;
    SpriteView* sprite_view;
    QComboBox* sprite_names_combo;
    QStringListModel* sprite_names_model;

    SpriteSheetModel* spritesheet_model;
    SpriteSheetView* spritesheet_view;
    QComboBox* spritesheet_names_combo;
    QStringListModel* spritesheet_names_model;

    QComboBox* spriteanim_names_combo;
    QStringListModel* spriteanim_names_model;

    NewNameDialog* editname_dialog;

public:
    SpriteEditor(QWidget *parent = nullptr);
    ~SpriteEditor();

    // Begin EditorInterface
    void setup(MainWindow* window) override;
    void reset() override;
    void reload() override;
    void undo() override;
    void redo() override;
    void zoomIn() override;
    void zoomOut() override;
    // End EditorInterface
private:
    void syncUI();
    void syncLabels();
    void syncViews();

protected slots:
    void on_toggleGrid();

    void on_spriteSheetAdd(bool enabled);
    void on_spriteSheetRemove(bool enabled);
    void on_spriteSheetSelectionChange(QString name);
    void on_spriteSheetEditName(bool /*enabled*/);
    void on_spriteSheetCheckName(QString name, bool& ok);
    void on_spriteSheetSizeChanged(int index);
    void on_spriteSheetLoad(bool enabled);

    void on_spriteAnimAdd(bool enabled);
    void on_spriteAnimRemove(bool enabled);
    void on_spriteAnimSelectionChange(QString name);
    void on_spriteAnimEditName(bool /*enabled*/);
    void on_spriteAnimCheckName(QString name, bool& ok);

    void on_previewAnimationChanged(int enabled);
    void on_animationFramesChanged(QString text);
    void on_animationFramesCommit();
    void on_animationFrameSpeedChange(int value);
    void on_animationHFlipChange(bool value);
    void on_animationVFlipChange(bool value);

private:
    class Ui_SpriteEditor* ui;
    class MainWindow* main_window;
};

#endif // SPRITEEDITOR_H
