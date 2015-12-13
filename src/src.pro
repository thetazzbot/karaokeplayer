#-------------------------------------------------
#
# Project created by QtCreator 2015-11-07T23:19:59
#
#-------------------------------------------------

QT       += core gui multimedia widgets

TARGET = karaokeplayer
TEMPLATE = app

CONFIG += link_pkgconfig
PKGCONFIG += libavformat libavcodec libswscale  libavresample libavutil

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
    audioplayer_ffmpeg.cpp \
    ffmpeg_headers.cpp \
    playerbackgroundvideo.cpp

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
    audioplayer_ffmpeg.h \
    ffmpeg_headers.h \
    playerbackgroundvideo.h

FORMS    += mainwindow.ui
INCLUDEPATH += ..
LIBS += -L ../libkaraokelyrics -lkaraokelyrics
POST_TARGETDEPS += ../libkaraokelyrics/libkaraokelyrics.a
