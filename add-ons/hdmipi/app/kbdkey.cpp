#include "kbdkey.h"

#include <QVector>
#include <QMap>
#include <QChar>
#include <QString>

#include <iostream>

KbdKey::KbdKey()
    : m_key(-1), m_isModifier(false)
{

}

KbdKey::KbdKey(int key, bool isModifier)
    : m_key(key), m_isModifier(isModifier)
{

}

#define NSC_L_CTRL  37
#define NSC_L_SHIFT 50
#define NSC_L_ALT   64
#define NSC_L_GUI   133
#define NSC_R_CTRL  105
#define NSC_R_SHIFT 62
#define NSC_R_ALT   108
#define NSC_R_GUI   135

KbdKey::KbdKey(QKeyEvent *kevent)
    : m_key(-1), m_isModifier(false)
{
    static QString specDigitMarks = "!@#$%^&*()";
    static QVector<int> modifiersNSC{
            NSC_L_CTRL,
            NSC_L_SHIFT,
            NSC_L_ALT,
            NSC_L_GUI,
            NSC_R_CTRL,
            NSC_R_SHIFT,
            NSC_R_ALT,
            NSC_R_GUI
    };
    static QMap<int, int> otherKeys{
        {36, 0x28}, // Return
        {9, 0x29}, // ESC
        {22, 0x2A}, // Backspace
        {23, 0x2B}, // Tab
        {65, 0x2C}, // Spc

        {20, 0x2D}, // - _
        {21, 0x2E}, // = +
        {34, 0x2F}, // [ {
        {35, 0x30}, // ] }
        {51, 0x31}, // \ |
        {47, 0x33}, // ; :
        {48, 0x34}, // ' "
        {49, 0x35}, // ` ~
        {59, 0x36}, // , <
        {60, 0x37}, // . >
        {61, 0x38}, // / ?

        {66, 0x39}, // Caps
        {67, 0x3A}, // F1
        {68, 0x3B}, // F2
        {69, 0x3C}, // F3
        {70, 0x3D}, // F4
        {71, 0x3E}, // F5
        {72, 0x3F}, // F6
        {73, 0x40}, // F7
        {74, 0x41}, // F8
        {75, 0x42}, // F9
        {76, 0x43}, // F10
        {95, 0x44}, // F11
        {96, 0x45}, // F12
        //{, 0x46}, // PrintScreen TODO Howto catch this?
        {78, 0x47}, // Scroll Lock
        //{, 0x48}, // Pause
        {118, 0x49}, // Insert
        {110, 0x4A}, // Home
        {112, 0x4B}, // PgUp
        {119, 0x4C}, // Delete
        {115, 0x4D}, // End
        {117, 0x4E}, // PgDown

        {114, 0x4F}, // RArr
        {113, 0x50}, // LArr
        {116, 0x51}, // Down
        {111, 0x52}, // Up
        /* TODO add also keypad codes */
    };
    QChar mark = kevent->text()[0];

    /* http://www.freebsddiary.org/APC/usb_hid_usages.php */
    if (modifiersNSC.contains(kevent->nativeScanCode())) {
        m_isModifier = true;
         std::cout<<"scan code: "<<kevent->nativeScanCode()<<std::endl;
        m_key = 1 << modifiersNSC.indexOf(kevent->nativeScanCode());
    } else if (mark.isLetter()) {
        /* If ascii letter */
        m_key = mark.toLower().unicode() - 'a' + 0x04;
    } else if (mark.isDigit()) {
        /* If digit */
        int val = mark.digitValue();

        m_key = val == 0 ? 0x27 : val - 1 + 0x1E;
    } else if (specDigitMarks.contains(mark)) {
        /* One of special marks above digits */
        m_key = 0x1E + specDigitMarks.indexOf(mark);
    } else {
        /* Rest of keyboard mapping */
        m_key = otherKeys.value(kevent->nativeScanCode(), -1);
        if (m_key == -1)
            std::cout << kevent->nativeScanCode()<<std::endl;
    }
}

