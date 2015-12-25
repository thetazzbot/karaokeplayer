#ifndef PLAYERRENDERER_H
#define PLAYERRENDERER_H

#include <QMutex>
#include <QThread>

class KaraokeWidget;
class PlayerNotification;
class KaraokeSong;

// This thread renders everything - lyrics, video, notifications. This way we can switch to GLWidget
// and keep this class as-is (almost)
//
class PlayerRenderer : public QThread
{
    Q_OBJECT

    public:
        explicit PlayerRenderer( PlayerNotification * notification, QImage * render, KaraokeWidget *parent );
        ~PlayerRenderer();

        void    stop();

    private:
        void    run();

    private:
        // Keeps renderer running (until exit)
        QAtomicInt          m_continue;

        QImage          *   m_renderImage;

        PlayerNotification* m_notification;

        KaraokeWidget    *   m_widget;
};

#endif // PLAYERRENDERER_H
