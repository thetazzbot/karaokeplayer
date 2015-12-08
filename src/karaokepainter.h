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

        KaraokePainter( Area area, qint64 timems, QImage& img );

        qint64  time() const { return m_time; }
        QSize   size() const { return m_rect.size(); }
        QRect   rect() const { return m_rect; }

        void    setTextLyricsMode();

        int     largetsFontSize( const QString& textline );
        static int     largetsFontSize( const QFont& font, int width, const QString& textline );

        void    drawOutlineText( int x, int y, const QColor& color, const QString& text );
        void    drawCenteredOutlineText( int ypercentage, const QColor& color, const QString& text );

    private:
        QRect       m_rect;
        qint64      m_time;

};

#endif // KARAOKEPAINTER_H
