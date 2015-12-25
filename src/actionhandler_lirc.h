#ifndef EVENTCONTROLLER_LIRC_H
#define EVENTCONTROLLER_LIRC_H

#include <QMap>
#include <QObject>
#include <QLocalSocket>

#include "actionhandler.h"

class ActionHandler_LIRC : public QObject
{
    Q_OBJECT

    public:
        explicit ActionHandler_LIRC(QObject *parent = 0);

    signals:
        void    lircEvent( int event );

    private slots:
        void	connected();
        void	disconnected();
        void	error( QLocalSocket::LocalSocketError socketError );
        void	readyRead();

    private:
        void    readMap();

        QLocalSocket    m_socket;

        // Buffers the partially-read data
        QByteArray      m_buffer;

        // Translation map
        QMap< QByteArray, int > m_eventMap;
};

#endif // EVENTCONTROLLER_LIRC_H
