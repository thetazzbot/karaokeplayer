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

#ifndef VERSION_H
#define VERSION_H

//TODO:
// change all timings to int / no qin64

// Features:
//
// Ensure QMediaPlayer etc works on all platforms for:
// - Playing music (all formats)
// - Good timing w/lyrics
// - Pause, seek, rewind
// - Video background
// then
// - Voice passthrough
// - Audio recording
// - Video screen capture/webcam capture
//

// - CDG, KFN, MID, LRC, Ultrastar, ZIP
// - Timidity
// - Remote song storage, download (incl encrypted)
// - Voice passthrough and recording
// - Video + voice recording


// DONE //
// - CDG, KFN, MID, LRC, Ultrastar, ZIP
// - Timidity
// - Video background
// - LIRC support
// - Queue
// - Song collection
// - Per-user queue w/intelligent
// - Remote management through Web site and Android app

#define APP_NAME			"Ulduzsoft Karaoke Player"

#define APP_VERSION_MAJOR	1
#define APP_VERSION_MINOR	0

#endif // VERSION_H
