#include <QDir>
#include <QFileInfo>
#include <QSettings>

#include "settings.h"
#include "logger.h"

Settings * pSettings;


Settings::Settings()
{
    songdbFilename = "/home/tim/work/my/karaokeplayer/test/karaoke.db";
    queueFilename = "/home/tim/work/my/karaokeplayer/test/queue.dat";

    load();

    // Validate settings
    m_playerRenderMSecPerFrame = 1000 / playerRenderFPS;

    if ( playerBackgroundType == BACKGROUND_TYPE_IMAGE || playerBackgroundType == BACKGROUND_TYPE_VIDEO )
        loadBackgroundObjects();

    if ( !songPathPrefix.isEmpty() && !songPathPrefix.endsWith( QDir::separator() ) )
        songPathPrefix.append( QDir::separator() );

    /*    playerBackgroundType = BACKGROUND_TYPE_VIDEO;
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

        ueueAddNewSingersNext = false;

        songPathPrefix = "/mnt/karaoke";

        lircDevicePath = "/dev/lircd";
        lircMappingFile = "/home/tim/work/my/karaokeplayer/test/lirc.map";

        httpListenPort = 8000;

        m_playerBackgroundLastObject = 0;
        m_playerBackgroundLastVideoTime = 0;

    */
}

void Settings::load()
{
    QSettings settings;

    playerBackgroundType = (BackgroundType) settings.value( "player/BackgroundType", (int)BACKGROUND_TYPE_COLOR ).toInt();
    playerBackgroundObjects = settings.value( "player/BackgroundObjects", QStringList() ).toStringList();
    playerRenderFPS = settings.value( "player/RenderFPS", 25 ).toInt();
    playerBackgroundColor = QColor( settings.value( "player/BackgroundColor", "black" ).toString() );
    playerLyricsFont = QFont( settings.value( "player/LyricsFont", "arial" ).toString() );
    playerLyricsTextBeforeColor = QColor( settings.value( "player/LyricsTextBeforeColor", "blue" ).toString() );
    playerLyricsTextAfterColor = QColor( settings.value( "player/LyricsTextAfterColor", "red" ).toString() );
    playerLyricsTextEachCharacter = settings.value( "player/LyricsTextEachCharacter", true ).toBool();
    playerCDGbackgroundTransparent = settings.value( "player/CDGbackgroundTransparent", false ).toBool();
    playerMusicLyricDelay = settings.value( "player/MusicLyricDelay", 0 ).toInt();

    cacheDir = settings.value( "player/cacheDir", "" ).toString();
    convertMidiFiles = settings.value( "player/convertMidiFiles", false ).toBool();

    queueAddNewSingersNext = settings.value( "queue/AddNewSingersNext", false ).toBool();

    songPathPrefix = settings.value( "database/PathPrefix", "" ).toString();

    lircDevicePath = settings.value( "lirc/DevicePath", "" ).toString();
    lircMappingFile = settings.value( "lirc/MappingFile", "" ).toString();

    httpListenPort = settings.value( "http/ListenPort", 0 ).toInt();

    m_playerBackgroundLastObject = settings.value( "temp/playerBackgroundLastObject", 0 ).toInt();
    m_playerBackgroundLastVideoTime = settings.value( "temp/playerBackgroundLastVideoTime", 0 ).toInt();
}

void Settings::save()
{
    QSettings settings;

    settings.setValue( "player/BackgroundType", playerBackgroundType );
    settings.setValue( "player/BackgroundObjects", playerBackgroundObjects );
    settings.setValue( "player/RenderFPS", playerRenderFPS );

    settings.setValue( "player/BackgroundColor", playerBackgroundColor.name() );
    settings.setValue( "player/LyricsFont", playerLyricsFont.family() );
    settings.setValue( "player/LyricsTextBeforeColor", playerLyricsTextBeforeColor.name() );
    settings.setValue( "player/LyricsTextAfterColor", playerLyricsTextAfterColor.name() );
    settings.setValue( "player/LyricsTextEachCharacter", playerLyricsTextEachCharacter );
    settings.setValue( "player/cacheDir", cacheDir );
    settings.setValue( "player/convertMidiFiles", convertMidiFiles );
    settings.setValue( "player/CDGbackgroundTransparent", playerCDGbackgroundTransparent );
    settings.setValue( "player/MusicLyricDelay", playerMusicLyricDelay );

    settings.setValue( "queue/AddNewSingersNext", queueAddNewSingersNext );

    settings.setValue( "database/PathPrefix", songPathPrefix );

    settings.setValue( "lirc/DevicePath", lircDevicePath );
    settings.setValue( "lirc/MappingFile", lircMappingFile );

    settings.setValue( "http/ListenPort", httpListenPort );

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
