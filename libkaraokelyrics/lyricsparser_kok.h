#ifndef LYRICSPARSER_KOK_H
#define LYRICSPARSER_KOK_H

#include "lyricsparser.h"

class LyricsParser_KOK : public LyricsParser
{
    public:
        LyricsParser_KOK();

        // Parses the lyrics, filling up the output container. Throws an error
        // if there are any issues during parsing, otherwise fills up output.
        void parse( QIODevice * file, LyricsLoader::Container& output, LyricsLoader::Properties& properties );
};

#endif // LYRICSPARSER_KOK_H
