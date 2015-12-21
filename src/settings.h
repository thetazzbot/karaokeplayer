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

        // Player parameters
        QColor      playerLyricsTextBeforeColor;
        QColor      playerLyricsTextAfterColor;
        QFont       playerLyricsFont;
        bool        playerLyricsTextEachCharacter;
        bool        convertMidiFiles;
        bool        playerCDGbackgroundTransparent;

        // Queue parameters
        bool        queueAddNewSingersNext; // if true, new singers are added right after the current singer; otherwise at the end.
        QString     queueFilename;  // if defined, we save queue into file


        // Player background
        BackgroundType  playerBackgroundType;
        QColor          playerBackgroundColor;
        QStringList     playerBackgroundObjects;    // images or videos

        unsigned int    m_playerBackgroundLastObject; // last shown image, or last played video
        qint64          m_playerBackgroundLastVideoTime;  // for video only

        // Songs database
        QString         songdbFilename;
        QString         songPathPrefix;

        // LIRC path
        QString         lircDevicePath;
        QString         lircMappingFile;

        // HTTP port
        unsigned int    httpListenPort;
};

extern Settings * pSettings;


#endif // SETTINGS_H
