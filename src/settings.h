#ifndef SETTINGS_H
#define SETTINGS_H

#include <QFont>
#include <QColor>

class Settings
{
    public:
        static Settings * g();

    public:
        QColor  playerBackgroundColor;
        QFont   playerLyricsFont;

    private:
        Settings();
};

#endif // SETTINGS_H
