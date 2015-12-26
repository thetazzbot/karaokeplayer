/**************************************************************************
 *  Ulduzsoft Karaoke PLayer - cross-platform desktop karaoke player      *
 *  Copyright (C) 2015-2016 George Yunaev, support@ulduzsoft.com          *
 *                                                                        *
 *  This program is free software: you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation, either version 3 of the License, or     *
 *  (at your option) any later version.                                   *
 *																	      *
 *  This program is distributed in the hope that it will be useful,       *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *  GNU General Public License for more details.                          *
 *                                                                        *
 *  You should have received a copy of the GNU General Public License     *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 **************************************************************************/

#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>

#include "settings.h"
#include "logger.h"

Settings * pSettings;


Settings::Settings()
{
    m_appDataPath = QStandardPaths::writableLocation( QStandardPaths::AppDataLocation );

    if ( m_appDataPath.isEmpty() )
        m_appDataPath = ".";

    if ( !m_appDataPath.endsWith( QDir::separator() ) )
        m_appDataPath += QDir::separator();

    songdbFilename = m_appDataPath + "karaoke.db";
    queueFilename = m_appDataPath + "queue.dat";

    // Create the application data dir if it doesn't exist
    if ( !QFile::exists( m_appDataPath ) )
    {
        if ( !QDir().mkpath( m_appDataPath ) )
            qCritical("Cannot create application data directory %s", qPrintable(m_appDataPath) );
    }

    // Create the cache data dir if it doesn't exist
    if ( !QFile::exists( QStandardPaths::writableLocation( QStandardPaths::CacheLocation ) ) )
    {
        if ( !QDir().mkpath( QStandardPaths::writableLocation( QStandardPaths::CacheLocation ) ) )
            qCritical("Cannot create application cache directory %s", qPrintable(QStandardPaths::writableLocation( QStandardPaths::AppDataLocation )) );
    }

    load();

    // Validate settings
    if ( playerBackgroundType == BACKGROUND_TYPE_IMAGE || playerBackgroundType == BACKGROUND_TYPE_VIDEO )
        loadBackgroundObjects();

    // songPathPrefix should follow directory separator
    if ( !songPathPrefix.isEmpty() && !songPathPrefix.endsWith( QDir::separator() ) )
        songPathPrefix.append( QDir::separator() );

    //customBackground = m_appDataPath + "background.jpg";
    //httpDocumentRoot = m_appDataPath + "wwwroot";

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

    convertMidiFiles = settings.value( "player/convertMidiFiles", false ).toBool();

    queueAddNewSingersNext = settings.value( "queue/AddNewSingersNext", false ).toBool();

    songPathPrefix = settings.value( "database/PathPrefix", "" ).toString();

    lircDevicePath = settings.value( "lirc/DevicePath", "" ).toString();
    lircMappingFile = settings.value( "lirc/MappingFile", "" ).toString();

    httpListenPort = settings.value( "http/ListenPort", 0 ).toInt();
    httpDocumentRoot = settings.value( "http/DocumentRoot", "" ).toString();

    customBackground = settings.value( "mainmenu/CustomBackground", "" ).toString();

    cacheDir = settings.value( "player/cacheDir", QStandardPaths::writableLocation( QStandardPaths::CacheLocation ) ).toString();

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

    settings.setValue( "player/convertMidiFiles", convertMidiFiles );
    settings.setValue( "player/CDGbackgroundTransparent", playerCDGbackgroundTransparent );
    settings.setValue( "player/MusicLyricDelay", playerMusicLyricDelay );

    settings.setValue( "queue/AddNewSingersNext", queueAddNewSingersNext );

    settings.setValue( "database/PathPrefix", songPathPrefix );

    settings.setValue( "lirc/DevicePath", lircDevicePath );
    settings.setValue( "lirc/MappingFile", lircMappingFile );

    settings.setValue( "http/ListenPort", httpListenPort );
    settings.setValue( "http/DocumentRoot", httpDocumentRoot );

    settings.setValue( "mainmenu/CustomBackground", customBackground );

    settings.setValue( "temp/playerBackgroundLastObject", m_playerBackgroundLastObject );
    settings.setValue( "temp/playerBackgroundLastVideoTime", m_playerBackgroundLastVideoTime );

    // Store the cache dir only if the location is changed (i.e. not standard)
    if ( QStandardPaths::writableLocation( QStandardPaths::CacheLocation ) != cacheDir )
        settings.setValue( "player/cacheDir", cacheDir );
}
