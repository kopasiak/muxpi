#ifndef KBDKEY_H
#define KBDKEY_H

#include <QKeyEvent>

class KbdKey
{
public:
    KbdKey();
    KbdKey(int key, bool isModifier=false);
    KbdKey(QKeyEvent *kevent);

    bool isValid()
    {
        return m_key >= 0;
    }

    unsigned char getKeyCode()
    {
        return m_key;
    }

    bool isModifier()
    {
        return m_isModifier;
    }

private:
    int m_key;
    bool m_isModifier;
};

#endif // KBDKEY_H
