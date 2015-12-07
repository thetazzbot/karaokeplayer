#include "lyricsparser.h"


LyricsParser::LyricsParser()
{
}

LyricsParser::~LyricsParser()
{
}

QStringList LyricsParser::loadText(QIODevice &file)
{
    // Other formats are text files, so we need to find out the encoding
    // FIXME: assuming UTF-8
    QByteArray datalyrics = file.readAll();

    if ( datalyrics.indexOf( '\r') != -1 )
        datalyrics.replace( "\r\n", "\n" );

    return QString::fromUtf8( datalyrics ).split( '\n' );
}

QString LyricsParser::timeAsText(quint64 timing)
{
    int min = timing / 60000;
    int sec = (timing - min * 60000) / 1000;
    int msec = timing - (min * 60000 + sec * 1000 );

    return QString().sprintf( "%02d:%02d.%04d", min, sec, msec );
}
