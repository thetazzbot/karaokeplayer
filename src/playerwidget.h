#ifndef PLAYERWIDGET_H
#define PLAYERWIDGET_H

#include <QWidget>
#include <QMutex>

// This class shows lyrics and background (video, pictures). It gets notifications from the player
// when timing changes, and calls the background and lyrics renderer.
//
// It also stores a copy of the rendered image and provides it to the recorder

class KaraokeFile;
class PlayerRenderer;
class PlayerNotification;

class PlayerWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit PlayerWidget( QWidget *parent = 0 );
        ~PlayerWidget();

        // Set current karaoke song
        void    startKaraoke( KaraokeFile * k );
        void    stopKaraoke();

        // Current karaoke position
        qint64  position() const;

    protected:
        void    closeEvent(QCloseEvent * event);
        void    paintEvent(QPaintEvent * event);
        void    keyPressEvent(QKeyEvent * event);

    private:
        friend class PlayerRenderer;

        // This is called from renderer. This sets the current rendering image as drawing,
        // and the drawing image as rendering
        QImage * switchImages();

        // Those images are used for rendering, at each moment
        // one image is "main", and the second one is rendered into. When
        // switchImages() is called, they switch. m_drawImageIndex points to the draw image.
        QImage   *  m_images[2];

        // Switching the images happen by changing this index, so it must be atomic
        int         m_drawImageIndex;

        // Protects m_images and m_drawImageIndex
        QMutex      m_imageMutex;

        // Player widget renderer, runs in a dedicated thread
        // and renders everything into image
        PlayerRenderer* m_renderer;

        // Current karaoke file
        KaraokeFile     *   m_karaoke;

        // Protects m_karaoke while being rendered - used both from widget and rendering thread
        mutable QMutex  m_karaokeMutex;

};

#endif // PLAYERWIDGET_H
