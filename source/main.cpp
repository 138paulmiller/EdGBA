/* main.cpp
 * main function for the tile editor GUI */

#include <stdio.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    /* load the main window ui from the one QT generates from the ui file */ 
    MainWindow* window = new MainWindow(&app);
    window->setWindowTitle("EdGBA");
    //window->setWindowFlags(Qt::Window | Qt::FramelessWindowHint );

    window->loadSession();
    window->show();

    return app.exec();
}

