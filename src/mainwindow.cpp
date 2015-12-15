#include <unistd.h>

#include <QDebug>
#include <QLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialog>

#include "mainwindow.h"
#include "playerwidget.h"
#include "karaokefile.h"
#include "songqueue.h"
#include "eventcontroller.h"
#include "playernotification.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), Ui::MainWindow()
{
    setupUi( this );

    pController = new EventController();
    pSongQueue = new SongQueue( this );
    pNotification = new PlayerNotification( this );

    connect( pSongQueue, SIGNAL(queueChanged()), pNotification, SLOT(queueUpdated()) );

    m_karfile = 0;
    m_widget = 0;

    m_widgetStack = new QStackedWidget();
    setCentralWidget( m_widgetStack );

    // Lyrics window
    m_widget = new PlayerWidget( this );
    m_widgetStack->addWidget( m_widget );
    m_widgetStack->setCurrentWidget( m_widget );
    m_widget->show();

    connect( pController, SIGNAL(playerStop()), this, SLOT(queueStop()) );
    connect( pController, SIGNAL(queueAdd(QString,QString)), this, SLOT(queueAdd(QString,QString)) );
    connect( pController, SIGNAL(queueNext()), this, SLOT(queueNext()) );
    connect( pController, SIGNAL(queuePrevious()), this, SLOT(queuePrevious()) );
}

MainWindow::~MainWindow()
{
    delete m_karfile;
}

void MainWindow::queueAdd(QString file, QString singer)
{
    pSongQueue->addSong( file, singer );

    if ( !m_karfile )
        playCurrentItem();
}

void MainWindow::queueStop()
{
    if ( !m_karfile )
        return;

    m_karfile->stop();
    delete m_karfile;
    m_karfile = 0;
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
    if ( m_karfile && m_karfile->position() > 10000 )
    {
        m_karfile->seekBackward();
        return;
    }

    if ( pSongQueue->prev() )
    {
        queueStop();
        playCurrentItem();
    }
}

void MainWindow::playCurrentItem()
{
    if ( pSongQueue->isEmpty() )
        return;

    SongQueue::Song current = pSongQueue->current();

    m_karfile = new KaraokeFile( m_widget );

    try
    {
        if ( !m_karfile->open( current.file ) )
            return;
    }
    catch ( const QString& ex )
    {
        QMessageBox::critical( 0,
                               "Cannot play file",
                               tr("Cannot play file %1:\n%2") .arg( current.file ) .arg( ex ) );
        delete m_karfile;
        m_karfile = 0;
        return;
    }

    m_karfile->start();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    pController->keyEvent( event );
}
