#ifndef LYRICSPARSER_H
#define LYRICSPARSER_H

#include <QIODevice>
#include <QStringList>

#include "lyricsloader.h"


// Base for the loader of lyrics of various formats.
// Mostly used to offer shared functions which are used among -
// those objects are transitional, and only created when lyrics
// of a specific format are parsed.
class LyricsParser
{
    public:
        enum Type
        {
            TYPE_UNKNOWN,
            TYPE_MID,
            TYPE_LRC,
            TYPE_TXT,   // may be Ultrastar or Power Karaoke
            TYPE_KFN,
            TYPE_KOK
        };

        LyricsParser();
        virtual ~LyricsParser();

        // Parses the lyrics, filling up the output container. Throws an error
        // if there are any issues during parsing, otherwise fills up output.
        virtual void parse( QIODevice * file, LyricsLoader::Container& output, LyricsLoader::Properties& properties ) = 0;

        // Timing to text converter
        static QString timeAsText( quint64 timing );

    protected:
        // This should handle encoding as well
        QStringList loadText( QIODevice * file );

        // Returns the lyric type by the file extension
        static Type getTypeByExtension( const QString& extension );
};

#endif // LYRICSPARSER_H
