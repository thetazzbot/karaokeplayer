#ifndef LYRICSPARSER_KFN_H
#define LYRICSPARSER_KFN_H

#include "lyricsparser.h"

class LyricsParser_KFN : public LyricsParser
{
    public:
        LyricsParser_KFN();

        // Parses the lyrics, filling up the output container. Throws an error
        // if there are any issues during parsing, otherwise fills up output.
        void parse( QIODevice * file, LyricsLoader::Container& output, LyricsLoader::Properties& properties );
};

#endif // LYRICSPARSER_KFN_H
