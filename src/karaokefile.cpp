#include <QDir>
#include <QCryptographicHash>

#include "player.h"
#include "settings.h"
#include "karaokefile.h"
#include "playerwidget.h"
#include "playerlyricscdg.h"
#include "playerlyricstext.h"
#include "playerbackgroundcolor.h"
#include "playerbackgroundvideo.h"
#include "playerbackgroundimage.h"
#include "karaokepainter.h"
#include "eventcontroller.h"
#include "songdatabase.h"
#include "playernotification.h"
#include "util.h"

#include "libkaraokelyrics/lyricsloader.h"


KaraokeFile::KaraokeFile( PlayerWidget *w, int id )
{
    m_widget = w;
    m_songID = id;

    m_lyrics = 0;
    m_background = 0;
    m_playState = STATE_RESET;

    m_nextRedrawTime = -1;
    m_lastRedrawTime = -1;

    connect( &m_player, SIGNAL( finished()), pController, SLOT(playerSongFinished() ) );
    connect( pController, SIGNAL( playerPauseResume()), this, SLOT(pause()) );
    connect( pController, SIGNAL( playerForward()), this, SLOT(seekForward()) );
    connect( pController, SIGNAL( playerBackward()), this, SLOT(seekBackward()) );
    connect( pController, SIGNAL(playerLyricsEarlier()), this, SLOT(lyricEarlier()) );
    connect( pController, SIGNAL(playerLyricsLater()), this, SLOT(lyricLater()) );
}

KaraokeFile::~KaraokeFile()
{
    m_player.stop();

    delete m_lyrics;
    delete m_background;
}

bool KaraokeFile::needsProcessing(const QString &filename)
{
    if ( isMidiFile( filename ) && pSettings->convertMidiFiles )
    {
        // We only need processing if we don't have a cached copy of a processed file
        if ( !QFile::exists( Util::cachedFile( filename )) )
            return true;
    }

    return false;
}

bool KaraokeFile::open(const QString &filename)
{
    QString lyricFile;
    QIODevice * lyricFileRead = 0;

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

        qDebug( "Found music file %s, lyric file %s", qPrintable(musicFile), qPrintable(lyricFile) );

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

    if ( !m_lyrics->load( *lyricFileRead, lyricFile ) )
        throw( QObject::tr("Can't load lyrics file %1: %2") .arg( lyricFile ) .arg( m_lyrics->errorMsg() ) );

    // If we have song ID, query the DB if it has a delay for those lyrics
    if ( m_songID != 0 )
        m_lyrics->setDelay( pSongDatabase->getLyricDelay( m_songID ) );

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
    m_playState = STATE_READY;

    return true;
}

void KaraokeFile::start()
{
    // If no conversion is in progress, start the player and rendering thread
    m_playState = STATE_PLAYING;
    m_background->start();
    m_player.play();
    pNotification->clearOnScreenMessage();
}

void KaraokeFile::pause()
{
    if ( m_playState == STATE_PLAYING )
    {
        m_playState = STATE_PAUSED;
        m_player.pause();
        m_background->pause( true );
    }
    else
    {
        m_playState = STATE_PLAYING;
        m_player.play();
        m_background->pause( false );
    }
}

void KaraokeFile::seekForward()
{
    seekTo( qMin( duration() - 10000, position() + 10000 ) );
}

void KaraokeFile::seekBackward()
{
    seekTo( qMax( position() - 10000, 0LL ) );
}

void KaraokeFile::seekTo(qint64 timing)
{
    m_player.seekTo( timing );
}

qint64 KaraokeFile::duration()
{
    return m_player.duration();
}

qint64 KaraokeFile::position()
{
    return m_player.position();
}

qint64 KaraokeFile::draw(KaraokePainter &p)
{
    // Background is always on
    qint64 bgtime = m_background->draw( p );
    qint64 lyrictime = m_lyrics->draw( p );

    return qMin( bgtime, lyrictime );
}

void KaraokeFile::stop()
{
    m_player.stop();

    if ( m_songID )
        pSongDatabase->updatePlayedSong( m_songID, m_lyrics->delay() );
}

void KaraokeFile::lyricEarlier()
{
    int newdelay = m_lyrics->delay() + 200;
    m_lyrics->setDelay( newdelay );

    pNotification->setMessage( tr("Lyrics show %1 by %2ms") .arg( newdelay < 0 ? "earlier" : "later") .arg(newdelay));
}

void KaraokeFile::lyricLater()
{
    int newdelay = m_lyrics->delay() - 200;
    m_lyrics->setDelay( newdelay );

    pNotification->setMessage( tr("Lyrics show %1 by %2ms") .arg( newdelay < 0 ? "earlier" : "later") .arg(newdelay));
}

bool KaraokeFile::isMidiFile(const QString &filename)
{
    static const char * extlist[] = { ".kar", ".mid", ".midi", 0 };

    for ( int i = 0; extlist[i]; i++ )
        if ( filename.endsWith( extlist[i], Qt::CaseInsensitive ) )
            return true;

    return false;
}

bool KaraokeFile::isSupportedMusicFile(const QString &filename)
{
    static const char * extlist[] = { ".mp3", ".wav", ".ogg", ".aac", ".flac", ".kar", ".mid", ".midi", 0 };

    for ( int i = 0; extlist[i]; i++ )
        if ( filename.endsWith( extlist[i], Qt::CaseInsensitive ) )
            return true;

    return false;
}

bool KaraokeFile::isSupportedLyricFile(const QString &filename)
{
    return LyricsLoader::isSupportedFile( filename ) || filename.endsWith( ".cdg", Qt::CaseInsensitive );
}
