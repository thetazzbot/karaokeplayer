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

#include <QScopedPointer>
#include <QStringList>
#include <QBuffer>

#include "karaokeplayable.h"
#include "karaokeplayable_file.h"
#include "karaokeplayable_zip.h"
#include "karaokeplayable_kfn.h"

#include "libkaraokelyrics/lyricsloader.h"

KaraokePlayable::KaraokePlayable(const QString &baseFile)
{
    m_baseFile = baseFile;
}


KaraokePlayable::~KaraokePlayable()
{
}

KaraokePlayable *KaraokePlayable::create(const QString &baseFile)
{
    KaraokePlayable * obj;

    if ( baseFile.endsWith( ".zip", Qt::CaseInsensitive ) )
        obj = new KaraokePlayable_ZIP( baseFile );
    else if ( baseFile.endsWith( ".kfn", Qt::CaseInsensitive ) )
        obj = new KaraokePlayable_KFN( baseFile );
    else
        obj = new KaraokePlayable_File( baseFile );

    return obj;
}

bool KaraokePlayable::parse()
{
    // Init typically loads and parses the file for compound formats
    if ( !init() )
        return false;

    // Sometime we already have the music and lyrics from this parsing
    if ( !m_musicObject.isEmpty() && !m_lyricObject.isEmpty() )
        return true;

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

QIODevice *KaraokePlayable::openObject(const QString &object)
{
    // Same logic as with lyrics above
    if ( isCompound() )
    {
        // Extract the data into QBuffer which we would then use to read the data from
        QBuffer * buffer = new QBuffer();
        buffer->open( QIODevice::ReadWrite );

        // Extract the lyrics into our buffer; failure is not a problem here
        if ( !extract( object, buffer ) )
            return 0;

        buffer->reset();
        return buffer;
    }
    else
    {
        // Just a regular file
        QFile * file = new QFile( absolutePath( object ) );

        if ( !file->open( QIODevice::ReadOnly ) )
            return 0;

        return file;
    }
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

bool KaraokePlayable::isSupportedCompleteFile(const QString &filename)
{
    static const char * extlist[] = { ".kar", ".mid", ".midi", ".kfn", ".zip", 0 };

    for ( int i = 0; extlist[i]; i++ )
        if ( filename.endsWith( extlist[i], Qt::CaseInsensitive ) )
            return true;

    return isVideoFile( filename );
}
