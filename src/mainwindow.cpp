#include <unistd.h>

#include <QDebug>
#include <QLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialog>

#include "mainwindow.h"
#include "playerwidget.h"
#include "karaokefile.h"
#include "eventcontroller.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), Ui::MainWindow()
{
    setupUi( this );

    pController = new EventController();

    m_karfile = 0;
    m_widget = 0;

    m_widgetStack = new QStackedWidget();
    setCentralWidget( m_widgetStack );

    // Lyrics window
    m_widget = new PlayerWidget( this );
    m_widgetStack->addWidget( m_widget );
    m_widgetStack->setCurrentWidget( m_widget );
    m_widget->show();

    menuOpenFile();
}

MainWindow::~MainWindow()
{
    delete m_karfile;
}

void MainWindow::menuOpenFile()
{
    QString file = QFileDialog::getOpenFileName( 0, "Music file", "/home/tim/work/my/karaokeplayer/test/", "*.*");

    if ( file.isEmpty() )
        exit( 1 );

    m_karfile = new KaraokeFile( m_widget );

    try
    {
        if ( !m_karfile->open( file ) )
            return;
    }
    catch ( const QString& ex )
    {
        QMessageBox::critical( 0,
                               "Cannot play file",
                               tr("Cannot play file %1:\n%2") .arg( file ) .arg( ex ) );
        delete m_karfile;
        m_karfile = 0;
        return;
    }

    m_karfile->start();
}

void MainWindow::playEnded()
{
    delete m_karfile;
    m_karfile = 0;

    menuOpenFile();
}

void MainWindow::cmdStop()
{
    if ( m_karfile )
    {
        delete m_karfile;
        m_karfile = 0;
    }
}

void MainWindow::cmdPause()
{
    m_karfile->pause();
}

void MainWindow::cmdQueuePrevious()
{

}

void MainWindow::cmdQueueNext()
{

}

void MainWindow::cmdSeekForward()
{

}

void MainWindow::cmdSeekBackward()
{

}
