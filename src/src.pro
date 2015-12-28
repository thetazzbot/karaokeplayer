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
    songqueue.cpp \
    songdatabase.cpp \
    playerlyricstext.cpp \
    playerlyrictext_line.cpp \
    karaokepainter.cpp \
    playerbackgroundvideo.cpp \
    playernotification.cpp \
    playerrenderer.cpp \
    songsearchdialog.cpp \
    logger.cpp \
    playerbackgroundimage.cpp \
    convertermidi.cpp \
    karaokewidget.cpp \
    karaokesong.cpp \
    playerwidget.cpp \
    playerbutton.cpp \
    currentstate.cpp \
    actionhandler.cpp \
    actionhandler_dbus.cpp \
    actionhandler_lirc.cpp \
    actionhandler_webserver.cpp \
    actionhandler_webserver_handler.cpp \
    universalplayer.cpp \
    archive_zip.cpp \
    karaokeplayable.cpp \
    karaokeplayable_file.cpp \
    karaokeplayable_zip.cpp

HEADERS  += mainwindow.h \
    playerbackground.h \
    playerbackgroundcolor.h \
    settings.h \
    playerlyrics.h \
    playerlyricscdg.h \
    songqueue.h \
    songdatabase.h \
    playerlyricstext.h \
    playerlyrictext_line.h \
    karaokepainter.h \
    playerbackgroundvideo.h \
    playernotification.h \
    playerrenderer.h \
    songsearchdialog.h \
    logger.h \
    util.h \
    playerbackgroundimage.h \
    convertermidi.h \
    karaokewidget.h \
    karaokesong.h \
    playerwidget.h \
    playerbutton.h \
    currentstate.h \
    actionhandler.h \
    actionhandler_dbus.h \
    actionhandler_lirc.h \
    actionhandler_webserver.h \
    actionhandler_webserver_handler.h \
    version.h \
    universalplayer.h \
    archive_zip.h \
    karaokeplayable.h \
    karaokeplayable_file.h \
    karaokeplayable_zip.h

FORMS    += mainwindow.ui \
    songsearchdialog.ui \
    playerwidget.ui \
    dialog_about.ui
RESOURCES += resources.qrc
INCLUDEPATH += .. ../libqhttpengine/QHttpEngine ../libqhttpengine
LIBS += -L ../libkaraokelyrics/ -lkaraokelyrics -L../libqhttpengine/ -lqhttpengine -lsqlite3 -lzip
POST_TARGETDEPS += ../libkaraokelyrics/libkaraokelyrics.a ../libqhttpengine/libqhttpengine.a
DEFINES += SQLITE_OMIT_LOAD_EXTENSION
