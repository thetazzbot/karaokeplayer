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
#include <QBuffer>
#include <QCryptographicHash>

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
#include "archive_zip.h"
#include "util.h"
#include "karaokeplayable.h"

#include "libkaraokelyrics/lyricsloader.h"


KaraokeSong::KaraokeSong( KaraokeWidget *w, const SongQueue::Song &song )
{
    m_widget = w;
    m_song = song;

    m_lyrics = 0;
    m_background = 0;
    m_rating = 0;
    m_tempMusicFile = 0;

    m_nextRedrawTime = -1;
    m_lastRedrawTime = -1;

    connect( &m_player, SIGNAL( finished()), pActionHandler, SLOT(playerSongFinished() ) );

    connect( pActionHandler, SIGNAL( playerPauseResume()), this, SLOT(pause()) );
    connect( pActionHandler, SIGNAL( playerForward()), this, SLOT(seekForward()) );
    connect( pActionHandler, SIGNAL( playerBackward()), this, SLOT(seekBackward()) );
    connect( pActionHandler, SIGNAL( playerLyricsEarlier()), this, SLOT(lyricEarlier()) );
    connect( pActionHandler, SIGNAL( playerLyricsLater()), this, SLOT(lyricLater()) );
}

KaraokeSong::~KaraokeSong()
{
    m_player.stop();

    delete m_lyrics;
    delete m_background;
    delete m_tempMusicFile;
}

bool KaraokeSong::needsProcessing(const QString &filename)
{
    if ( KaraokePlayable::isMidiFile( filename ) && pSettings->convertMidiFiles )
    {
        // We only need processing if we don't have a cached copy of a processed file
        if ( !QFile::exists( Util::cachedFile( filename )) )
            return true;
    }

    return false;
}

bool KaraokeSong::open()
{
    // Reset just in case
    m_lyrics = 0;
    m_background = 0;

    // If we have song ID, query the DB first
    SongDatabaseInfo info;

    if ( m_song.songid != 0 && pSongDatabase->songById( m_song.songid, info ) )
        m_rating = info.rating;
    else
        m_rating = 0;

    // If this is a video file, there is no lyrics
    if ( KaraokePlayable::isVideoFile( m_song.file ) )
    {
        // Load the video file
        m_musicFileName = m_song.file;

        if ( !m_player.loadVideo( m_musicFileName, false ) )
            throw QString( "Cannot load video file %1: %2") .arg( m_musicFileName ) .arg( m_player.errorMsg() );

        // And we use empty background
        m_background = new PlayerBackgroundNone();
    }
    else
    {
        KaraokePlayable * karaoke = KaraokePlayable::create( m_song.file );

        if ( !karaoke->parse() )
            throw QString( "Cannot find a matching music/lyric file");

        // Get the files
        m_musicFileName = karaoke->musicObject();
        QString lyricFile = karaoke->lyricObject();

        Logger::debug( "KaraokeSong: found music file %s and lyric file %s", qPrintable(m_musicFileName), qPrintable(lyricFile) );

        // If this is MIDI file, try to find a cached file which must be there
        if ( KaraokePlayable::isMidiFile( m_musicFileName ) && pSettings->convertMidiFiles )
        {
            // If we have converted it, a cached file should be there
            QString test = Util::cachedFile( m_musicFileName );

            if ( !QFile::exists( test ) )
                throw QString("Cannot open music file %1 or cached file %2").arg( m_musicFileName ) .arg(test);

            m_musicFileName = test;
        }

        // If music file is not local, we need to extract it into the temp file
        if ( karaoke->isCompound() )
        {
            // This temporary file will store the extracted music file
            m_tempMusicFile = new QTemporaryFile("karaokeplayer");

            if ( !m_tempMusicFile->open() )
                throw QString( "Cannot open temporary file");

            // Extract the music into a temporary file
            if ( !karaoke->extract( m_musicFileName, m_tempMusicFile ) )
                throw QString( "Cannot extract a music file from an archive");

            // this is our music file now! Close it and remember it.
            m_tempMusicFile->close();
            m_musicFileName = m_tempMusicFile->fileName();
        }
        else
            m_musicFileName = karaoke->absolutePath( karaoke->musicObject() );

        // Load the music file
        if ( !m_player.loadAudio( m_musicFileName ) )
            throw QString( "Cannot load music file %1: %2") .arg( m_musicFileName ) .arg( m_player.errorMsg() );

        Logger::debug( "KaraokeSong: music file loaded" );

        // If lyrics are not in a local file, extract it into QBuffer first
        QScopedPointer< QIODevice > lyricDevice;

        if ( karaoke->isCompound() )
        {
            // Extract lyrics into QBuffer which we would then use to read the lyrics from
            QBuffer * buffer = new QBuffer();
            buffer->open( QIODevice::ReadWrite );

            // Extract the lyrics into our buffer
            if ( !karaoke->extract( lyricFile, buffer ) )
                throw QString( "Cannot extract a lyric file from an archive");

            buffer->reset();
            lyricDevice.reset( buffer );
        }
        else
        {
            // Lyrics are in a file
            lyricDevice.reset( new QFile( karaoke->absolutePath( karaoke->lyricObject()) ) );

            if ( !lyricDevice->open( QIODevice::ReadOnly ) )
                throw QString( "Cannot open lyrics file %1: %2") .arg( lyricFile ) .arg( lyricDevice->errorString() );
        }

        // Now we can load the lyrics
        if ( lyricFile.endsWith( ".cdg", Qt::CaseInsensitive ) )
            m_lyrics = new PlayerLyricsCDG();
        else
            m_lyrics = new PlayerLyricsText( info.artist, info.title );

        if ( !m_lyrics->load( lyricDevice.data(), lyricFile ) )
            throw( QObject::tr("Can't load lyrics file %1: %2") .arg( lyricFile ) .arg( m_lyrics->errorMsg() ) );

        // Set the lyric delay
        m_lyrics->setDelay( info.lyricDelay );

        Logger::debug( "KaraokeSong: lyrics loaded" );

        // Now check if we got custom background (Ultrastar or KFN)
        if ( !pSettings->playerIgnoreBackgroundFromFormats && m_lyrics->properties().contains( LyricsLoader::PROP_BACKGROUND ) )
        {
            QString filename = m_lyrics->properties() [ LyricsLoader::PROP_BACKGROUND ];

            // Same logic as with lyrics above
            if ( karaoke->isCompound() )
            {
                // Extract the data into QBuffer which we would then use to read the data from
                QBuffer buffer;
                buffer.open( QIODevice::ReadWrite );

                // Extract the lyrics into our buffer; failure is not a problem here
                if ( karaoke->extract( filename, &buffer ) )
                {
                    buffer.reset();

                    // Load the background from this buffer
                    m_background = new PlayerBackgroundImage();

                    if ( !m_background->initFromFile( &buffer ) )
                    {
                        delete m_background;
                        m_background = 0;
                    }
                }
            }
            else
            {
                // Just a regular file
                QFile bgfile( karaoke->absolutePath( filename ) );

                if ( bgfile.open( QIODevice::ReadOnly ) )
                {
                    // Load the background from this buffer
                    m_background = new PlayerBackgroundImage();

                    if ( !m_background->initFromFile( &bgfile ) )
                    {
                        delete m_background;
                        m_background = 0;
                    }
                }
            }

            if ( !m_background )
                Logger::debug( "Could not load background from file %s", qPrintable(filename) );
        }
    }

    // If not set already, which background should we use?
    if ( !m_background )
    {
        switch ( pSettings->playerBackgroundType )
        {
            case Settings::BACKGROUND_TYPE_NONE:
                m_background = new PlayerBackgroundNone();
                break;

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
    }

    // Set the song state
    pCurrentState->playerSong = m_song;

    return true;
}

void KaraokeSong::start()
{
    pCurrentState->playerDuration = m_player.duration();
    pCurrentState->playerState = CurrentState::PLAYERSTATE_PLAYING;

    if ( m_background )
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
        m_player.pauseOrResume( true );
        m_background->pause( true );
    }
    else
    {
        pCurrentState->playerState = CurrentState::PLAYERSTATE_PLAYING;
        m_player.pauseOrResume( false );
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
    qint64 time = pCurrentState->playerDuration;

    if ( m_background )
        time = qMin( m_background->draw( p ), time );

    if ( m_lyrics )
    {
        m_lyrics->draw( p );
    }
    else
    {
        m_player.draw( p );
        time = 0;
    }

    return time;
}

void KaraokeSong::stop()
{
    pCurrentState->playerState = CurrentState::PLAYERSTATE_STOPPED;

    m_background->stop();
    m_player.stop();

    pSongQueue->statusChanged( m_song.id, false );

    if ( m_song.songid )
        pSongDatabase->updatePlayedSong( m_song.songid, m_lyrics ? m_lyrics->delay() : 0, m_rating );
}

void KaraokeSong::lyricEarlier()
{
    if ( !m_lyrics )
        return;

    int newdelay = m_lyrics->delay() + 200;
    m_lyrics->setDelay( newdelay );

    pNotification->setMessage( tr("Lyrics show %1 by %2ms") .arg( newdelay < 0 ? "earlier" : "later") .arg(newdelay));
}

void KaraokeSong::lyricLater()
{
    if ( !m_lyrics )
        return;

    int newdelay = m_lyrics->delay() - 200;
    m_lyrics->setDelay( newdelay );

    pNotification->setMessage( tr("Lyrics show %1 by %2ms") .arg( newdelay < 0 ? "earlier" : "later") .arg(newdelay));
}
