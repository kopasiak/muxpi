#include "inputfilter.h"

#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>

#include <QChar>
#include <QtEndian>
#include <QDateTime>

#include <VLCQtWidgets/WidgetVideo.h>

#include <iostream>

#include "kbdkey.h"
#include "sshremoteprocess.h"

InputFilter::InputFilter(QObject *parent) : QObject(parent), m_modifiers(0), m_touch(false)
{

}

InputFilter::InputFilter(QSharedPointer<QSsh::SshRemoteProcess> hidShell, VlcMediaPlayer *player, QObject *parent)
    : QObject(parent), m_hidShell(hidShell), m_player(player), m_modifiers(0), m_touch(false), m_lastMove(QDateTime::currentMSecsSinceEpoch())
{

}

bool InputFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *kevent = static_cast<QKeyEvent*>(event);

        return handleKeyPress(obj, kevent);
    } else if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *kevent = static_cast<QKeyEvent*>(event);

        return handleKeyRelease(obj, kevent);
    } else if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseMove || event->type() == event->MouseButtonRelease || event->type() == event->MouseButtonDblClick) {
        QMouseEvent *mevent = static_cast<QMouseEvent*>(event);

        return handleMouse(obj, mevent);
    }

    return QObject::eventFilter(obj, event);
}

bool InputFilter::handleKeyPress(QObject *obj, QKeyEvent *kevent)
{
    if (kevent->isAutoRepeat())
        return false;

    KbdKey k(kevent);

    if (k.isValid()) {
        if (k.isModifier()) {
            m_modifiers |= k.getKeyCode();

            std::cout << "Modifier pressed: "<<(int)k.getKeyCode()<<std::endl;
        } else {
            m_keys += k.getKeyCode();

            std::cout<<"Key pressed: "<<(int)k.getKeyCode()<<std::endl;
        }

        updateKbdReport();
    }
    return false;
}

bool InputFilter::handleKeyRelease(QObject *obj, QKeyEvent *kevent)
{
    if (kevent->isAutoRepeat())
        return false;

    KbdKey k(kevent);

    if (k.isValid()) {
        if (k.isModifier()) {
            m_modifiers &= ~k.getKeyCode();

            std::cout << "Modifier released: "<<(int)k.getKeyCode()<<std::endl;
        } else {
            m_keys -= k.getKeyCode();

            std::cout<<"Key released: "<<(int)k.getKeyCode()<<std::endl;
        }

        updateKbdReport();
    }
    return false;
}

void InputFilter::updateKbdReport()
{
    QString reportDesc;
    int count = 2;

    reportDesc = "\\x" + QString::number(m_modifiers, 16) + "\\x00";

    for (auto it = m_keys.begin(); it != m_keys.end(); ++it) {
        reportDesc += "\\x" + QString::number(*it, 16);
        ++count;
    }

    while (count < 8) {
        reportDesc += "\\x00";
        ++count;
    }

    m_hidShell->write(QString("echo -n -e '" + reportDesc +"' > /dev/hidg0\n").toLocal8Bit());
}

bool InputFilter::handleMouse(QObject *obj, QMouseEvent *mevent)
{
    //static const float frame_size_percent = 0.0416;
    // It's enought to switch of hdmi overrun on raspberry to eliminate this
    static const float frame_size_percent = 0;

    int video_h, video_w;
    float desired_prop, curr_prop;
    int widget_h, widget_w;
    int frame_h, frame_v;
    int x, y;
    int x_to_rep, y_to_rep;

    video_h = m_player->video()->size().height();
    video_w = m_player->video()->size().width();

    // Magic calculation because of that ugly black frame:(
    // frame is expected to be 5% of video height on each side
    desired_prop = (float)video_w/video_h;

    widget_h = ((QWidget*)obj)->height();
    widget_w = ((QWidget*)obj)->width();

    curr_prop = (float)widget_w/widget_h;

    if (desired_prop == curr_prop) {
        frame_h = frame_v = round((widget_h)*frame_size_percent);
    } else if (curr_prop < desired_prop) {
        // widget is higher than expected

        int height_with_frame = round(widget_w/desired_prop);

        frame_h = round(height_with_frame*frame_size_percent);
        frame_v = (widget_h - height_with_frame)/2 + frame_h;
    } else {
        //widget is wider then expected

        int width_with_frame = round(widget_h*desired_prop);

        frame_v = round(widget_h*frame_size_percent);
        frame_h = (widget_w - width_with_frame)/2 + frame_v;
    }

    if (mevent->x() < frame_h || mevent->x() > widget_w - frame_h
            || mevent->y() < frame_v || mevent->y() > widget_h - frame_v) {
        std::cout<<"Mouse event on frame. Ignoring..."<<std::endl;
        return false;
    }

    x = mevent->x() - frame_h;
    y = mevent->y() - frame_v;

    x_to_rep = x*10000/(widget_w - 2*frame_h);
    y_to_rep = y*10000/(widget_h - 2*frame_v);

    std::cout<<"event type: "<< mevent->type() << std::endl;
    if (mevent->type() == QEvent::MouseButtonPress || mevent->type() == QEvent::MouseButtonDblClick)
        m_touch = true;
    else if (mevent->type() == QEvent::MouseButtonRelease)
        m_touch = false;

    std::cout<<"Mouse event: x: "<<x_to_rep<<" y: "<<y_to_rep<<" touch: "<<m_touch<<std::endl;

    if (mevent->type() == QEvent::MouseMove) {
        qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
        std::cout << "current" << timestamp << " prev: "<< m_lastMove<<std::endl;
        if (timestamp - m_lastMove < 100)
            return false;
        else
            m_lastMove = timestamp;
    }

    updateTouchReport(x_to_rep, y_to_rep, m_touch);

    if (mevent->type() == QEvent::MouseButtonDblClick)
        std::cout<< "Double click"<<std::endl;

    return false;
}

void InputFilter::updateTouchReport(uint16_t x, uint16_t y, bool touch)
{
        struct touch_report {
            uint8_t contact_count;
            uint8_t contact_id;
            uint8_t touch;
            uint16_t x_pos;
            uint16_t y_pos;
        } __attribute((packed)) touch_report;

        uint8_t *trep = (uint8_t *)&touch_report;

        touch_report.contact_count = 1;
        touch_report.contact_id = 42;
        touch_report.touch = touch ? 3 : 2;
        // assume little endian
        touch_report.x_pos = x;
        touch_report.y_pos = y;

        QString cmd = "echo -n -e '";

        for (unsigned i = 0; i < sizeof(touch_report); ++i) {
            uint tmp = trep[i];
            cmd += "\\x" + QString::number(tmp, 16);
        }

        cmd += "' > /dev/hidg1\n";

        qDebug()<<cmd;
        m_hidShell->write(cmd.toLocal8Bit());
}
