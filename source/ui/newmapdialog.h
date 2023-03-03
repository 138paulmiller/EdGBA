#ifndef NEWMAPDIALOG_H
#define NEWMAPDIALOG_H

#include "ui_newmapdialog.h"

#include <QDialog>
#include <QComboBox>

class NewMapDialog : public QDialog
{
    Q_OBJECT

private:
    QComboBox* combo;
    bool success;
    int option;
    bool affine;

public:
    NewMapDialog(QWidget *parent);
    bool getMapSelection(bool& out_affine, int& out_width, int& out_height);

public slots:
    void on_ok();
    void on_cancel();
    void on_regular();
    void on_affine();

private:
    Ui_NewMapDialog *ui;

};

#endif //NEWMAPDIALOG_H
