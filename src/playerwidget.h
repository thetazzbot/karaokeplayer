#ifndef PLAYERWIDGET_H
#define PLAYERWIDGET_H

#include <QWidget>
#include <QMutex>
#include <QTime>

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
        explicit PlayerWidget( QWidget *parent = 0 );

        bool    setImage( QImage &img );

    public slots:
        void    refresh();

    protected:
        void    paintEvent(QPaintEvent * event);
        void    resizeEvent(QResizeEvent * event);

    private:
        // This image is being redrawn here
        QImage      m_image;

        // Prevents simultaneous access to image
        mutable QMutex m_mutex;

        // Tracking widget resizing
        QTime       m_lastResize;
};

#endif // PLAYERWIDGET_H
