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

    KaraokeFile * f = new KaraokeFile();

    try
    {
        if ( !f->open( file ) )
            return;
    }
    catch ( const QString& ex )
    {
        QMessageBox::critical( 0,
                               "Cannot play file",
                               tr("Cannot play file %1:\n%2") .arg( file ) .arg( ex ) );
        return;
    }

    // Load the music
    if ( !m_player.load( f->musicFile() ) )
    {
        QMessageBox::critical( 0,
                               "Cannot play file",
                               tr("Cannot play file %1:\n%2") .arg( file ) .arg( m_player.errorMsg() ) );
        return;
    }

    // Load the lyrics
    m_widget = new PlayerWidget( f->lyrics(), f->background(), this );
    connect( &m_player, SIGNAL(musicTick(qint64)), m_widget, SLOT(updateLyrics(qint64)) );
    m_widgetStack->addWidget( m_widget );
    m_widgetStack->setCurrentWidget( m_widget );
    m_widget->show();

    m_player.start();
}

void MainWindow::playEnded()
{
    m_widgetStack->removeWidget( m_widget );
    m_widget->close();
    delete m_widget;
    m_widget = 0;

    menuOpenFile();
}
