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

#include <unistd.h>

#include <QDebug>
#include <QLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialog>
#include <QTimer>
#include <QDesktopWidget>

#include "settings.h"
#include "mainwindow.h"
#include "karaokewidget.h"
#include "karaokesong.h"
#include "songqueue.h"
#include "songdatabase.h"
#include "playernotification.h"
#include "actionhandler.h"
#include "currentstate.h"
#include "convertermidi.h"
#include "version.h"
#include "playerwidget.h"
#include "ui_dialog_about.h"


MainWindow * pMainWindow;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), Ui::MainWindow()
{
    setupUi( this );
    pMainWindow = this;
    m_playerWindow  = 0;

    // We don't need any specific crypto
    qsrand( (unsigned int) (long) pMainWindow * (unsigned int) time(0) );

    // Settings should be created first
    pSettings = new Settings();

    // Current state doesn't connect to anything
    pCurrentState = new CurrentState( this );

    // Controller the second
    pActionHandler = new ActionHandler();

    // Then notification
    pNotification = new PlayerNotification( 0 );

    // Song database
    pSongDatabase = new SongDatabase( this );

    // MIDI converter
    pConverterMIDI = new ConverterMIDI( this );

    // Lyrics window should be created depending on the mode we use
    m_widget = new KaraokeWidget( this );
    setCentralWidget( m_widget );

    if ( pCurrentState->modeFullscreen )
        menuToggleFullscreen();

    // Song queue should be created last, as it emits signals intercepted by other modules
    pSongQueue = new SongQueue( this );
    connect( pSongQueue, SIGNAL(queueChanged()), pNotification, SLOT(queueUpdated()) );

    // This may load the queue
    pSongQueue->init();

    // Load the song database
    pSongDatabase->init();

    // Initialize the controller
    pActionHandler->start();

    // Init the notification
    pNotification->showStopped();

    // Connect slots
    connect( pActionHandler, SIGNAL(playerStart()), this, SLOT(queueStart()) );
    connect( pActionHandler, SIGNAL(playerStop()), this, SLOT(queueStop()) );
    connect( pActionHandler, SIGNAL(queueAdd(QString,QString,int)), this, SLOT(queueAdd(QString,QString,int)) );
    connect( pActionHandler, SIGNAL(queueNext()), this, SLOT(queueNext()) );
    connect( pActionHandler, SIGNAL(queuePrevious()), this, SLOT(queuePrevious()) );
    connect( pActionHandler, SIGNAL(queueClear()), pSongQueue, SLOT(clear()) );

    connect( pConverterMIDI, SIGNAL(finished(QString,bool)), pSongQueue, SLOT(processingFinished(QString,bool)) );

    // Connect menu actions
    connect( action_About, SIGNAL(triggered()), this, SLOT(menuAbout()) );
    connect( actionPlay_file, SIGNAL(triggered()), this, SLOT(menuOpenKaraoke()) );
    connect( actionSettings, SIGNAL(triggered()), this, SLOT(menuSettings()) );
    connect( action_Player_window, SIGNAL(triggered()), this, SLOT(menuToggleWindowPlayer()) );
    connect( action_Quit, SIGNAL(triggered()), qApp, SLOT(quit()) );

    // Currently unused
    mainToolBar->hide();
}

MainWindow::~MainWindow()
{
}

void MainWindow::queueAdd(QString file, QString singer, int id)
{
    pSongQueue->addSong( file, singer, id );
    queueStart();
}

void MainWindow::queueStop()
{
    m_widget->stopKaraoke();
    pNotification->showStopped();
}

void MainWindow::queueNext()
{
    if ( pSongQueue->next() )
    {
        queueStop();
        playCurrentItem();
    }
}

void MainWindow::queuePrevious()
{
    if ( pCurrentState->playerState != CurrentState::PLAYERSTATE_STOPPED && pCurrentState->playerPosition > 10000 )
    {
        pActionHandler->cmdAction( ActionHandler::ACTION_PLAYER_SEEK_BACK );
        return;
    }

    if ( pSongQueue->prev() )
    {
        queueStop();
        playCurrentItem();
    }
}

void MainWindow::queueStart()
{
    if ( pCurrentState->playerState == CurrentState::PLAYERSTATE_STOPPED )
        playCurrentItem();
}

void MainWindow::playCurrentItem()
{
    if ( pSongQueue->isEmpty() )
        return;

    SongQueue::Song current = pSongQueue->current();

    // If current song is not ready yet, show the notification and wait until it is
    if ( current.state == SongQueue::Song::STATE_PREPARING )
    {
        pNotification->setOnScreenMessage( tr("Conversion in progress") );
        QTimer::singleShot( 500, this, SLOT( playCurrentItem() ) );
        return;
    }

    pNotification->clearOnScreenMessage();
    KaraokeSong * karfile = new KaraokeSong( m_widget, current );

    try
    {
        if ( !karfile->open() )
        {
            delete karfile;
            return;
        }
    }
    catch ( const QString& ex )
    {
        QMessageBox::critical( 0,
                               "Cannot play file",
                               tr("Cannot play file %1:\n%2") .arg( current.file ) .arg( ex ) );
        delete karfile;
        return;
    }

    m_widget->startKaraoke( karfile );
}

void MainWindow::menuOpenKaraoke()
{
    QString file = QFileDialog::getOpenFileName( 0, "Music file", "/home/tim/work/my/karaokeplayer/test/", "*.*");

    if ( file.isEmpty() )
        return;

    queueAdd( file, "Console", 0 );
}

void MainWindow::menuSettings()
{

}

void MainWindow::menuToggleWindowPlayer()
{
    if ( m_playerWindow )
    {
        delete m_playerWindow;
        m_playerWindow = 0;
    }
    else
    {
        m_playerWindow = new PlayerWidget( this );
        connect( m_playerWindow, SIGNAL(destroyed(QObject*)), this, SLOT(dockWindowClosed(QObject*)) );
        m_playerWindow->show();
    }
}

void MainWindow::menuAbout()
{
    QDialog dlg;
    Ui::DialogAbout ui_about;

    ui_about.setupUi( &dlg );

    ui_about.labelAbout->setText( tr("<b>Ulduzsoft Karaoke Player version %1.%2</b><br><br>"
            "Copyright (C) George Yunaev 2015-2016, <a href=\"mailto:support@ulduzsoft.com\">support@ulduzsoft.com</a><br><br>"
            "Web site: <a href=\"http://www.ulduzsoft.com\">www.ulduzsoft.com/karplayer</a><br><br>"
            "This program is licensed under terms of GNU General Public License "
            "version 3; see LICENSE file for details.") .arg(APP_VERSION_MAJOR) .arg(APP_VERSION_MINOR) );

    dlg.exec();
}

void MainWindow::menuToggleFullscreen()
{
    if ( isFullScreen() )
    {
        QApplication::restoreOverrideCursor();
        mainMenuBar->show();
        showNormal();
    }
    else
    {
        mainMenuBar->hide();
        showFullScreen();

        // In case we run without screen manager
        move( 0, 0 );
        resize( QApplication::desktop()->size() );

        // Hide the mouse cursor
        QApplication::setOverrideCursor( Qt::BlankCursor );
    }
}


void MainWindow::dockWindowClosed(QObject *widget)
{
    if ( widget == m_playerWindow )
    {
        // Player dock window closed, and is already destroyed (no need to delete it)
        m_playerWindow = 0;
        action_Player_window->setChecked( false );
    }
    else
        abort();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    pActionHandler->keyEvent( event );
}

void MainWindow::closeEvent(QCloseEvent *)
{
    m_widget->stopEverything();

    pActionHandler->stop();
    pSettings->save();
}
