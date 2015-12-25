//TODO:
// change all timings to int / no qin64

// Features:
//
// Ensure QMediaPlayer etc works on all platforms for:
// - Playing music (all formats)
// - Good timing w/lyrics
// - Pause, seek, rewind
// - Video background
// then
// - Voice passthrough
// - Audio recording
// - Video screen capture/webcam capture
//

// - CDG, KFN, MID, LRC, Ultrastar, ZIP
// - Timidity
// - Remote song storage, download (incl encrypted)
// - Voice passthrough and recording
// - Video + voice recording


// DONE //
// - CDG, KFN, MID, LRC, Ultrastar, ZIP
// - Timidity
// - Video background
// - LIRC support
// - Queue
// - Song collection
// - Per-user queue w/intelligent
// - Remote management through Web site and Android app


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>

#include "ui_mainwindow.h"

class KaraokeWidget;
class KaraokeSong;
class SongQueue;
class WebServer;
class PlayerWidget;

class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

    public slots:
        void    queueAdd( QString file, QString singer, int id );
        void    queueStop();
        void    queueNext();
        void    queuePrevious();
        void    queueStart();
        void    playCurrentItem();

        // Menu actions
        void    menuOpenKaraoke();
        void    menuSettings();
        void    menuToggleWindowPlayer();
        void    menuAbout();

        // Handling dock window closures
        void    dockWindowClosed( QObject* widget );

    private:
        void    keyPressEvent(QKeyEvent * event);
        void    closeEvent(QCloseEvent *);

    private:
        KaraokeWidget *  m_widget;
        QStackedWidget* m_widgetStack;
        PlayerWidget *  m_playerWindow;
};


extern MainWindow * pMainWindow;

#endif // MAINWINDOW_H
