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
    settings.cpp \
    playerlyrics.cpp \
    playerlyricscdg.cpp \
    songqueue.cpp \
    songdatabase.cpp \
    playerlyricstext.cpp \
    playerlyrictext_line.cpp \
    karaokepainter.cpp \
    playernotification.cpp \
    playerrenderer.cpp \
    songsearchdialog.cpp \
    logger.cpp \
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
    karaokeplayable.cpp \
    karaokeplayable_file.cpp \
    karaokeplayable_zip.cpp \
    karaokeplayable_kfn.cpp \
    util.cpp \
    songdatabasescanner.cpp \
    languagedetector.cpp \
    background.cpp \
    backgroundcolor.cpp \
    backgroundimage.cpp \
    backgroundvideo.cpp \
    settingsdialog.cpp \
    colorbutton.cpp \
    labelshowhelp.cpp

HEADERS  += mainwindow.h \
    settings.h \
    playerlyrics.h \
    playerlyricscdg.h \
    songqueue.h \
    songdatabase.h \
    playerlyricstext.h \
    playerlyrictext_line.h \
    karaokepainter.h \
    playernotification.h \
    playerrenderer.h \
    songsearchdialog.h \
    logger.h \
    util.h \
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
    karaokeplayable.h \
    karaokeplayable_file.h \
    karaokeplayable_zip.h \
    karaokeplayable_kfn.h \
    aes.h \
    songdatabasescanner.h \
    languagedetector.h \
    background.h \
    backgroundcolor.h \
    backgroundimage.h \
    backgroundvideo.h \
    settingsdialog.h \
    colorbutton.h \
    labelshowhelp.h

FORMS    += mainwindow.ui \
    songsearchdialog.ui \
    playerwidget.ui \
    dialog_about.ui \
    settingsdialog.ui
RESOURCES += resources.qrc
INCLUDEPATH += .. ../libqhttpengine/QHttpEngine ../libqhttpengine
LIBS += -L ../libkaraokelyrics/ -lkaraokelyrics -L../libqhttpengine/ -lqhttpengine -L../libuchardet/ -luchardet -lsqlite3 -lzip
POST_TARGETDEPS += ../libkaraokelyrics/libkaraokelyrics.a ../libqhttpengine/libqhttpengine.a ../libuchardet/libuchardet.a
DEFINES += SQLITE_OMIT_LOAD_EXTENSION
