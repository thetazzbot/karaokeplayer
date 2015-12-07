#ifndef LYRICSPARSER_ULTRASTAR_H
#define LYRICSPARSER_ULTRASTAR_H

#include <QStringList>
#include "lyricsparser.h"

// This class parses both Ultrastar and PowerKaraoke lyrics
class LyricsParser_Texts : public LyricsParser
{
    public:
        LyricsParser_Texts();

        // Parses the lyrics, filling up the output container. Throws an error
        // if there are any issues during parsing, otherwise fills up output.
        void parse( QIODevice& file, LyricsLoader::Container& output, LyricsLoader::Properties& properties );

    private:
        void parseUStar( const QStringList& text, LyricsLoader::Container& output, LyricsLoader::Properties& properties );
        void parsePowerKaraoke( const QStringList& text, LyricsLoader::Container& output, LyricsLoader::Properties& properties );
};

#endif // LYRICSPARSER_ULTRASTAR_H
