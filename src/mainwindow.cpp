#include <unistd.h>

#include <QLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialog>

#include "mainwindow.h"
#include "playerwidget.h"
#include "karaokefile.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), Ui::MainWindow()
{
    setupUi( this );

    connect( &m_player, SIGNAL(musicEnded()), this, SLOT(playEnded()) );

    m_karfile = 0;
    m_widget = 0;

    m_widgetStack = new QStackedWidget();
    setCentralWidget( m_widgetStack );

    menuOpenFile();
}

MainWindow::~MainWindow()
{
}

void MainWindow::menuOpenFile()
{
    QString file = QFileDialog::getOpenFileName( 0, "Music file", "/home/tim/work/my/karaokeplayer/test/", "*.*");

    if ( file.isEmpty() )
        exit( 1 );

    m_karfile = new KaraokeFile();

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

    // Load the lyrics
    m_widget = new PlayerWidget( m_karfile->lyrics(), m_karfile->background(), this );
    connect( &m_player, SIGNAL(musicTick(qint64)), m_widget, SLOT(updateLyrics(qint64)) );
    m_widgetStack->addWidget( m_widget );
    m_widgetStack->setCurrentWidget( m_widget );
    m_widget->show();

    // Load the music if no conversion is needed
    if ( !m_karfile->needsConversion() )
    {
        if ( !m_player.load( m_karfile->musicFile() ) )
        {
            QMessageBox::critical( 0,
                                   "Cannot play file",
                                   tr("Cannot play file %1:\n%2") .arg( file ) .arg( m_player.errorMsg() ) );
            return;
        }

        m_player.start();
    }
    else
    {
        m_widget->showCustomText( "Conversion in progress" );
        connect( m_karfile, SIGNAL(conversionFinished(int)), this, SLOT(conversionFinished(int)) );
        m_karfile->startConversion();
    }

}

void MainWindow::playEnded()
{
    m_widgetStack->removeWidget( m_widget );
    m_widget->close();
    delete m_widget;
    m_widget = 0;

    menuOpenFile();
}

void MainWindow::conversionFinished(int code)
{
    if ( code != 0 )
    {
        QMessageBox::critical( 0,
                               "Conversion failed",
                               tr("Conversion failed") );
        delete m_karfile;
        m_karfile = 0;
        return;

    }

    if ( !m_player.load( m_karfile->musicFile() ) )
    {
        QMessageBox::critical( 0,
                               "Cannot play file",
                               tr("Cannot play converted file:\n%2") .arg( m_player.errorMsg() ) );
        return;
    }

    m_widget->showCustomText("");
    m_player.start();
}
