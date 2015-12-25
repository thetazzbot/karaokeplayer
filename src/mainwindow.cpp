#include <unistd.h>

#include <QDebug>
#include <QLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialog>
#include <QTimer>

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


MainWindow * pMainWindow;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), Ui::MainWindow()
{
    setupUi( this );
    pMainWindow = this;

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

    m_widgetStack = new QStackedWidget();
    setCentralWidget( m_widgetStack );

    // Lyrics window should be created
    m_widget = new KaraokeWidget( this );
    m_widgetStack->addWidget( m_widget );
    m_widgetStack->setCurrentWidget( m_widget );
    m_widget->show();

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
}

MainWindow::~MainWindow()
{
}

void MainWindow::queueAdd(QString file, QString singer, int id)
{
    pSongQueue->addSong( file, singer, id );

    if ( m_widget->position() == -1 )
        playCurrentItem();
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
    if ( m_widget->position() > 10000 )
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
    if ( m_widget->position() == -1 )
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
