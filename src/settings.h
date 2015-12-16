#ifndef SETTINGS_H
#define SETTINGS_H

#include <QFont>
#include <QColor>

class Settings
{
    public:
        Settings();

        QString cacheDir;
        QColor  playerBackgroundColor;
        QColor  playerLyricsTextBeforeColor;
        QColor  playerLyricsTextAfterColor;
        QFont   playerLyricsFont;
        bool    playerLyricsTextEachCharacter;
        bool    convertMidiFiles;
        bool    playerCDGbackgroundTransparent;
};

extern Settings * pSettings;


#endif // SETTINGS_H
