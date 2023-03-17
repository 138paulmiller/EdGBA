#ifndef CONFIGMENU_H
#define CONFIGMENU_H

#include <QWidget>
#include "ui_configmenu.h"

#include <QMap>
#include <QLineEdit>

class ConfigMenu : public QWidget
{
    Q_OBJECT

    QMap<QString, QLineEdit*> build_options;
public:
    explicit ConfigMenu(QWidget *parent = nullptr);
    ~ConfigMenu();
    void closeEvent(QCloseEvent* event) override;

public slots:

    void on_buildChange(QString);
    void on_resetBuildDefaults(bool);

private:
    Ui_ConfigMenu *ui;
};

#endif // CONFIGMENU_H
