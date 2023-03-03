#include "utils.h"
#include "defines.h"

#include <QAction>
#include <QStyle>
#include <QPushButton>
#include <QMessageBox>

void Utils::setupAction(QAction* action, QString icon)
{
    action->setIcon(QIcon(icon));
}

void Utils::setupIconButton(QPushButton* button, QString icon)
{
    button->setProperty("type", "icon");
    button->setIcon(QIcon(icon));
    button->setText("");
    button->setIconSize(QSize(24,24));
    button->setMaximumSize(QSize(24,24));

    button->style()->unpolish(button);
    button->style()->polish(button);
}

void Utils::popupWarning(const char* message)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(EDGBA_TITLE" Warning");
    msgBox.setText(message);
    msgBox.exec();
}
