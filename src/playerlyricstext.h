#ifndef PLAYERLYRICSTEXT_H
#define PLAYERLYRICSTEXT_H

#include <QFont>
#include <QString>
#include <QSize>
#include <QList>

#include "playerlyrics.h"
#include "playerlyrictext_line.h"
#include "libkaraokelyrics/lyricsloader.h"

// This class encapsulates all possible text formats
class PlayerLyricsText : public PlayerLyrics
{
    public:
        enum
        {
            PADDING_LEFTRIGHT_PERCENTAGE = 10,
            PADDING_TOPBOTTOM_PERCENTAGE = 5,
        };
        PlayerLyricsText();

        // Load the lyrics from the QIODevice (which could be an original file or an archive entry)
        // Returns true if load succeed, otherwise must return false and set m_errorMsg to the reason
        bool    load( QIODevice& file, const QString &filename = "" );

        // Must return next time when the lyrics would be updated. If returns -1, no more updates
        qint64  nextUpdate() const;

    protected:
        // Render lyrics for current timestamp into the QImage provided. Must "render over",
        // and not mess up with the rest of the screen. True on success, false + m_errorMsg on error.
        virtual bool    render( qint64 timems, QImage& image );

        void    calculateFontSize();

    private:
        LyricsLoader::Properties    m_properties;

        // Lyric lines
        QList<PlayerLyricTextLine>  m_lines;

        // The current lyric line played
        int                         m_currentLine;

        // The longest lyric sentence (which generated the widest rendering)
        int                         m_longestLine;

        // Time when the next lyric to be played
        qint64                      m_nextLyricTime;

        // Rendering font - family is from settings, but size is autocalculated.
        QFont                       m_renderFont;

        // The font size is calculated for this image size
        QSize                       m_usedImageSize;
};

#endif // PLAYERLYRICSTEXT_H
