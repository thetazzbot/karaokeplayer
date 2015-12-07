HEADERS += \
    lyricsparser_kfn.h \
    lyricsparser_lrc.h \
    lyricsparser_midi.h \
    lyricsloader.h \
    lyricsparser.h \
    lyricsparser_texts.h \
    lyricsparser_kok.h
SOURCES += \
    lyricsparser_kfn.cpp \
    lyricsparser_lrc.cpp \
    lyricsparser_midi.cpp \
    lyricsparser_powerkaraoke.cpp \
    lyricsloader.cpp \
    lyricsparser.cpp \
    lyricsparser_texts.cpp \
    lyricsparser_kok.cpp
TARGET = karaokelyrics
CONFIG += warn_on qt staticlib
TEMPLATE = lib
