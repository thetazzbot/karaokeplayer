#ifndef KARAOKEOBJECT_H
#define KARAOKEOBJECT_H

#include <QString>
#include <QIODevice>

// Base class encapsulating various Karaoke song objects such as:
//
// - Regular on-disk files
// - Archives (such as ZIP) - not just MP3+G but also Ultrastar etc
// - Complex formats such as KFN
//
class KaraokePlayable
{
    public:
        virtual ~KaraokePlayable();

        // baseFile is a file which was enqueued. It may be music file (we need to find lyrics), lyrics file (we need to find music)
        // or a compound file (we need to find both)
        static KaraokePlayable * create( const QString& baseFile );

        // Finds matching music and lyrics. Returns true if both are found, false otherwise
        bool        parse();

        // Music and lyric object names; must both be set (maybe to the same value) if parse() returned true
        QString     musicObject() const;
        QString     lyricObject() const;

        // Opens the object as QIODevice, which the caller must delete after it is not used anymore.
        QIODevice * openObject( const QString& object );

        // Must be reimplemented in subclass.
        // Should return true if the file is a compound object (i.e. ZIP), i.e. enumerate returns files IN the object, not NEXT to it
        virtual bool        isCompound() const = 0;

        // Must be reimplemented in subclass.
        // Should return full absolute path if the object is on file system. Otherwise an empty string; DO NOT EXTRACT here.
        // Should return the path even if it is not valid/file does not exist.
        // Basically for base "base/file/foo" and "baz" object should return "base/file/baz"
        virtual QString     absolutePath( const QString& object ) = 0;

        // Must be reimplemented in subclass.
        // Should extract the object into the out QIODevice; returns true if succeed, false on error. THis will only be called if
        // absolutePath() returned an empty string.
        virtual bool        extract( const QString& object,  QIODevice *out ) = 0;

        // Karaoke file type support
        static bool isSupportedMusicFile( const QString& filename );
        static bool isSupportedLyricFile( const QString& filename );
        static bool isMidiFile( const QString& filename );
        static bool isVideoFile( const QString& filename );

    protected:
        KaraokePlayable();

        // Must be reimplemented in subclass.
        // Initializes the object
        virtual bool  init() = 0;

        // Must be reimplemented in subclass.
        // Returns a list of files in the archive/next to the file (same directory); empty array in case of error
        virtual QStringList enumerate() = 0;


        // Original file
        QString     m_baseFile;

        // Objects
        QString     m_musicObject;
        QString     m_lyricObject;

        QString     m_errorMsg;
};

#endif // KARAOKEOBJECT_H
