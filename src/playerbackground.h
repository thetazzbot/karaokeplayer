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

#ifndef PLAYERBACKGROUND_H
#define PLAYERBACKGROUND_H

#include <QIODevice>

class KaraokePainter;

// An abstract background renderer class
class PlayerBackground
{
    public:
        PlayerBackground();
        virtual ~PlayerBackground();

        // Background could be initialized either from the settings (by callign initFromSettings() or from
        // a specific file/QIODevice - for example for KFN files. If the background cannot be initialized
        // a specific way, it must return an error.
        virtual bool    initFromSettings( const QString& param = "" ) = 0;
        virtual bool    initFromFile( QIODevice * file ) = 0;

        // Draws the background on the painter; the painter is filled up black. Returns time for the
        // next update when the image will change. -1 means the current image will never change.
        virtual qint64  draw( KaraokePainter& p ) = 0;

        // Current state notifications, may be reimplemented. Default implementation does nothing.
        virtual void    start();
        virtual void    pause( bool pausing );
        virtual void    stop();
};

// An empty background
class PlayerBackgroundNone : public PlayerBackground
{
    public:
        PlayerBackgroundNone() { }
        virtual ~PlayerBackgroundNone() { }

        // Background could be initialized either from the settings (by callign initFromSettings() or from
        // a specific file/QIODevice - for example for KFN files. If the background cannot be initialized
        // a specific way, it must return an error.
        bool    initFromSettings( const QString& param = "" ) { return true; }
        bool    initFromFile( QIODevice * file )  { return true; }

        // Draws the background on the painter; the painter is filled up black. Returns time for the
        // next update when the image will change. -1 means the current image will never change.
        qint64  draw( KaraokePainter& p )  { return -1; }
};


#endif // PLAYERBACKGROUND_H
