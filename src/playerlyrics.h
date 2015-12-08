#ifndef LYRICSRENDERER_H
#define LYRICSRENDERER_H

#include <QString>
#include <QColor>
#include <QIODevice>


class KaraokePainter;

//
// This class is a base class for the lyric renderer of all kind (text and graphics, in future maybe video too)
// All implemented renderers must take care of:
// - Loading the lyrics (with necessary preprocessing if needed), either from disk or from an archive;
// - Rendering the lyrics for a specific timeframe in a proper state - it should expect the new time to be far ahead or behind;
// - Being able to tell whether the lyrics need to be redrawn between the last and current timeframe
//
class PlayerLyrics
{
    public:
        PlayerLyrics();
        virtual ~PlayerLyrics();

        // Finds the lyrics for a music file among the files. This function receives a list of files in a current

        // Load the lyrics from the QIODevice (which could be an original file or an archive entry)
        // Returns true if load succeed, otherwise must return false and set m_errorMsg to the reason
        virtual bool    load( QIODevice& file, const QString &filename = "" ) = 0;

        // Must return next time when the lyrics would be updated. If returns -1, no more updates
        virtual qint64  nextUpdate() const = 0;

        // Render the lyrics; also sets the m_lastRenderedTime
        bool    draw( KaraokePainter& p );

        // Error message
        QString errorMsg() const { return m_errorMsg; }

    protected:
        // Render lyrics for current timestamp into the QImage provided. Must "render over",
        // and not mess up with the rest of the screen. True on success, false + m_errorMsg on error.
        virtual bool    render( KaraokePainter& p ) = 0;

    protected:
        // Any error should set this member
        QString         m_errorMsg;

        // Last time lyrics renderer was called (in ms)
        qint64          m_lastRenderedTime;
};

#endif // LYRICSRENDERER_H
