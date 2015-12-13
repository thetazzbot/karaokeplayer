#ifndef PLAYERWIDGET_H
#define PLAYERWIDGET_H

#include <QWidget>
#include <QMutex>

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

        QImage& acquireImage();
        void    releaseImage() const;

    public slots:
        void    refresh();

    protected:
        void    paintEvent(QPaintEvent * event);
//        void    resizeEvent(QResizeEvent * event);

    private:
        // This image is being redrawn here
        QImage      m_image;

        // Prevents simultaneous access to image
        mutable QMutex m_mutex;
};

#endif // PLAYERWIDGET_H
