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

#include <QDir>
#include <QFileInfo>
#include <QStringList>

#include "karaokeplayable_file.h"

KaraokePlayable_File::KaraokePlayable_File(const QString &baseFile)
    : KaraokePlayable( baseFile )
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
