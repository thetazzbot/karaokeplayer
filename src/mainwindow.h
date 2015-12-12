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
// - Video background
// - LIRC support
// - Queue
// - Song collection
// - Per-user queue w/intelligent
// - Remote song storage, download (incl encrypted)
// - Voice passthrough and recording
// - Video + voice recording
// - Remote management through Web site and Android app



#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>

#include "ui_mainwindow.h"
#include "player.h"

class PlayerWidget;
class KaraokeFile;

class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

    public slots:
        void    menuOpenFile();
        void    playEnded();

    private slots:
        void    conversionFinished(int code);

    private:
        KaraokeFile  *  m_karfile;
        Player          m_player;
        PlayerWidget *  m_widget;
        QStackedWidget* m_widgetStack;
};

#endif // MAINWINDOW_H
