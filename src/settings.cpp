#include <QDir>
#include <QFileInfo>
#include <QSettings>

#include "settings.h"
#include "logger.h"

Settings * pSettings;


Settings::Settings()
{
    playerBackgroundType = BACKGROUND_TYPE_VIDEO;
    playerBackgroundObjects << "/home/tim/work/my/karaokeplayer/test/bgvideos";

//    playerBackgroundType = BACKGROUND_TYPE_IMAGE;
//    playerBackgroundObjects << "/home/tim/work/my/karaokeplayer/test/images";
//    playerBackgroundTransitionDelay = 30;

    playerRenderFPS = 25;
    playerBackgroundColor = QColor( Qt::black );
    playerLyricsFont = QFont("arial");
    playerLyricsTextBeforeColor = QColor( Qt::red );
    playerLyricsTextAfterColor = QColor( Qt::blue );
    playerLyricsTextEachCharacter = true;
    cacheDir = "/home/tim/work/my/karaokeplayer/test/cache";
    convertMidiFiles = true;
    playerCDGbackgroundTransparent = true;
    playerMusicLyricDelay = 300;

    queueAddNewSingersNext = false;
    queueFilename = "/home/tim/work/my/karaokeplayer/test/queue.dat";

    songdbFilename = "/home/tim/work/my/karaokeplayer/test/karaoke.db";

    lircDevicePath = "/dev/lircd";
    lircMappingFile = "/home/tim/work/my/karaokeplayer/test/lirc.map";

    httpListenPort = 8000;


    m_playerBackgroundLastObject = 0;
    m_playerBackgroundLastVideoTime = 0;
    m_playerRenderMSecPerFrame = 1000 / playerRenderFPS;

    if ( playerBackgroundType == BACKGROUND_TYPE_IMAGE || playerBackgroundType == BACKGROUND_TYPE_VIDEO )
        loadBackgroundObjects();
}

void Settings::save()
{
    QSettings settings;

    settings.setValue( "player/BackgroundType", playerBackgroundType );
    settings.setValue( "player/BackgroundObjects", playerBackgroundObjects );
    settings.setValue( "player/RenderFPS", playerRenderFPS );

    settings.setValue( "player/BackgroundColor", playerBackgroundColor );
    settings.setValue( "player/LyricsFont", playerLyricsFont );
    settings.setValue( "player/LyricsTextBeforeColor", playerLyricsTextBeforeColor );
    settings.setValue( "player/LyricsTextAfterColor", playerLyricsTextAfterColor );
    settings.setValue( "player/LyricsTextEachCharacter", playerLyricsTextEachCharacter );
    settings.setValue( "player/cacheDir", cacheDir );
    settings.setValue( "player/convertMidiFiles", convertMidiFiles );
    settings.setValue( "player/CDGbackgroundTransparent", playerCDGbackgroundTransparent );
    settings.setValue( "player/MusicLyricDelay", playerMusicLyricDelay );

    settings.setValue( "queueAddNewSingersNext", queueAddNewSingersNext );

    settings.setValue( "lircDevicePath", lircDevicePath );

    settings.setValue( "httpListenPort", httpListenPort );

    settings.setValue( "temp/playerBackgroundLastObject", m_playerBackgroundLastObject );
    settings.setValue( "temp/playerBackgroundLastVideoTime", m_playerBackgroundLastVideoTime );
}

void Settings::loadBackgroundObjects()
{
    m_playerBackgroundObjects.clear();

    QStringList extensions;

    if ( playerBackgroundType == BACKGROUND_TYPE_IMAGE )
        extensions << "*.jpg" << "*.png" << "*.gif" << "*.bmp";
    else
        extensions << "*.avi" << "*.mkv" << "*.mp4" << "*.3gp" << "*.mov";

    // Entries in playerBackgroundObjects could be files or directories
    for ( int i = 0; i < playerBackgroundObjects.size(); i++ )
    {
        QFileInfo finfo( playerBackgroundObjects[i] );

        if ( finfo.isDir() )
        {
            QFileInfoList list = QDir( playerBackgroundObjects[i] ).entryInfoList( extensions, QDir::Files | QDir::NoDotAndDotDot );

            foreach ( const QFileInfo& f, list )
                m_playerBackgroundObjects.push_back( f.absoluteFilePath() );
        }
        else if ( finfo.isFile() )
            m_playerBackgroundObjects.push_back( finfo.absoluteFilePath() );
    }

    Logger::debug( "Background: %d objects found", m_playerBackgroundObjects.size() );
}
