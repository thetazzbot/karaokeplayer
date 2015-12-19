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
#include "eventcontroller.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), Ui::MainWindow()
{
    setupUi( this );

    pSettings = new Settings();
    pController = new EventController();
    pSongQueue = new SongQueue( this );

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

    m_widget->startKaraoke( m_karfile );
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    pController->keyEvent( event );
}
