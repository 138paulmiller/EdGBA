#include "newnamedialog.h"
#include <QLineEdit>

NewNameDialog::NewNameDialog(QWidget *parent, const QString& name)
    : QDialog(parent)
    , ui(new Ui_NewNameDialog)
{
    setWindowTitle("New Name");
    skip_sync = false;
    ok = false;

    ui->setupUi(this);
    ui->edit->setText(name);

    QObject::connect(ui->ok, SIGNAL(clicked(bool)), this, SLOT(on_ok(bool)));
}

NewNameDialog::~NewNameDialog()
{
    delete ui;
}

QString NewNameDialog::getName() const
{
    return ui->edit->text();
}


void NewNameDialog::setName(QString name)
{
    skip_sync = true;
    ui->edit->setText(name);
    skip_sync = false;
}

void NewNameDialog::on_ok(bool)
{
    ok = true;
    emit checkName(getName(), ok);

    if (ok)
    {
        close();
    }
}

void NewNameDialog::accept()
{
    if (ok)
    {
        QDialog::accept();
    }
}
