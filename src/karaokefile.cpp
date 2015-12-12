#include <QDir>
#include <QCryptographicHash>

#include "settings.h"
#include "karaokefile.h"
#include "playerlyricscdg.h"
#include "playerlyricstext.h"
#include "playerbackground.h"
#include "playerbackgroundcolor.h"
#include "libkaraokelyrics/lyricsloader.h"


KaraokeFile::KaraokeFile()
{
    m_musicFile = 0;
    m_lyrics = 0;
    m_background = 0;
    m_convProcess = 0;
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
                    m_musicFileName = musicFile;
                    m_cachedFileName = test;
                }
            }
        }

        if ( !needsConversion() )
        {
            QFile * mf = new QFile( musicFile );

            if ( !mf->open( QIODevice::ReadOnly ) )
            {
                QString err = mf->errorString();
                delete mf;
                throw QString("Cannot open music file %1: %2").arg( musicFile ) .arg(err);
            }

            m_musicFileName = musicFile;
            m_musicFile = mf;
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

    m_background = new PlayerBackgroundColor();
    m_background->initFromSettings();

    return true;
}

void KaraokeFile::startConversion()
{
    qDebug("started conversion of %s to %s", qPrintable(m_musicFileName), qPrintable(m_cachedFileName) );

    QProcess * convProcess = new QProcess( this );
    connect( convProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(convError(QProcess::ProcessError)) );
    connect( convProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(convFinished(int,QProcess::ExitStatus)) );

    QStringList args;
    args << "-Ow" << m_musicFileName << "-o" << m_cachedFileName;
    convProcess->start( "timidity", args );
}

void KaraokeFile::convError(QProcess::ProcessError error)
{
    emit conversionFinished( 255 );

}

void KaraokeFile::convFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    // FIXME: copy?
    QFile * mf = new QFile( m_cachedFileName );

    if ( !mf->open( QIODevice::ReadOnly ) )
    {
        emit conversionFinished( 255 );
        return;
    }

    m_musicFileName = m_cachedFileName;
    m_musicFile = mf;

    emit conversionFinished( exitCode );
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
