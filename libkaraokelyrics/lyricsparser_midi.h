#ifndef LYRICSPARSER_MIDI_H
#define LYRICSPARSER_MIDI_H

#include "lyricsparser.h"

class LyricsParser_MIDI : public LyricsParser
{
    public:
        LyricsParser_MIDI();

        // Parses the lyrics, filling up the output container. Throws an error
        // if there are any issues during parsing, otherwise fills up output.
        void parse( QIODevice * file, LyricsLoader::Container& output, LyricsLoader::Properties& properties );

    private:
        // Byteorder-safe readers
        unsigned char 	readByte();
        unsigned short	readWord();
        unsigned int	readDword();
        int 			readVarLen();
        void			readData( void * buf, unsigned int length );

        unsigned int 	currentPos() const;
        void			setPos( unsigned int offset );

        QByteArray      m_mididata;
        int             m_currentOffset;
};

#endif // LYRICSPARSER_MIDI_H
