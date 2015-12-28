#ifndef ARCHIVE_ZIP_H
#define ARCHIVE_ZIP_H

#include <QFile>
#include <QIODevice>

struct zip;

// Generic handler (both for karaoke and old KFN) of ZIP archives
// Based on libzip
class ArchiveZip
{
    public:
        ArchiveZip( const QString& zipfilename );

        // Opens an archive file
        bool        open();

        // Error message in case of errors
        QString     errorMsg() const { return m_errorMsg; }

        // List of files in the archive; empty array in case of error
        QStringList enumerate() const;

        // Extracts the file into the out; returns true if succeed, false on error
        bool        extract(const QString& file,  QIODevice *out ) const;

    private:
        mutable QString     m_errorMsg;

        // We pass file handle to libzip
        QFile       m_zipFile;

        struct zip* m_zip;
};

#endif // ARCHIVE_ZIP_H
