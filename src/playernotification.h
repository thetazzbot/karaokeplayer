#ifndef PLAYERNOTIFICATION_H
#define PLAYERNOTIFICATION_H

#include <QList>
#include <QString>

#include "karaokepainter.h"

// Renders a notification on top of the screen
class PlayerNotification : public QObject
{
    Q_OBJECT

    public:
        PlayerNotification( QObject * parent );

        // Draws the notification on the painter. Returns time for the next update
        // when the image will change.
        virtual qint64  draw( KaraokePainter& p );

    public slots:
        void    queueUpdated();

    private:
        void    reset();

    private:
        QFont           m_font;
        QFontMetrics    m_fontMetrics;
        unsigned int    m_lastScreenHeight;

        QString         m_firstItem;
        QString         m_notificationLine;

        unsigned int    m_textOffset;
        unsigned int    m_scrollOffset;
};

extern PlayerNotification * pNotification;

#endif // PLAYERNOTIFICATION_H
