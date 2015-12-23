#ifndef UTIL_H
#define UTIL_H

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QCryptographicHash>

#include "settings.h"

class Util
{
    public:
        static inline int indexOfSpace( const QString& str )
        {
            for ( int i = 0; i < str.length(); i++ )
                if ( str[i].isSpace() )
                    return i;

            return -1;
        }

        static QString sha512File( const QString& filename )
        {
            QCryptographicHash hash( QCryptographicHash::Sha3_512 );
            QFile file( filename );

            if ( !file.open( QIODevice::ReadOnly ) )
                return QString();

            hash.addData( &file );
            return hash.result().toHex();
        }

        static QString cachedFile( const QString& filename )
        {
            // Cached file could be either file next to music file with .wav extension or a file in cache directory
            QFileInfo finfo( filename );

            QString test = finfo.absolutePath() + QDir::separator() + finfo.completeBaseName() + ".wav";

            if ( QFile::exists( test ) )
                return test;

            // Try the hashed version
            QString hash = sha512File( filename );

            if ( hash.isEmpty() )
                return QString();

            return pSettings->cacheDir + QDir::separator() + hash + ".wav";
        }

    private:
        Util();
};

#endif // UTIL_H
