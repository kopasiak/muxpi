#include "connectiondialog.h"
#include "ui_connectiondialog.h"

ConnectionDialog::ConnectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionDialog), m_ok(false)
{
    ui->setupUi(this);
}

ConnectionDialog::~ConnectionDialog()
{
    delete ui;
}

QSsh::SshConnectionParameters ConnectionDialog::getConnectionParams()
{
    QSsh::SshConnectionParameters params;

    params.setHost(ui->hostInput->text());
    params.setPort(ui->portInput->text().toUShort());
    params.setUserName(ui->userInput->text());
    params.setPassword(ui->passInput->text());
    params.authenticationType = QSsh::SshConnectionParameters::AuthenticationTypePassword;
    params.timeout = 300;

    return params;
}
