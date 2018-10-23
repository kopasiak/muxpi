#ifndef INPUTFILTER_H
#define INPUTFILTER_H

#include <QObject>
#include <QKeyEvent>
#include <QMouseEvent>

#include <VLCQtCore/MediaPlayer.h>
#include <VLCQtCore/Video.h>

#include "sshconnection.h"

class InputFilter : public QObject
{
    Q_OBJECT
public:
    explicit InputFilter(QObject *parent = 0);
    explicit InputFilter(QSharedPointer<QSsh::SshRemoteProcess> hidShell, VlcMediaPlayer *player, QObject *parent = 0);

signals:

public slots:

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    bool handleKeyPress(QObject *obj, QKeyEvent *kevent);
    bool handleKeyRelease(QObject *obj, QKeyEvent *kevent);
    bool handleMouse(QObject *obj, QMouseEvent *mevent);

    void updateKbdReport();
    void updateTouchReport(uint16_t x, uint16_t y, bool touch);

    QSharedPointer<QSsh::SshRemoteProcess> m_hidShell;
    VlcMediaPlayer *m_player;

    unsigned char m_modifiers;
    QSet<unsigned char> m_keys;

    bool m_touch;
    qint64 m_lastMove;
};

#endif // INPUTFILTER_H
