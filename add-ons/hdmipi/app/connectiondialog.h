#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <QDialog>

#include "sshconnection.h"

namespace Ui {
class ConnectionDialog;
}

class ConnectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionDialog(QWidget *parent = 0);
    ~ConnectionDialog();

    QSsh::SshConnectionParameters getConnectionParams();

private:
    Ui::ConnectionDialog *ui;
    bool m_ok;
};

#endif // CONNECTIONDIALOG_H
