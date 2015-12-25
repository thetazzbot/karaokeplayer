#-------------------------------------------------
#
# Project created by QtCreator 2015-11-07T23:19:59
#
#-------------------------------------------------

QT       += core gui multimedia widgets dbus network

TARGET = karaokeplayer
TEMPLATE = app

#CONFIG += link_pkgconfig
#PKGCONFIG += libavformat libavcodec libswscale  libavresample libavutil

SOURCES += main.cpp\
        mainwindow.cpp \
    playerbackground.cpp \
    playerbackgroundcolor.cpp \
    settings.cpp \
    playerlyrics.cpp \
    playerlyricscdg.cpp \
    player.cpp \
    songqueue.cpp \
    songdatabase.cpp \
    playerlyricstext.cpp \
    playerlyrictext_line.cpp \
    karaokepainter.cpp \
    playerbackgroundvideo.cpp \
    eventcontroller.cpp \
    eventcontroller_dbus.cpp \
    songenqueuedialog.cpp \
    playernotification.cpp \
    playerrenderer.cpp \
    songsearchdialog.cpp \
    eventcontroller_lirc.cpp \
    eventcontroller_webserver.cpp \
    eventcontroller_webserver_handler.cpp \
    logger.cpp \
    playerbackgroundimage.cpp \
    convertermidi.cpp \
    karaokewidget.cpp \
    karaokesong.cpp

HEADERS  += mainwindow.h \
    playerbackground.h \
    playerbackgroundcolor.h \
    settings.h \
    playerlyrics.h \
    playerlyricscdg.h \
    player.h \
    songqueue.h \
    songdatabase.h \
    playerlyricstext.h \
    playerlyrictext_line.h \
    karaokepainter.h \
    playerbackgroundvideo.h \
    eventcontroller.h \
    eventcontroller_dbus.h \
    songenqueuedialog.h \
    playernotification.h \
    playerrenderer.h \
    songsearchdialog.h \
    eventcontroller_lirc.h \
    eventcontroller_webserver.h \
    eventcontroller_webserver_handler.h \
    logger.h \
    util.h \
    playerbackgroundimage.h \
    convertermidi.h \
    karaokewidget.h \
    karaokesong.h

FORMS    += mainwindow.ui \
    songenqueuedialog.ui \
    songsearchdialog.ui
INCLUDEPATH += .. ../libqhttpengine/QHttpEngine ../libqhttpengine
LIBS += -L ../libkaraokelyrics/ -lkaraokelyrics -L../libqhttpengine/ -lqhttpengine -lsqlite3
POST_TARGETDEPS += ../libkaraokelyrics/libkaraokelyrics.a ../libqhttpengine/libqhttpengine.a
DEFINES += SQLITE_OMIT_LOAD_EXTENSION
