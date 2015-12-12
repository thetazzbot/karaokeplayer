#ifndef SETTINGS_H
#define SETTINGS_H

#include <QFont>
#include <QColor>

class Settings
{
    public:
        static Settings * g();

    public:
        QString cacheDir;
        QColor  playerBackgroundColor;
        QColor  playerLyricsTextBeforeColor;
        QColor  playerLyricsTextAfterColor;
        QFont   playerLyricsFont;
        bool    playerLyricsTextEachCharacter;
        bool    convertMidiFiles;

    private:
        Settings();
};

#endif // SETTINGS_H
