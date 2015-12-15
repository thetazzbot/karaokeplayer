#-------------------------------------------------
#
# Project created by QtCreator 2015-11-07T23:19:59
#
#-------------------------------------------------

QT       += core gui multimedia widgets dbus

TARGET = karaokeplayer
TEMPLATE = app

#CONFIG += link_pkgconfig
#PKGCONFIG += libavformat libavcodec libswscale  libavresample libavutil

SOURCES += main.cpp\
        mainwindow.cpp \
    playerwidget.cpp \
    playerbackground.cpp \
    playerbackgroundcolor.cpp \
    settings.cpp \
    playerlyrics.cpp \
    playerlyricscdg.cpp \
    player.cpp \
    songqueue.cpp \
    songdatabase.cpp \
    playerlyricstext.cpp \
    karaokefile.cpp \
    playerlyrictext_line.cpp \
    karaokepainter.cpp \
    playerbackgroundvideo.cpp \
    eventcontroller.cpp \
    eventcontroller_dbus.cpp \
    songenqueuedialog.cpp \
    playernotification.cpp

HEADERS  += mainwindow.h \
    playerwidget.h \
    playerbackground.h \
    playerbackgroundcolor.h \
    settings.h \
    playerlyrics.h \
    playerlyricscdg.h \
    player.h \
    songqueue.h \
    songdatabase.h \
    playerlyricstext.h \
    karaokefile.h \
    playerlyrictext_line.h \
    karaokepainter.h \
    playerbackgroundvideo.h \
    eventcontroller.h \
    eventcontroller_dbus.h \
    songenqueuedialog.h \
    playernotification.h

FORMS    += mainwindow.ui \
    songenqueuedialog.ui
INCLUDEPATH += ..
LIBS += -L ../libkaraokelyrics -lkaraokelyrics
POST_TARGETDEPS += ../libkaraokelyrics/libkaraokelyrics.a
