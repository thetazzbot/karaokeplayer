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

#ifndef KARAOKESONG_H
#define KARAOKESONG_H

#include <QIODevice>
#include <QProcess>

#include "universalplayer.h"
#include "playerlyrics.h"
#include "playerbackground.h"
#include "songqueue.h"


class KaraokeWidget;

// Represents a playable Karaoke file. It may be one of the following:
// - Two files (music + lyrics of proper format);
// - One archive file (such as ZIP) containing the above;
// - A compound file (such as KFN) containing the above;
//
// All preparation (and conversion) happens in the main thread, without a new thread
// being started. Then only when the player should be starting, the new thread
// starts too
class KaraokeSong : public QObject
{
    Q_OBJECT

    public:
        KaraokeSong( KaraokeWidget * w, const SongQueue::Song& song );
        ~KaraokeSong();

        // Checks if the file queued needs processing (i.e. MIDI conversion)
        static bool needsProcessing( const QString& filename );

        // open a file
        bool    open();

        // Draws current song in its current state
        qint64  draw( KaraokePainter& p );

    public slots:
        void    start();
        void    pause();
        void    seekForward();
        void    seekBackward();
        void    seekTo( qint64 timing );
        void    stop();
        void    lyricEarlier();
        void    lyricLater();

    protected:
        // Convert the src to the m_musicFileName
        void    startConversion( const QString& src );

        QString             m_musicFileName;
        PlayerLyrics     *  m_lyrics;
        PlayerBackground *  m_background;

        // Player and rendering widget
        UniversalPlayer     m_player;
        KaraokeWidget    *  m_widget;

        // Next redraw time
        qint64              m_lastRedrawTime;
        qint64              m_nextRedrawTime;

        // Song info
        SongQueue::Song     m_song;

        // Song rating
        int                 m_rating;

        static bool isMidiFile( const QString& filename );
        static bool isSupportedMusicFile( const QString& filename );
        static bool isSupportedLyricFile( const QString& filename );
};

#endif // KARAOKESONG_H
