#ifndef PLAYERLYRICTEXTLINE_H
#define PLAYERLYRICTEXTLINE_H

#include <QFont>
#include <QPainter>
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
        qint64  endTime() const;

        // Draw the lyrics according to time. Returns next time when the lyrics need
        // to be updated
        qint64  draw(qint64 time, int width, int yoffset, QPainter& p);

        // Draw the disappearing animation with specific percentage(0-100)
        void    drawDisappear( int percentage, int width, int yoffset, QPainter& p );

        // Empty set means screen space, and will not have draw/nextTime called
        bool    isEmpty() const { return m_line.empty(); }

        // Should return the longest  possible width (i.e. if the width increases when the words
        // are being played, should return the width as if the longest part is being played)
        int     screenWidth( const QFontMetrics& font );

        // Dump the text line, optionally with the time marker
        void    dump( qint64 time = -1 ) const;

        static void drawOutlineText( QPainter& p, int x, int y, const QColor& color, const QString& text );
        static void drawOutlineText( QPainter& p, const QSize& size, int ypercentage, const QColor& color, const QString& text );

    private:
        LyricsLoader::Container     m_line;

        QString                     m_text;


};

#endif // PLAYERLYRICTEXTLINE_H
