#ifndef KARAOKEPAINTER_H
#define KARAOKEPAINTER_H

#include <QImage>
#include <QPainter>


// Extends QPainter with our specific functionality
class KaraokePainter : public QPainter
{
    public:
        KaraokePainter( QImage * img );

        // Paint area locations
        QSize   size() const { return m_rect.size(); }
        QRect   rect() const { return m_rect; }

        // Setters and getters for times
        void    setTimes( qint64 position, qint64 duration );

        // Those would both return 0 if we are not playing
        qint64  time() const { return m_time; }
        qint64  duration() const { return m_duration; }

        // Returns the notification area rect
        QRect   notificationRect() const;

        // Sets clipping/translation to draw on the main area (no notifications)
        void    setClipAreaMain();

        // Sets clipping/translation to draw text on the main area (extra padding)
        void    setClipAreaText();

        // Returns the largest font size the textline could fit the current area lenght-wise
        int     largetsFontSize( const QString& textline );

        // Same but also using the specific font and specific width
        static int     largetsFontSize( const QFont& font, int width, const QString& textline );

        // Returns the tallest font size which could fit into the specified height
        static int     tallestFontSize( QFont &font, int height );

        // Draws text with the specified color, but with black outline
        void    drawOutlineText( int x, int y, const QColor& color, const QString& text );

        // Draws text x-centered at percentage y-wise (such as "70% of y") with the specified color, but with black outline
        void    drawCenteredOutlineText( int ypercentage, const QColor& color, const QString& text );

    private:
        QRect       m_rect;
        qint64      m_time;
        qint64      m_duration;
        QImage  *   m_image;
};

#endif // KARAOKEPAINTER_H
