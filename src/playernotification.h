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

#ifndef PLAYERNOTIFICATION_H
#define PLAYERNOTIFICATION_H

#include <QList>
#include <QMutex>
#include <QString>

#include "karaokepainter.h"

class Background;

// Renders a notification on top of the screen
class PlayerNotification : public QObject
{
    Q_OBJECT

    public:
        PlayerNotification( QObject * parent );
        ~PlayerNotification();

        // Draws the notification on the painter. Returns time for the next update
        // when the image will change.
        qint64  drawTop(KaraokePainter& p);

        // Draw the regular notification
        qint64  drawRegular( KaraokePainter& p );


    public slots:
        void    queueUpdated();

        // Received from notification handler; to pause the animation on background
        void    songStarted();
        void    songStopped();

        void    setOnScreenMessage( const QString& message );
        void    clearOnScreenMessage();

        void    setMessage( const QString& message, int show = 2000 );

    private:
        void    reset();

    private:
        QFont           m_font;
        int             m_lastScreenHeight;

        QString         m_firstItem;
        QString         m_notificationLine;
        QString         m_textQueueSize;

        int             m_textOffset;
        int             m_scrollOffset;
        int             m_scrollStep;

        // Custom message
        QMutex          m_mutex;
        QFont           m_customFont;
        QString         m_customMessage;
};

extern PlayerNotification * pNotification;

#endif // PLAYERNOTIFICATION_H
