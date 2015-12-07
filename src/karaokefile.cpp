#include <QDir>

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
}

KaraokeFile::~KaraokeFile()
{
    delete m_musicFile;
    delete m_lyrics;
    delete m_background;
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
        QFile * mf = new QFile( musicFile );

        if ( !mf->open( QIODevice::ReadOnly ) )
        {
            QString err = mf->errorString();
            delete mf;
            throw QString("Cannot open music file %1: %2").arg( musicFile ) .arg(err);
        }

        m_musicFile = mf;

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

bool KaraokeFile::isSupportedMusicFile(const QString &filename)
{
    static const char * extlist[] = { ".mp3", ".wav", ".ogg", ".aac", ".flac", ".kar", ".mid", ".midi", 0 };

    for ( const char * ext = extlist[0]; *ext; ext += strlen(ext) + 1 )
        if ( filename.endsWith( ext, Qt::CaseInsensitive ) )
            return true;

    return false;
}

bool KaraokeFile::isSupportedLyricFile(const QString &filename)
{
    return LyricsLoader::isSupportedFile( filename ) || filename.endsWith( ".cdg", Qt::CaseInsensitive );
}
