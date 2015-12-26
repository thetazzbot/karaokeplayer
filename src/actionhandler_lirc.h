/**************************************************************************
 *  Ulduzsoft Karaoke PLayer - cross-platform desktop karaoke player      *
 *  Copyright (C) 2015-2016 George Yunaev, support@ulduzsoft.com          *
 *                                                                        *
 *  This program is free software: you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation, either version 3 of the License, or     *
 *  (at your option) any later version.                                   *
 *																	      *
 *  This program is distributed in the hope that it will be useful,       *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *  GNU General Public License for more details.                          *
 *                                                                        *
 *  You should have received a copy of the GNU General Public License     *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 **************************************************************************/

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
        void	error(QLocalSocket::LocalSocketError);
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
