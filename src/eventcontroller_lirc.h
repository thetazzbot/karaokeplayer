#ifndef EVENTCONTROLLER_LIRC_H
#define EVENTCONTROLLER_LIRC_H

#include <QObject>
#include <QLocalSocket>

#include "eventcontroller.h"

class EventController_LIRC : public QObject
{
    Q_OBJECT

    public:
        explicit EventController_LIRC(QObject *parent = 0);

    signals:
        void    lircEvent( EventController::Event event );

    private:
        QLocalSocket    m_socket;
};

#endif // EVENTCONTROLLER_LIRC_H
