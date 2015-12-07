#ifndef KARAOKEFILE_H
#define KARAOKEFILE_H

#include <QIODevice>

#include "playerlyrics.h"
#include "playerbackground.h"


// Represents a playable Karaoke file. It may be one of the following:
// - Two files (music + lyrics of proper format);
// - One archive file (such as ZIP) containing the above;
// - A compound file (such as KFN) containing the above;
class KaraokeFile
{
    public:
        KaraokeFile();
        ~KaraokeFile();

        static bool isValid( const QString& filename );

        // open - a music file, a lyrics file, a combined file (KFN) or a ZIP archive
        bool    open( const QString& filename );

        // Accessors
        QIODevice * musicFile() const { return m_musicFile; }
        PlayerLyrics * lyrics() const { return m_lyrics; }
        PlayerBackground * background() const { return m_background; }

    protected:
        QIODevice        *   m_musicFile;
        PlayerLyrics     *   m_lyrics;
        PlayerBackground *   m_background;

        static bool isSupportedMusicFile( const QString& filename );
        static bool isSupportedLyricFile( const QString& filename );
};

#endif // KARAOKEFILE_H
