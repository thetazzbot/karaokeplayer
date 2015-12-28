#ifndef KARAOKEPLAYABLE_ZIP_H
#define KARAOKEPLAYABLE_ZIP_H

#include <QFile>
#include <QIODevice>

#include "karaokeplayable.h"

struct zip;

// Generic handler (both for karaoke and old KFN) of ZIP archives
class KaraokePlayable_ZIP : public KaraokePlayable
{
    public:
        KaraokePlayable_ZIP();

        // Initializes the object
        bool        init();

        // Should return true if the file is a compound object (i.e. ZIP), i.e. enumerate returns files IN the object, not NEXT to it
        bool        isCompound() const;

        // Returns a list of files in the archive/next to the file (same directory) EXCLUDING the base object;
        // Should return an empty array in case of error
        QStringList enumerate();

        // Should return full absolute path if the object is on file system. Otherwise an empty string; DO NOT EXTRACT here.
        // Should return the path even if it is not valid/file does not exist.
        // Basically for base "base/file/foo" and "baz" object should return "base/file/baz"
        QString     absolutePath( const QString& object );

        // Should extract the object into the out QIODevice; returns true if succeed, false on error. THis will only be called if
        // absolutePath() returned an empty string.
        bool        extract( const QString& object,  QIODevice *out );

    private:
        // We pass file handle to libzip
        QFile       m_zipFile;

        struct zip* m_zip;
};

#endif // KARAOKEPLAYABLE_ZIP_H
