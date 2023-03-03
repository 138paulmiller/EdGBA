#include "editorinterface.h"
#include "mainwindow.h"

Game* EditorInterface::game()
{
    QWidget* self = dynamic_cast<QWidget*>(this);
    if(self)
    {
        MainWindow* window = dynamic_cast<MainWindow*>(self->window());
        if(window)
        {
            return window->getGame();
        }
    }
    return nullptr;
}
