#include "rmon.h"
#include "ui_rmon.h"

#include <QInputDialog>
#include <QStringList>
#include <QErrorMessage>
#include <QMessageBox>

#include <VLCQtCore/Common.h>
#include <VLCQtCore/Instance.h>
#include <VLCQtCore/Media.h>
#include <VLCQtCore/MediaPlayer.h>

#include "inputfilter.h"
#include "connectiondialog.h"
#include "sshremoteprocess.h"

RMon::RMon(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RMon),
    m_media(0),
    m_connection(NULL)
{
    ui->setupUi(this);

    m_instance = new VlcInstance(VlcCommon::args(), this);
    m_player = new VlcMediaPlayer(m_instance);
    m_player->setVideoWidget(ui->video);

    ui->video->setMediaPlayer(m_player);

    connect(ui->actionConnect, &QAction::triggered, this, &RMon::connectToMuxpi);

}

RMon::~RMon()
{
    delete m_player;
    delete m_media;
    delete m_instance;
    delete ui;
}

void RMon::connectToMuxpi()
{
    ConnectionDialog conDial;

    conDial.setModal(true);
    conDial.exec();

    if (conDial.result() != QDialog::Accepted)
        return;

    if (m_connection) {
        delete m_connection;
    }

    m_connection = new QSsh::SshConnection(conDial.getConnectionParams(), this);

    connect(m_connection, SIGNAL(connected()), SLOT(onMuxPiConnected()));
    connect(m_connection, SIGNAL(error(QSsh::SshError)), SLOT(onMuxPiConnectionError(QSsh::SshError)));

    m_connection->connectToHost();
}


void RMon::onMuxPiConnected()
{
    QMessageBox errorMessageDialog;

    errorMessageDialog.setText(QString("Connection to MuxPi succeded"));
    errorMessageDialog.exec();

    m_socatShell = m_connection->createRemoteShell();

    connect(m_socatShell.data(), SIGNAL(started()), SLOT(socatShellStarted()));
    connect(m_socatShell.data(), SIGNAL(readyReadStandardOutput()), SLOT(socatShellStdOutReady()));

    m_socatShell->start();

    m_hidShell = m_connection->createRemoteShell();

    connect(m_hidShell.data(), SIGNAL(started()), SLOT(hidShellStarted()));
    connect(m_hidShell.data(), SIGNAL(readyReadStandardOutput()), SLOT(hidShellStdOutReady()));

    m_hidShell->start();
}

void RMon::onMuxPiConnectionError(QSsh::SshError error)
{
    QMessageBox errorMessageDialog;

    errorMessageDialog.setText(QString("Unable to connect to MuxPi: ") + QString::number(error));
    errorMessageDialog.exec();
}


void RMon::socatShellStarted()
{
//    m_socatShell->write("ifconfig eth1 192.168.1.1 up\n");
    // No idea what for but this has to be executed to make dypers workig
    m_socatShell->write("stm -dut\n");
    m_socatShell->write("stm -ts\n");

    // Reset of our video grabber
    m_socatShell->write("stm -dyper1 off\n");
    m_socatShell->write("stm -dyper1 on\n");

    // TODO add this to menu for a user interaction
    m_socatShell->write("stm -dut\n");

    //TODO add execution of this on a current host:
    //iptables -t raw -A PREROUTING -p udp -m length --length 28 -j DROP

    //Using socat
 //   m_socatShell->write("socat  UDP-RECV:5004,ip-add-membership=239.255.42.42:192.168.1.1 UDP-SENDTO:192.168.0.1:5004 &\n");

    // Using smcroute and iptables
    m_socatShell->write("killall smcroute\n");
    m_socatShell->write("smcroute -d\n");
    m_socatShell->write("smcroute -a eth1 192.168.1.238 239.255.42.42 eth0\n");
    m_socatShell->write("iptables -t nat -A POSTROUTING -p udp -d 239.255.42.42 -j SNAT --to-source 192.168.0.13\n");
    m_socatShell->write("conntrack -F\n");

    //m_media = new VlcMedia("udp://@:5004", m_instance);
    m_media = new VlcMedia("udp://@239.255.42.42:5004", m_instance);
    m_media->setOption("network-caching=200");
    m_media->setOption(":clock-jitter=0");
    //m_media->setOption(":clock-synchro=0");
    m_player->open(m_media);
}

void RMon::hidShellStarted()
{
    m_hidShell->write("./usb_hid/setup.sh\n");

    ui->video->installEventFilter(new InputFilter(m_hidShell, m_player));
    ui->video->setAspectRatio(Vlc::R_16_9);
}

void RMon::socatShellStdOutReady()
{
    qDebug()<< m_socatShell->readAllStandardOutput();
}

void RMon::hidShellStdOutReady()
{
    qDebug()<< m_hidShell->readAllStandardOutput();
}

