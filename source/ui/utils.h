#ifndef UTILS_H
#define UTILS_H

#include <QString>

class QAction;
class QPushButton;

class Utils
{
public:
    static void setupAction(QAction* action, QString icon);
    static void setupIconButton(QPushButton* button, QString icon);
    static void popupWarning(const char* message);

};

#endif // UTILS_H
