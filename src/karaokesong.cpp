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
#include <QCryptographicHash>

#include "player.h"
#include "logger.h"
#include "settings.h"
#include "karaokesong.h"
#include "karaokewidget.h"
#include "playerlyricscdg.h"
#include "playerlyricstext.h"
#include "playerbackgroundcolor.h"
#include "playerbackgroundvideo.h"
#include "playerbackgroundimage.h"
#include "karaokepainter.h"
#include "actionhandler.h"
#include "songdatabase.h"
#include "playernotification.h"
#include "currentstate.h"
#include "util.h"

#include "libkaraokelyrics/lyricsloader.h"


KaraokeSong::KaraokeSong( KaraokeWidget *w, const SongQueue::Song &song )
{
    m_widget = w;
    m_song = song;

    m_lyrics = 0;
    m_background = 0;

    m_nextRedrawTime = -1;
    m_lastRedrawTime = -1;

    connect( &m_player, SIGNAL( finished()), pActionHandler, SLOT(playerSongFinished() ) );

    connect( pActionHandler, SIGNAL( playerPauseResume()), this, SLOT(pause()) );
    connect( pActionHandler, SIGNAL( playerForward()), this, SLOT(seekForward()) );
    connect( pActionHandler, SIGNAL( playerBackward()), this, SLOT(seekBackward()) );
    connect( pActionHandler, SIGNAL(playerLyricsEarlier()), this, SLOT(lyricEarlier()) );
    connect( pActionHandler, SIGNAL(playerLyricsLater()), this, SLOT(lyricLater()) );
}

KaraokeSong::~KaraokeSong()
{
    m_player.stop();

    delete m_lyrics;
    delete m_background;
}

bool KaraokeSong::needsProcessing(const QString &filename)
{
    if ( isMidiFile( filename ) && pSettings->convertMidiFiles )
    {
        // We only need processing if we don't have a cached copy of a processed file
        if ( !QFile::exists( Util::cachedFile( filename )) )
            return true;
    }

    return false;
}

bool KaraokeSong::open()
{
    QIODevice * lyricFileRead = 0;
    QString lyricFile;
    QString filename = m_song.file;

    if ( filename.endsWith( ".zip", Qt::CaseInsensitive ) )
    {
        // ZIP archive

    }
    else if ( filename.endsWith( ".kfn", Qt::CaseInsensitive ) )
    {
        // Karafun file
    }
    else
    {
        QString musicFile;

        // One file could be both music and lyrics (i.e. KAR)
        if ( isSupportedMusicFile( filename ) )
            musicFile = filename;

        if ( isSupportedLyricFile( filename ) )
            lyricFile = filename;

        // Not a music nor lyric file?
        if ( musicFile.isEmpty() && lyricFile.isEmpty() )
            return false;

        // Find the missing component if we miss anything
        if ( musicFile.isEmpty() || lyricFile.isEmpty() )
        {
            QFileInfo finfo( filename );

            // Enumerate all the files in the file's directory with the same name and .* extension
            QFileInfoList list = finfo.dir().entryInfoList( QStringList() << (finfo.baseName() + ".*"), QDir::Files | QDir::NoDotAndDotDot );

            foreach ( QFileInfo fi, list )
            {
                // Skip current file
                if ( fi == finfo )
                    continue;

                if ( lyricFile.isEmpty() && isSupportedLyricFile( fi.absoluteFilePath() ) )
                {
                    lyricFile = fi.absoluteFilePath();
                    break;
                }
                else if ( musicFile.isEmpty() && isSupportedMusicFile( fi.absoluteFilePath() ) )
                {
                    musicFile = fi.absoluteFilePath();
                    break;
                }
            }

            if ( lyricFile.isEmpty() || musicFile.isEmpty() )
                return false;
        }

        Logger::debug( "KaraokeSong: matching music file %s with lyric file %s", qPrintable(musicFile), qPrintable(lyricFile) );

        // Open the music file as we need QIODevice
        if ( isMidiFile( musicFile ) && pSettings->convertMidiFiles )
        {
            // If we have converted it, a cached file should be there
            QString test = Util::cachedFile( filename );

            if ( !QFile::exists( test ) )
                throw QString("Cannot open music file %1 or cached file %2").arg( musicFile ) .arg(test);

            musicFile = test;
        }

        // Load the music file
        m_musicFileName = musicFile;
        m_player.load( m_musicFileName );

        // Read the lyrics
        QFile * lfile = new QFile( lyricFile );

        if ( !lfile->open( QIODevice::ReadOnly ) )
            throw QString( "Cannot open lyrics file %1: %2") .arg( lyricFile ) .arg(lfile->errorString());

        lyricFileRead = lfile;
    }

    if ( lyricFile.endsWith( ".cdg", Qt::CaseInsensitive ) )
        m_lyrics = new PlayerLyricsCDG();
    else
        m_lyrics = new PlayerLyricsText();

    if ( !m_lyrics->load( lyricFileRead, lyricFile ) )
        throw( QObject::tr("Can't load lyrics file %1: %2") .arg( lyricFile ) .arg( m_lyrics->errorMsg() ) );

    lyricFileRead->close();
    delete lyricFileRead;

    // If we have song ID, query the DB if it has a delay for those lyrics
    if ( m_song.songid != 0 )
        m_lyrics->setDelay( pSongDatabase->getLyricDelay( m_song.songid ) );

    // Which background should we use?
    switch ( pSettings->playerBackgroundType )
    {
        case Settings::BACKGROUND_TYPE_IMAGE:
            m_background = new PlayerBackgroundImage();
            break;

        case Settings::BACKGROUND_TYPE_VIDEO:
            m_background = new PlayerBackgroundVideo();
            break;

        case Settings::BACKGROUND_TYPE_COLOR:
            m_background = new PlayerBackgroundColor();
            break;
    }

    // Initialize
    m_background->initFromSettings();

    // Set the song state
    pCurrentState->playerSong = m_song;

    return true;
}

void KaraokeSong::start()
{
    pCurrentState->playerDuration = m_player.duration();
    pCurrentState->playerState = CurrentState::PLAYERSTATE_PLAYING;

    m_background->start();
    m_player.play();

    pSongQueue->statusChanged( m_song.id, true );
    pNotification->clearOnScreenMessage();
}

void KaraokeSong::pause()
{
    if ( pCurrentState->playerState == CurrentState::PLAYERSTATE_PLAYING )
    {
        pCurrentState->playerState = CurrentState::PLAYERSTATE_PAUSED;
        m_player.pause();
        m_background->pause( true );
    }
    else
    {
        pCurrentState->playerState = CurrentState::PLAYERSTATE_PLAYING;
        m_player.play();
        m_background->pause( false );
    }
}

void KaraokeSong::seekForward()
{
    seekTo( qMin( m_player.duration() - 10000, m_player.position() + 10000 ) );
}

void KaraokeSong::seekBackward()
{
    seekTo( qMax( m_player.position() - 10000, 0LL ) );
}

void KaraokeSong::seekTo(qint64 timing)
{
    m_player.seekTo( timing );
}

qint64 KaraokeSong::draw(KaraokePainter &p)
{
    // Update position
    pCurrentState->playerPosition = m_player.position();

    // Background is always on
    qint64 bgtime = m_background->draw( p );
    qint64 lyrictime = m_lyrics->draw( p );

    return qMin( bgtime, lyrictime );
}

void KaraokeSong::stop()
{
    pCurrentState->playerState = CurrentState::PLAYERSTATE_STOPPED;

    m_player.stop();

    pSongQueue->statusChanged( m_song.id, false );

    if ( m_song.songid )
        pSongDatabase->updatePlayedSong( m_song.songid, m_lyrics->delay() );
}

void KaraokeSong::lyricEarlier()
{
    int newdelay = m_lyrics->delay() + 200;
    m_lyrics->setDelay( newdelay );

    pNotification->setMessage( tr("Lyrics show %1 by %2ms") .arg( newdelay < 0 ? "earlier" : "later") .arg(newdelay));
}

void KaraokeSong::lyricLater()
{
    int newdelay = m_lyrics->delay() - 200;
    m_lyrics->setDelay( newdelay );

    pNotification->setMessage( tr("Lyrics show %1 by %2ms") .arg( newdelay < 0 ? "earlier" : "later") .arg(newdelay));
}

bool KaraokeSong::isMidiFile(const QString &filename)
{
    static const char * extlist[] = { ".kar", ".mid", ".midi", 0 };

    for ( int i = 0; extlist[i]; i++ )
        if ( filename.endsWith( extlist[i], Qt::CaseInsensitive ) )
            return true;

    return false;
}

bool KaraokeSong::isSupportedMusicFile(const QString &filename)
{
    static const char * extlist[] = { ".mp3", ".wav", ".ogg", ".aac", ".flac", ".kar", ".mid", ".midi", 0 };

    for ( int i = 0; extlist[i]; i++ )
        if ( filename.endsWith( extlist[i], Qt::CaseInsensitive ) )
            return true;

    return false;
}

bool KaraokeSong::isSupportedLyricFile(const QString &filename)
{
    return LyricsLoader::isSupportedFile( filename ) || filename.endsWith( ".cdg", Qt::CaseInsensitive );
}
