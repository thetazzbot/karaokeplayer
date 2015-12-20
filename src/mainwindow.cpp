#include <unistd.h>

#include <QDebug>
#include <QLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialog>

#include "settings.h"
#include "mainwindow.h"
#include "playerwidget.h"
#include "karaokefile.h"
#include "songqueue.h"
#include "songdatabase.h"
#include "playernotification.h"
#include "eventcontroller.h"
#include "webserver.h"


MainWindow * pMainWindow;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), Ui::MainWindow()
{
    setupUi( this );

    // Settings should be created first
    pSettings = new Settings();

    // Controller the second
    pController = new EventController();

    // Then notification
    pNotification = new PlayerNotification( 0 );

    // Song database
    pSongDatabase = new SongDatabase();

    m_widgetStack = new QStackedWidget();
    setCentralWidget( m_widgetStack );

    // Lyrics window should be created
    m_widget = new PlayerWidget( this );
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

    // Launch the web server
    pWebServer = new WebServer();
    pWebServer->start();

    connect( pController, SIGNAL(playerStart()), this, SLOT(queueStart()) );
    connect( pController, SIGNAL(playerStop()), this, SLOT(queueStop()) );
    connect( pController, SIGNAL(queueAdd(QString,QString)), this, SLOT(queueAdd(QString,QString)) );
    connect( pController, SIGNAL(queueNext()), this, SLOT(queueNext()) );
    connect( pController, SIGNAL(queuePrevious()), this, SLOT(queuePrevious()) );
    connect( pController, SIGNAL(queueClear()), pSongQueue, SLOT(clear()) );
}

MainWindow::~MainWindow()
{
}

void MainWindow::queueAdd(QString file, QString singer)
{
    pSongQueue->addSong( file, singer );

    if ( m_widget->position() == -1 )
        playCurrentItem();
}

void MainWindow::queueStop()
{
    m_widget->stopKaraoke();
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
        pController->cmdEvent( EventController::EVENT_PLAYER_BACKWARD );
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

    KaraokeFile * karfile = new KaraokeFile( m_widget );

    try
    {
        if ( !karfile->open( current.file ) )
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
    pController->keyEvent( event );
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_widget->stopKaraoke();
}
