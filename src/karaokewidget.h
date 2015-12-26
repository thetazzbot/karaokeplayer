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

#ifndef KARAOKEWIDGET_H
#define KARAOKEWIDGET_H

#include <QDockWidget>
#include <QWidget>
#include <QMutex>

// This class shows lyrics and background (video, pictures). It gets notifications from the player
// when timing changes, and calls the background and lyrics renderer.
//
// It also stores a copy of the rendered image and provides it to the recorder

class KaraokeSong;
class PlayerRenderer;
class PlayerNotification;

class KaraokeWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit KaraokeWidget( QWidget *parent = 0 );
        ~KaraokeWidget();

        // Set current karaoke song
        void    startKaraoke( KaraokeSong * k );
        void    stopKaraoke();
        void    stopEverything();

    protected:
        void    paintEvent(QPaintEvent *);
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
        KaraokeSong     *   m_karaoke;

        // Protects m_karaoke while being rendered - used both from widget and rendering thread
        mutable QMutex  m_karaokeMutex;

};

#endif // KARAOKEWIDGET_H
