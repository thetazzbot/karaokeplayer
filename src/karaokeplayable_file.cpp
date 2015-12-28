#include <QDir>
#include <QFileInfo>
#include <QStringList>

#include "karaokeplayable_file.h"

KaraokePlayable_File::KaraokePlayable_File()
    : KaraokePlayable()
{
}

bool KaraokePlayable_File::init()
{
    return true;
}

bool KaraokePlayable_File::isCompound() const
{
    return false;
}

QStringList KaraokePlayable_File::enumerate()
{
    QFileInfo finfo( m_baseFile );
    QStringList files;

    // Enumerate all the files in the file's directory with the same name and .* extension
    QFileInfoList list = finfo.dir().entryInfoList( QStringList() << (finfo.baseName() + ".*"), QDir::Files | QDir::NoDotAndDotDot );

    foreach ( QFileInfo fi, list )
    {
        // Skip current file
        if ( fi == finfo )
            continue;

        files.push_back( fi.fileName() );
    }

    return files;
}

QString KaraokePlayable_File::absolutePath( const QString &object )
{
    if ( QFileInfo( object ).isAbsolute() )
        return object;

    // Convert to absolute path
    QFileInfo finfo( m_baseFile );

    return finfo.absoluteDir().absolutePath() + QDir::separator() + object;
}

bool KaraokePlayable_File::extract(const QString &, QIODevice *)
{
    // This one should never be called; if it is, the usage is invalid
    abort();
}
