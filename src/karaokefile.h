#ifndef KARAOKEFILE_H
#define KARAOKEFILE_H

#include <QIODevice>
#include <QProcess>

#include "playerlyrics.h"
#include "playerbackground.h"


// Represents a playable Karaoke file. It may be one of the following:
// - Two files (music + lyrics of proper format);
// - One archive file (such as ZIP) containing the above;
// - A compound file (such as KFN) containing the above;
class KaraokeFile : public QObject
{
    Q_OBJECT

    public:
        KaraokeFile();
        ~KaraokeFile();

        static bool isValid( const QString& filename );

        // open - a music file, a lyrics file, a combined file (KFN) or a ZIP archive
        bool    open( const QString& filename );

        // Accessors
        QIODevice * musicFile() const { return m_musicFile; }
        PlayerLyrics * lyrics() const { return m_lyrics; }
        PlayerBackground * background() const { return m_background; }

        // Does it need conversion before the music file could be played?
        bool    needsConversion() const { return !m_cachedFileName.isEmpty(); }

        // Starts the conversion
        void    startConversion();

    signals:
        void    conversionFinished( int code );

    private slots:
        void	convError( QProcess::ProcessError error );
        void	convFinished( int exitCode, QProcess::ExitStatus exitStatus );

    protected:
        QIODevice        *   m_musicFile;
        PlayerLyrics     *   m_lyrics;
        PlayerBackground *   m_background;

        // For conversion
        QString              m_musicFileName;
        QString              m_cachedFileName;
        QProcess         *   m_convProcess;

        static bool isMidiFile( const QString& filename );
        static bool isSupportedMusicFile( const QString& filename );
        static bool isSupportedLyricFile( const QString& filename );
};

#endif // KARAOKEFILE_H
