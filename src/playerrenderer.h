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
