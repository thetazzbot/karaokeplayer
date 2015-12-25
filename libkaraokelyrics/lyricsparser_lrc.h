#ifndef LYRICSPARSER_LRC_H
#define LYRICSPARSER_LRC_H

#include "lyricsparser.h"

class LyricsParser_LRC : public LyricsParser
{
    public:
        LyricsParser_LRC();

        // Parses the lyrics, filling up the output container. Throws an error
        // if there are any issues during parsing, otherwise fills up output.
        void parse( QIODevice * file, LyricsLoader::Container& output, LyricsLoader::Properties& properties );

    private:
        int         offset;

    private:
        qint64  parseLRCTime( const QString& str );
};

#endif // LYRICSPARSER_LRC_H
