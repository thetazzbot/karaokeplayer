#ifndef LYRICSLOADER_H
#define LYRICSLOADER_H

#include <QIODevice>
#include <QString>
#include <QMap>
#include <QList>


// Contains a single lyric entry as present in the lyrics file
class Lyric
{
    public:
        Lyric( qint64 start_, const QString& text_, qint64 duration_ = -1, int pitch_ = -1 );

    public:
        enum
        {
            PITCH_FREESTYLE = -1,
            PITCH_GOLDEN = -2
        };

        qint64	start;      // Only if not end of block
        qint64	duration;   // If not set by the parser, calculated later
        QString	text;		// May be empty, this indicates end of block - in this case start/duration is not valid
        int		pitch;		// PITCH_FREESTYLE if not set, used for Ultrastar
};


// Public API for karaoke lyrics loader
class LyricsLoader
{
    public:
        enum Property
        {
            PROP_INVALID,
            PROP_TITLE,
            PROP_ARTIST,
            PROP_MUSICFILE,
            PROP_BACKGROUND,
            PROP_VIDEO
        };

        // Lyrics parameters
        typedef QMap< Property, QString > Properties;
        typedef QList<Lyric>   Container;

        LyricsLoader( Properties& properties, Container& container );

        // Parses the lyrics, filling up the output container. Returns true
        // on success, false if there is an rror, see errorMsg() contains reason.
        //
        // It gets the lyrics file name and QIODevice because the file may be extracted (from ZIP or KFN)
        bool parse( const QString& lyricsfile, QIODevice& file );

        static bool    isSupportedFile( const QString& filename );

        static void dump( const Container& output );

        // In case of error this will return the reason
        QString errorMsg() const { return m_errorMsg; }

    protected:
        // Fix the lyrics array:
        // - Calculate duration for each syllable if this information is not available
        void calculateDurations();        

    protected:
        Container&   m_lyricsOutput;
        Properties&  m_props;

        QString     m_errorMsg;        
};

#endif // LYRICSLOADER_H
