#ifndef PLAYERWIDGET_H
#define PLAYERWIDGET_H

#include <QWidget>

// This class shows lyrics and background (video, pictures). It gets notifications from the player
// when timing changes, and calls the background and lyrics renderer.
//
// It also stores a copy of the rendered image and provides it to the recorder

class PlayerLyrics;
class PlayerBackground;

class PlayerWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit PlayerWidget( PlayerLyrics * lyricRenderer, PlayerBackground  * bgRenderer, QWidget *parent = 0 );

    signals:

    public slots:
        void    showCustomText( const QString& text );
        void    updateLyrics( qint64 time );
        void    redrawLyrics();

    protected:
        void    paintEvent(QPaintEvent * event);
        void    resizeEvent(QResizeEvent * event);

    private:
        // This image is being redrawn here
        QImage      m_image;

        // Background and lyrics renderers
        PlayerLyrics    * m_lyricRenderer;
        PlayerBackground  * m_backgroundRenderer;

        // Next redraw time
        qint64      m_lastRedrawTime;
        qint64      m_nextRedrawTime;

        // Custom text to show instead of lyrics
        QString     m_customText;
};

#endif // PLAYERWIDGET_H
