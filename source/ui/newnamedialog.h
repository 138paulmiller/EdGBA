#ifndef NEWNAMEDIALOG_H
#define NEWNAMEDIALOG_H

#include "ui_newnamedialog.h"
#include <QDialog>

class NewNameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewNameDialog(QWidget *parent, const QString& name);
    ~NewNameDialog();

    bool accepted() { return ok; }
    QString getName() const;
    void setName(QString name);

    void accept() override;

signals:
    void checkName(QString name, bool& ok);

public slots:
    void on_ok(bool);

private:
    Ui_NewNameDialog* ui;
    bool skip_sync;
    bool ok;
};


#endif // NEWNAMEDIALOG_H
