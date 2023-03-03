#ifndef AssetControls_H
#define AssetControls_H

#include <QPushButton>
#include <QComboBox>

class AssetControls : public QWidget
{
    Q_OBJECT

private:
    class Ui_AssetControls* ui;

public:
    QPushButton* add;
    QPushButton* remove;
    QPushButton* edit;
    QPushButton* load;

public:
    AssetControls(QWidget *parent = nullptr);
    ~AssetControls();
};

#endif // AssetControls_H
