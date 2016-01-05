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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>

#include "ui_mainwindow.h"

class KaraokeWidget;
class KaraokeSong;
class SongQueue;
class WebServer;
class SettingsDialog;
class PlayerWidget;
class SongDatabaseScanner;

class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

    public slots:
        void    queueAdd( QString singer, int id );
        void    queueAdd( QString singer, QString file );
        void    queueStop();
        void    queueNext();
        void    queuePrevious();
        void    queueStart();
        void    playCurrentItem();

        // Update collection database information
        void    collectionScanUpdate();
        void    collectionScanFinished();

        // Menu actions
        void    menuOpenKaraoke();
        void    menuSettings();
        void    menuToggleWindowPlayer();
        void    menuAbout();
        void    menuToggleFullscreen();
        void    menuPlayPause();
        void    menuRescanDatabase();
        void    menuUpdateDatabase();

        // Handling window closures
        void    windowClosed( QObject* widget );

    private:
        void    keyPressEvent(QKeyEvent * event);
        void    closeEvent(QCloseEvent *);

        bool    hasCmdLineOption( const QString& option );

    private:
        KaraokeWidget *  m_widget;
        PlayerWidget *  m_playerWindow;

        // Only when the scan is in progress
        SongDatabaseScanner *   m_songScanner;

        // TO make sure we only have one Settings dialog
        SettingsDialog      *   m_settings;
};


extern MainWindow * pMainWindow;

#endif // MAINWINDOW_H
