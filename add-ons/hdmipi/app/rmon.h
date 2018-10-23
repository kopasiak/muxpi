#ifndef RMON_H
#define RMON_H

#include <QMainWindow>

#include "sshconnection.h"

namespace Ui {
class RMon;
}

class VlcInstance;
class VlcMedia;
class VlcMediaPlayer;

class RMon : public QMainWindow
{
    Q_OBJECT

public:
    explicit RMon(QWidget *parent = 0);
    ~RMon();

private slots:
    void connectToMuxpi();
    void onMuxPiConnected();
    void onMuxPiConnectionError(QSsh::SshError error);
    void socatShellStarted();
    void hidShellStarted();
    void socatShellStdOutReady();
    void hidShellStdOutReady();

private:
    Ui::RMon *ui;

    VlcInstance *m_instance;
    VlcMedia *m_media;
    VlcMediaPlayer *m_player;
    QSsh::SshConnection *m_connection;
    QSharedPointer<QSsh::SshRemoteProcess> m_socatShell;
    QSharedPointer<QSsh::SshRemoteProcess> m_hidShell;
};

#endif // RMON_H
