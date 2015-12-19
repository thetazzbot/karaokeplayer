#ifndef PLAYERNOTIFICATION_H
#define PLAYERNOTIFICATION_H

#include <QList>
#include <QMutex>
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
        qint64  drawTop( KaraokePainter& p );

        qint64  drawRegular( KaraokePainter& p );


    public slots:
        void    queueUpdated();

        void    setMessage( const QString& message );
        void    clearMessage();

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

        // Custom message
        QMutex          m_mutex;
        QFont           m_customFont;
        QString         m_customMessage;

};

extern PlayerNotification * pNotification;

#endif // PLAYERNOTIFICATION_H
