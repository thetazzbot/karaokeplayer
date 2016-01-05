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
            BACKGROUND_TYPE_NONE = 2,
            BACKGROUND_TYPE_COLOR = 4,
            BACKGROUND_TYPE_IMAGE = 6,
            BACKGROUND_TYPE_VIDEO = 8
        };

        class Collection
        {
        public:
            Collection();

            // This is used in load/save collection
            Collection( const QString& parsed );
            QString toString();

            enum PathFormat
            {
                PATH_FORMAT_ARTIST_SLASH_TITLE, // artist is the directory name, title is the file name
                PATH_FORMAT_ARTIST_DASH_TITLE,  // artist and title are the file name separated by dash
            };

            // Root path to the directory
            QString     rootPath;

            // Force language (if non -1, the language detection is not performed, and all songs will assume
            // the language specified here. 0 is valid and means no language.
            bool        detectLanguage;

            // Default language (if detection fails or is impossible such as for CD+G or video files)
            int         defaultLanguage;

            // If true, ZIP archives would be scanned. Otherwise they would be ignored.
            bool        scanZips;

            // Collection path format (to detect artist/title from filenames)
            PathFormat  pathFormat;
        };

        QString cacheDir;

        // Player parameters
        QColor      playerLyricsTextBeforeColor;
        QColor      playerLyricsTextAfterColor;
        QFont       playerLyricsFont;
        int         playerLyricsFontMaxSize;
        bool        playerLyricsTextEachCharacter;
        bool        convertMidiFiles;
        bool        playerCDGbackgroundTransparent;
        bool        playerIgnoreBackgroundFromFormats;
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


        // Songs database
        QString         songdbFilename;
        QList<Collection>   songCollection;

        // LIRC path
        QString         lircDevicePath;
        QString         lircMappingFile;

        // HTTP port
        unsigned int    httpListenPort;
        QString         httpDocumentRoot;

        // If true, we should start in a full screen mode (but this doesn't mean NOW is a fullscreen mode)
        bool            startInFullscreen;

        // Replaces the database path according to rules
        QString         replacePath( const QString& origpath );

    public:
        // Load and save settings
        void            load();
        void            save();

    private:
        QString         m_appDataPath;

        // If the song path replacement is set (songPathReplacementFrom is not empty), this would replace '^from' to '^to'
        QString         songPathReplacementFrom;
        QString         songPathReplacementTo;
};

extern Settings * pSettings;


#endif // SETTINGS_H
