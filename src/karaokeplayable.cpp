#include <QStringList>

#include "karaokeplayable.h"
#include "karaokeplayable_file.h"
#include "karaokeplayable_zip.h"

#include "libkaraokelyrics/lyricsloader.h"

KaraokePlayable::KaraokePlayable()
{
}


KaraokePlayable::~KaraokePlayable()
{
}

KaraokePlayable *KaraokePlayable::create(const QString &baseFile)
{
    KaraokePlayable * obj;

    if ( baseFile.endsWith( ".zip", Qt::CaseInsensitive ) )
        obj = new KaraokePlayable_ZIP();
    else
        obj = new KaraokePlayable_File();

    obj->m_baseFile = baseFile;
    return obj;
}

bool KaraokePlayable::parse()
{
    // One file could be both music and lyrics (i.e. KAR)
    if ( isSupportedMusicFile( m_baseFile ) )
        m_musicObject = m_baseFile;

    if ( isSupportedLyricFile( m_baseFile ) )
        m_lyricObject = m_baseFile;

    // Not a music nor lyric file, and not compound?
    if ( m_musicObject.isEmpty() && m_lyricObject.isEmpty() && !isCompound() )
        return false;

    // Find the missing component(s) if we miss anything
    if ( m_musicObject.isEmpty() || m_lyricObject.isEmpty() )
    {
        QStringList objects = enumerate();

        foreach ( const QString& obj, objects )
        {
            if ( m_lyricObject.isEmpty() && isSupportedLyricFile( obj ) )
                m_lyricObject = obj;

            if ( m_musicObject.isEmpty() && isSupportedMusicFile( obj ) )
                m_musicObject = obj;
        }
    }

    // Final check
    if ( m_musicObject.isEmpty() && m_lyricObject.isEmpty() )
        return false;

    return true;
}

QString KaraokePlayable::lyricObject() const
{
    return m_lyricObject;
}

QString KaraokePlayable::musicObject() const
{
    return m_musicObject;
}

bool KaraokePlayable::isSupportedMusicFile(const QString &filename)
{
    static const char * extlist[] = { ".mp3", ".wav", ".ogg", ".aac", ".flac", ".kar", ".mid", ".midi", 0 };

    for ( int i = 0; extlist[i]; i++ )
        if ( filename.endsWith( extlist[i], Qt::CaseInsensitive ) )
            return true;

    return false;
}

bool KaraokePlayable::isSupportedLyricFile(const QString &filename)
{
    return LyricsLoader::isSupportedFile( filename ) || filename.endsWith( ".cdg", Qt::CaseInsensitive );
}

bool KaraokePlayable::isMidiFile(const QString &filename)
{
    static const char * extlist[] = { ".kar", ".mid", ".midi", 0 };

    for ( int i = 0; extlist[i]; i++ )
        if ( filename.endsWith( extlist[i], Qt::CaseInsensitive ) )
            return true;

    return false;
}

bool KaraokePlayable::isVideoFile(const QString &filename)
{
    static const char * extlist[] = { ".mp4", ".mov", ".avi", ".mkv", ".3gp", ".flv", 0 };

    for ( int i = 0; extlist[i]; i++ )
        if ( filename.endsWith( extlist[i], Qt::CaseInsensitive ) )
            return true;

    return false;
}
