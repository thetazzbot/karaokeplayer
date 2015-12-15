#include "karaokepainter.h"
#include "settings.h"

// Screen composition:
static const unsigned int TOP_SCREEN_RESERVED_STATUS = 5;

// Those are percents, not pixels
static const unsigned int TEXTLYRICS_SPACING_LEFTRIGHT = 5;
static const unsigned int TEXTLYRICS_SPACING_TOPBOTTOM = 3;


KaraokePainter::KaraokePainter(KaraokePainter::Area area, qint64 pos, qint64 duration, QImage &img)
    : QPainter( &img )
{
    m_time = pos;
    m_duration = duration;

    m_origTransform = transform();
    m_origSize = img.size();

    switchArea( area );
}


void KaraokePainter::setTextLyricsMode()
{
    // More clipping
    int xspacing = (m_rect.width() * TEXTLYRICS_SPACING_LEFTRIGHT) / 100;
    int yspacing = (m_rect.height() * TEXTLYRICS_SPACING_TOPBOTTOM) / 100;

    int textwidth = m_rect.width() - xspacing * 2;
    int textheight = m_rect.height() - yspacing * 2;

    m_rect = QRect( 0, 0, textwidth, textheight );
    translate( xspacing, yspacing );
}

void KaraokePainter::switchArea(KaraokePainter::Area area)
{
    setTransform( m_origTransform );

    int toparea = (m_origSize.height() * TOP_SCREEN_RESERVED_STATUS) / 100;

    if ( area == AREA_MAIN_SCREEN )
    {
        // Keep the top off the renderer
        m_rect = QRect( 0, 0, m_origSize.width(), m_origSize.height() - toparea );
        translate( 0, toparea );
    }
    else
    {
        // Allow the top
        m_rect = QRect( 0, 0, m_origSize.width(), toparea );
    }
}

int KaraokePainter::largetsFontSize(const QFont &font, int width, const QString &textline)
{
    int maxsize = 128;
    int minsize = 8;
    int cursize;
    QFont testfont( font );

    // We are trying to find the maximum font size which fits by doing the binary search
    while ( maxsize - minsize > 1 )
    {
        cursize = minsize + (maxsize - minsize) / 2;
        testfont.setPointSize( cursize );
        QFontMetrics fm( testfont );
        //qDebug("%d-%d: trying font size %d to draw on %d: width %d", minsize, maxsize, cursize, drawing_width, fm.width(m_longestLyricLine) );

        if ( fm.width( textline ) < width )
            minsize = cursize;
        else
            maxsize = cursize;
    }

    return cursize;
}

int KaraokePainter::tallestFontSize( QFont &font, int height )
{
    int maxsize = 128;
    int minsize = 8;
    int cursize;

    // We are trying to find the maximum font size which fits by doing the binary search
    while ( maxsize - minsize > 1 )
    {
        cursize = minsize + (maxsize - minsize) / 2;
        font.setPointSize( cursize );
        QFontMetrics fm( font );
        //qDebug("%d-%d: trying font size %d to draw on %d: width %d", minsize, maxsize, cursize, drawing_width, fm.width(m_longestLyricLine) );

        if ( fm.height() < height )
            minsize = cursize;
        else
            maxsize = cursize;
    }

    return cursize;
}

int KaraokePainter::largetsFontSize( const QString &textline )
{
    return largetsFontSize( font(), m_rect.width(), textline );
}

void KaraokePainter::drawOutlineText(int x, int y, const QColor &color, const QString &text)
{
    /*QPainterPath path;

    path.addText( x, y, font(), text );

    fillPath( path, QBrush(color) );
    QPen strokepen( Qt::black );
    strokepen.setWidth( font().pointSize() > 32 ? 2 : 1 );

    strokePath( path, strokepen );*/
    setPen( Qt::black );
    drawText( x-1, y-1, text );
    drawText( x-1, y+1, text );
    drawText( x+1, y-1, text );
    drawText( x+1, y+1, text );

    setPen( color );
    drawText( x, y, text );
}

void KaraokePainter::drawCenteredOutlineText(int ypercentage, const QColor &color, const QString &text)
{
    int x = (m_rect.width() - fontMetrics().width( text ) ) / 2;
    int y = (m_rect.height() * ypercentage / 100 ) + fontMetrics().height();

    drawOutlineText( x, y, color, text );
}
