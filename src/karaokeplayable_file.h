#ifndef KARAOKEPLAYABLE_FILE_H
#define KARAOKEPLAYABLE_FILE_H

#include <QFileInfo>

#include "karaokeplayable.h"

// Playable karaoke object as local, on-disk file
class KaraokePlayable_File : public KaraokePlayable
{
    public:
        KaraokePlayable_File();

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
        bool extract( const QString& object,  QIODevice *out );
};

#endif // KARAOKEPLAYABLE_FILE_H
