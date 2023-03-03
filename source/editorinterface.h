#ifndef EDITORINTERFACE_H
#define EDITORINTERFACE_H

class Game;
class MainWindow;

class EditorInterface
{
public:
    virtual ~EditorInterface() {}

    virtual void setup(MainWindow* window) = 0;
    virtual void reset() = 0;
    virtual void reload() = 0;
    virtual void undo() = 0;
    virtual void redo() = 0;
    virtual void zoomIn() = 0;
    virtual void zoomOut() = 0;

    virtual Game* game();
};

#endif // EDITORINTERFACE_H
