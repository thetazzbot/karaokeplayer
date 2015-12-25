#ifndef PLAYERLYRICSTEXT_H
#define PLAYERLYRICSTEXT_H

#include <QFont>
#include <QString>
#include <QSize>
#include <QList>

#include "playerlyrics.h"
#include "playerlyrictext_line.h"
#include "libkaraokelyrics/lyricsloader.h"

class KaraokePainter;

// This class encapsulates all possible text formats
class PlayerLyricsText : public PlayerLyrics
{
    public:
        enum
        {
            MAX_DISAPPEARING_TIME_MS = 1000,

            MIN_DURATION_TITLE = 1000,
            MAX_DURATION_TITLE = 10000,
            TITLE_FADEOUT_TIME = 500,
            MIN_INTERVAL_AFTER_TITLE = 2000,

            // Notification parameters
            MIN_NOTIFICATION_PAUSE = 8000,
            MAX_NOTIFICATION_DURATION = 5000,
        };

        PlayerLyricsText();

        // Load the lyrics from the QIODevice (which could be an original file or an archive entry)
        // Returns true if load succeed, otherwise must return false and set m_errorMsg to the reason
        bool    load( QIODevice * file, const QString &filename = "" );

        // Must return next time when the lyrics would be updated. If returns -1, no more updates
        qint64  nextUpdate() const;

    protected:
        // Render lyrics for current timestamp into the QImage provided. Must "render over",
        // and not mess up with the rest of the screen. True on success, false + m_errorMsg on error.
        virtual bool    render( KaraokePainter& p );

        void    calculateFontSize();
        int     largetsFontSize( const QSize& size, const QString& text );
        void    renderTitle( KaraokePainter &p );
        qint64  firstLyricStart() const;

        void    drawNotification( KaraokePainter &p, qint64 timeleft);

    private:
        // Lyric lines
        QList<PlayerLyricTextLine>  m_lines;

        // The current lyric line played
        int                         m_currentLine;

        // The longest lyric sentence (which generated the widest rendering)
        int                         m_longestLine;

        // Time when the next display update is required
        qint64                      m_nextUpdateTime;

        // Rendering font - family is from settings, but size is autocalculated.
        QFont                       m_renderFont;

        // The font size is calculated for this image size
        QRect                       m_usedImageSize;

        // Current song artist and title
        QString                     m_artist;
        QString                     m_title;
        qint64                      m_showTitleTime;
};

#endif // PLAYERLYRICSTEXT_H
