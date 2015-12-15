#ifndef KARAOKEPAINTER_H
#define KARAOKEPAINTER_H

#include <QPainter>

// Extends QPainter with our specific functionality
class KaraokePainter : public QPainter
{
    public:
        enum Area
        {
            AREA_TOP,
            AREA_MAIN_SCREEN,
        };

        KaraokePainter( Area area, qint64 pos, qint64 duration, QImage& img );

        qint64  time() const { return m_time; }
        qint64  duration() const { return m_duration; }
        QSize   size() const { return m_rect.size(); }
        QRect   rect() const { return m_rect; }

        void    setTextLyricsMode();
        void    switchArea( Area area );

        int     largetsFontSize( const QString& textline );
        static int     tallestFontSize( QFont &font, int height );
        static int     largetsFontSize( const QFont& font, int width, const QString& textline );

        void    drawOutlineText( int x, int y, const QColor& color, const QString& text );
        void    drawCenteredOutlineText( int ypercentage, const QColor& color, const QString& text );

    private:
        QRect       m_rect;
        qint64      m_time;
        qint64      m_duration;

        QTransform  m_origTransform;
        QSize       m_origSize;

};

#endif // KARAOKEPAINTER_H
