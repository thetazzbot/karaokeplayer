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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QFont>
#include <QColor>

class Settings
{
    public:
        Settings();

        enum BackgroundType
        {
            BACKGROUND_TYPE_COLOR,
            BACKGROUND_TYPE_IMAGE,
            BACKGROUND_TYPE_VIDEO
        };

        QString cacheDir;

        // Player parameters
        QColor      playerLyricsTextBeforeColor;
        QColor      playerLyricsTextAfterColor;
        QFont       playerLyricsFont;
        bool        playerLyricsTextEachCharacter;
        bool        convertMidiFiles;
        bool        playerCDGbackgroundTransparent;
        int         playerRenderFPS;
        int         playerMusicLyricDelay;      // delay in milliseconds between lyrics and music for this hardware


        // Queue parameters
        bool        queueAddNewSingersNext; // if true, new singers are added right after the current singer; otherwise at the end.
        QString     queueFilename;  // if defined, we save queue into file


        // Player background
        BackgroundType  playerBackgroundType;
        QColor          playerBackgroundColor;
        QStringList     playerBackgroundObjects;    // images or videos
        unsigned int    playerBackgroundTransitionDelay; // images only, in seconds. 0 - no transitions/slideshow

        unsigned int    m_playerBackgroundLastObject; // last shown image, or last played video
        qint64          m_playerBackgroundLastVideoTime;  // for video only
        QStringList     m_playerBackgroundObjects;  // actual objects

        // Songs database
        QString         songdbFilename;
        QString         songPathPrefix;

        // LIRC path
        QString         lircDevicePath;
        QString         lircMappingFile;

        // HTTP port
        unsigned int    httpListenPort;
        QString         httpDocumentRoot;

        // Custom background for notifications
        QString         customBackground;

    public:
        void    load();
        void    save();

    private:
        void    loadBackgroundObjects();
};

extern Settings * pSettings;


#endif // SETTINGS_H
