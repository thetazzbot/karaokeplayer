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
    QScopedPointer<QIODevice> lyricFileDevice;
    QString lyricFile;

    QString filename = m_song.file;
    Settings::BackgroundType bgtype = pSettings->playerBackgroundType;

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
    if ( isVideoFile( filename) )
    {
        bgtype = Settings::BACKGROUND_TYPE_NONE;

        // Load the video file
        m_musicFileName = filename;

        if ( !m_player.loadVideo( m_musicFileName, false ) )
            throw QString( "Cannot load video file %1: %2") .arg( m_musicFileName ) .arg( m_player.errorMsg() );
    }
    else if ( filename.endsWith( ".zip", Qt::CaseInsensitive ) )
    {
        ArchiveZip ziparch( filename );

        if ( !ziparch.open() )
            throw QString( "Cannot open ZIP archive %1: %2") .arg( filename ) .arg( ziparch.errorMsg() );

        // Enumerate files in the archive to find music and lyrics
        Q_FOREACH( const QString& file, ziparch.enumerate() )
        {
            if ( isSupportedMusicFile( file ) && m_musicFileName.isEmpty() )
                m_musicFileName = file;

            // No else here - could be the same file (i.e. zipped MIDI)
            if ( isSupportedLyricFile( file ) && lyricFile.isEmpty() )
                lyricFile = file;
        }

        if ( lyricFile.isEmpty() || m_musicFileName.isEmpty() )
            throw QString( "ZIP archive doesn't contain both music and lyric files");

        Logger::debug( "KaraokeSong: ZIP archive contains music file %s and lyric file %s", qPrintable(m_musicFileName), qPrintable(lyricFile) );

        // This temporary file will store the extracted music file
        m_tempMusicFile = new QTemporaryFile("karaokeplayer");

        if ( !m_tempMusicFile->open() )
            throw QString( "Cannot open temporary file");

        // Extract the music into a temporary file
        if ( !ziparch.extract( m_musicFileName, m_tempMusicFile ) )
            throw QString( "Cannot extract a music file from an archive: %1") .arg( ziparch.errorMsg() );

        // this is our music file now! Close it and remember it.
        m_tempMusicFile->close();
        m_musicFileName = m_tempMusicFile->fileName();

        // Extract lyrics into QBuffer which we would then use to read the lyrics from
        lyricFileDevice.reset( new QBuffer() );
        lyricFileDevice->open( QIODevice::ReadWrite );

        // Extract the lyrics into our buffer
        if ( !ziparch.extract( lyricFile, lyricFileDevice.data() ) )
            throw QString( "Cannot extract a lyric file from an archive: %1") .arg( ziparch.errorMsg() );

        // Rewind the pointer so lyrics are read from the beginning
        lyricFileDevice->reset();
    }
    else if ( filename.endsWith( ".kfn", Qt::CaseInsensitive ) )
    {
        // Karafun file
    }
    else
    {
        // One file could be both music and lyrics (i.e. KAR)
        if ( isSupportedMusicFile( filename ) )
            m_musicFileName = filename;

        if ( isSupportedLyricFile( filename ) )
            lyricFile = filename;

        // Not a music nor lyric file?
        if ( m_musicFileName.isEmpty() && lyricFile.isEmpty() )
            return false;

        // Find the missing component if we miss anything
        if ( m_musicFileName.isEmpty() || lyricFile.isEmpty() )
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
                else if ( m_musicFileName.isEmpty() && isSupportedMusicFile( fi.absoluteFilePath() ) )
                {
                    m_musicFileName = fi.absoluteFilePath();
                    break;
                }
            }

            if ( lyricFile.isEmpty() || m_musicFileName.isEmpty() )
                throw QString( "Cannot find a matching music/lyric file");
        }

        // Lyrics are in a file
        lyricFileDevice.reset( new QFile( lyricFile ) );

        if ( !lyricFileDevice->open( QIODevice::ReadOnly ) )
            throw QString( "Cannot open lyrics file %1: %2") .arg( lyricFile ) .arg( lyricFileDevice->errorString() );

        Logger::debug( "KaraokeSong: matching music file %s with lyric file %s", qPrintable(m_musicFileName), qPrintable(lyricFile) );
    }

    // Open the music file as we need QIODevice
    if ( isMidiFile( m_musicFileName ) && pSettings->convertMidiFiles )
    {
        // If we have converted it, a cached file should be there
        QString test = Util::cachedFile( m_musicFileName );

        if ( !QFile::exists( test ) )
            throw QString("Cannot open music file %1 or cached file %2").arg( m_musicFileName ) .arg(test);

        m_musicFileName = test;
    }

    // Load the music file
    if ( !m_player.loadAudio( m_musicFileName ) )
        throw QString( "Cannot load music file %1: %2") .arg( m_musicFileName ) .arg( m_player.errorMsg() );

    // Read the lyrics if we have them. Note this not always the case (i.e. video files)
    if ( lyricFileDevice )
    {
        if ( lyricFile.endsWith( ".cdg", Qt::CaseInsensitive ) )
            m_lyrics = new PlayerLyricsCDG();
        else
            m_lyrics = new PlayerLyricsText( info.artist, info.title );

        if ( !m_lyrics->load( lyricFileDevice.data(), lyricFile ) )
            throw( QObject::tr("Can't load lyrics file %1: %2") .arg( lyricFile ) .arg( m_lyrics->errorMsg() ) );

        lyricFileDevice->close();

        // Set the lyric delay
        m_lyrics->setDelay( info.lyricDelay );
    }

    // Which background should we use?
    switch ( bgtype )
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

bool KaraokeSong::isMidiFile(const QString &filename)
{
    static const char * extlist[] = { ".kar", ".mid", ".midi", 0 };

    for ( int i = 0; extlist[i]; i++ )
        if ( filename.endsWith( extlist[i], Qt::CaseInsensitive ) )
            return true;

    return false;
}

bool KaraokeSong::isVideoFile(const QString &filename)
{
    static const char * extlist[] = { ".mp4", ".mov", ".avi", ".mkv", ".3gp", ".flv", 0 };

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
