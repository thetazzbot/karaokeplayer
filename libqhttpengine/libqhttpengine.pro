SOURCES += qfilesystemhandler.cpp  qhttphandler.cpp  qhttpparser.cpp  qhttpserver.cpp  qhttpsocket.cpp  qibytearray.cpp  qiodevicecopier.cpp  qlocalfile.cpp  qobjecthandler.cpp
TARGET = qhttpengine
CONFIG += warn_on qt staticlib
QT += network
TEMPLATE = lib

HEADERS += \
    qhttpengine.h \
    qhttphandler_p.h \
    qhttpserver_p.h \
    qhttpsocket_p.h \
    qiodevicecopier_p.h \
    qfilesystemhandler_p.h \
    qlocalfile_p.h \
    qobjecthandler_p.h \
    QHttpEngine/QFilesystemHandler \
    QHttpEngine/QHttpHandler \
    QHttpEngine/qhttphandler.h \
    QHttpEngine/QHttpParser \
    QHttpEngine/qhttpparser.h \
    QHttpEngine/QHttpServer \
    QHttpEngine/qhttpserver.h \
    QHttpEngine/QHttpSocket \
    QHttpEngine/qhttpsocket.h \
    QHttpEngine/QIByteArray \
    QHttpEngine/qibytearray.h \
    QHttpEngine/QIODeviceCopier \
    QHttpEngine/qiodevicecopier.h \
    QHttpEngine/QLocalFile \
    QHttpEngine/qlocalfile.h \
    QHttpEngine/QObjectHandler \
    QHttpEngine/qfilesystemhandler.h \
    QHttpEngine/qobjecthandler.h
