#include <QDir>
#include <QTime>
#include <QCryptographicHash>

#include "player.h"
#include "settings.h"
#include "karaokefile.h"
#include "playerwidget.h"
#include "playerlyricscdg.h"
#include "playerlyricstext.h"
#include "playerbackgroundcolor.h"
#include "playerbackgroundvideo.h"
#include "karaokepainter.h"

#include "libkaraokelyrics/lyricsloader.h"


//KaraokeFile::KaraokeFile(AudioPlayer_FFmpeg *plr, PlayerWidget *w)
KaraokeFile::KaraokeFile(Player *plr, PlayerWidget *w)
{
    m_widget = w;
    m_player = plr;

    m_musicFile = 0;
    m_lyrics = 0;
    m_background = 0;
    m_convProcess = 0;

    m_nextRedrawTime = -1;
    m_lastRedrawTime = -1;
}

KaraokeFile::~KaraokeFile()
{
    delete m_musicFile;
    delete m_lyrics;
    delete m_background;
    delete m_convProcess;
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
        if ( isMidiFile( musicFile ) && Settings::g()->convertMidiFiles )
        {
            // Do we have it cached?
            QFileInfo finfo( musicFile );
            QString test = finfo.absolutePath() + QDir::separator() + finfo.completeBaseName() + ".wav";

            if ( QFile::exists( test ) )
                musicFile = test;
            else
            {
                QCryptographicHash hash( QCryptographicHash::Sha3_512 );
                QFile file( musicFile );

                if ( !file.open( QIODevice::ReadOnly ) )
                {
                    QString err = file.errorString();
                    throw QString("Cannot open music file %1: %2").arg( musicFile ) .arg(err);
                }

                hash.addData( &file );
                test = Settings::g()->cacheDir + QDir::separator() + hash.result().toHex() + ".wav";

                if (  QFile::exists( test ) )
                    musicFile = test;
                else
                {
                    // Start conversion right away
                    m_musicFileName = test;
                    startConversion( musicFile );
                }
            }
        }

        // If we started a process, the conversion is in progress
        if ( !m_convProcess )
        {
            m_musicFileName = musicFile;
            loadMusicFile();
        }
        else
            m_musicFile = 0;

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

    //m_background = new PlayerBackgroundColor();
    m_background = new PlayerBackgroundVideo();
    m_background->initFromSettings();

    return true;
}

void KaraokeFile::start()
{
    // If no conversion is in progress, start the player and rendering thread
    if ( !m_convProcess )
    {
        m_continue.store( 1 );
        m_player->start();
        QThread::start();
    }
    else
    {
       // Else we ignore it and wait until conversion is done
        m_customMessage = "MIDI conversion in progress";
    }
}

void KaraokeFile::stop()
{
    m_continue.store( 0 );
    wait();
}

void KaraokeFile::convError(QProcess::ProcessError error)
{
    m_customMessage = "MIDI conversion failed";
}

void KaraokeFile::convFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    // FIXME: copy?

    // Do not delete the object when we're in its slot!
    m_convProcess->deleteLater();
    m_convProcess = 0;

    m_customMessage.clear();
    start();
}

void KaraokeFile::startConversion(const QString &src)
{
    // Using Timidity
    QProcess * convProcess = new QProcess( this );
    connect( convProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(convError(QProcess::ProcessError)) );
    connect( convProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(convFinished(int,QProcess::ExitStatus)) );

    QStringList args;
    args << "-Ow" << src << "-o" << m_musicFileName;

    convProcess->start( "timidity", args );
}

void KaraokeFile::loadMusicFile()
{
    QFile * mf = new QFile( m_musicFileName );

    if ( !mf->open( QIODevice::ReadOnly ) )
    {
        QString err = mf->errorString();
        delete mf;
        throw QString("Cannot open music file %1: %2").arg( m_musicFileName ) .arg(err);
    }

    if ( !m_player->load( mf ) )
        throw QString( "Cannot play file %1: %2") .arg( m_musicFileName ) .arg( m_player->errorMsg() );

    m_musicFile = mf;
}

void KaraokeFile::run()
{
    // from http://www.koonsolo.com/news/dewitters-gameloop/
    // We do not need fast rendering, and need constant FPS, so 2nd solution works well

    // This assumes 25 FPS, so we have 1000/25ms per cycle
    static const unsigned int MS_PER_CYCLE = 1000 / 20;
    QTime next_cycle;
    QImage renderimage( 800, 600, QImage::Format_ARGB32 );

    while ( m_continue )
    {
        qint64 time = m_player->position();
        next_cycle = QTime::currentTime().addMSecs( MS_PER_CYCLE );

        renderimage.fill( Qt::black );

        // Redraw main screen
        KaraokePainter p( KaraokePainter::AREA_MAIN_SCREEN, time, renderimage );

        // Background is always on
        m_background->draw( p );

        // But lyrics are only on if the text is not set
        if ( m_customMessage.isEmpty() )
        {
            m_lyrics->draw( p );
            m_nextRedrawTime = m_lyrics->nextUpdate();
        }
        else
        {
            p.drawCenteredOutlineText( 50, Qt::white, m_customMessage );
        }

        p.end();

        if ( !m_widget->setImage( renderimage ) )
            continue; // rerender

        QMetaObject::invokeMethod( m_widget, "refresh", Qt::QueuedConnection );

        // When should we have next tick?
        int remainingms = qMax( QTime::currentTime().msecsTo( next_cycle ), 5 );
        msleep( remainingms );
    }
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
