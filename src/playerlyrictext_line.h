#ifndef PLAYERLYRICTEXTLINE_H
#define PLAYERLYRICTEXTLINE_H

#include <QFont>
#include <QImage>
#include <QFontMetrics>

#include "libkaraokelyrics/lyricsloader.h"

// Encapsulates a single lyric text line, together with duration, timing and pitch info
// Draws inself in certain state depending on time (not yet, current, disappearing)
class PlayerLyricTextLine
{
    public:
        // Empty constructor - paragraph break, draws nothing
        PlayerLyricTextLine();
        PlayerLyricTextLine( const LyricsLoader::Container& line );

        // When is the next time something needs to be (re)drawn assuming no FF/RW
        qint64  startTime() const;

        // Draw the lyrics according to time. Returns next time when the lyrics need
        // to be updated
        qint64  draw( qint64 time, int yoffset, const QFont& font, QImage& image );

        // Empty set means screen space, and will not have draw/nextTime called
        bool    isEmpty() const { return m_line.empty(); }

        // Should return the longest  possible width (i.e. if the width increases when the words
        // are being played, should return the width as if the longest part is being played)
        int     screenWidth( const QFontMetrics& font );

        // Dump the text line, optionally with the time marker
        void    dump( qint64 time = -1 ) const;

    private:
        LyricsLoader::Container     m_line;

};

#endif // PLAYERLYRICTEXTLINE_H
