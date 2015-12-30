/**************************************************************************
 *  Ulduzsoft Karaoke PLayer - cross-platform desktop karaoke player      *
 *  Copyright (C) 2015-2016 George Yunaev, support@ulduzsoft.com          *
 *                                                                        *
 *  This program is free software: you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation, either version 3 of the License, or     *
 *  (at your option) any later version.                                   *
 *																	      *
 *  This program is distributed in the hope that it will be useful,       *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *  GNU General Public License for more details.                          *
 *                                                                        *
 *  You should have received a copy of the GNU General Public License     *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 **************************************************************************/

#ifndef UTIL_H
#define UTIL_H

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QTextCodec>
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

        static QTextCodec * detectEncoding( const QByteArray& data );
        static QString convertEncoding( const QByteArray& data );

    private:
        Util();
};

#endif // UTIL_H
