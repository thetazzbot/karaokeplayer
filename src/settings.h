#ifndef SETTINGS_H
#define SETTINGS_H

#include <QFont>
#include <QColor>

class Settings
{
    public:
        Settings();

        enum BackgroundType
        {
            BACKGROUND_TYPE_COLOR,
            BACKGROUND_TYPE_IMAGE,
            BACKGROUND_TYPE_VIDEO
        };

        QString cacheDir;

        QColor  playerLyricsTextBeforeColor;
        QColor  playerLyricsTextAfterColor;
        QFont   playerLyricsFont;
        bool    playerLyricsTextEachCharacter;
        bool    convertMidiFiles;
        bool    playerCDGbackgroundTransparent;

        // Player background
        BackgroundType  playerBackgroundType;
        QColor          playerBackgroundColor;
        QStringList     playerBackgroundObjects;    // images or videos

        unsigned int    m_playerBackgroundLastObject; // last shown image, or last played video
        qint64          m_playerBackgroundLastVideoTime;  // for video only
};

extern Settings * pSettings;


#endif // SETTINGS_H
