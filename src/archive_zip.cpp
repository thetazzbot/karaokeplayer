#include <QStringList>
#include <QBuffer>

#include "zip.h"
#include "archive_zip.h"


ArchiveZip::ArchiveZip(const QString &zipfilename)
    : m_zipFile( zipfilename )
{
    m_zip = 0;
}

bool ArchiveZip::open()
{
    // We use QFile and zip_fdopen instead of zip_open because latter does not support Unicode file names
    if ( !m_zipFile.open( QIODevice::ReadOnly ) )
    {
        m_errorMsg = m_zipFile.errorString();
        return false;
    }

    // Open the ZIP archive: http://www.nih.at/libzip/zip_fdopen.html
    int errcode;
    m_zip = zip_fdopen( m_zipFile.handle(), 0, &errcode );

    if ( !m_zip  )
    {
        m_errorMsg = QString( "Error %1 opening ZIP archive") .arg( errcode );
        return false;
    }

    return true;
}

QStringList ArchiveZip::enumerate() const
{
    QStringList filelist;

    // http://www.nih.at/libzip/zip_get_num_entries.html
    int files = zip_get_num_entries( m_zip, ZIP_FL_UNCHANGED );

    if ( files == 0 )
    {
        m_errorMsg = "archive has no files";
        return filelist;
    }

    for ( int i = 0; i < files; i++ )
    {
        // Retrieve the file size
        struct zip_stat fileinfo;

        // http://www.nih.at/libzip/zip_stat_index.html
        if ( zip_stat_index( m_zip, i, 0, &fileinfo) != 0 )
        {
            m_errorMsg = "file info not found";
            return QStringList();
        }

        filelist.push_back( QString::fromUtf8( fileinfo.name ) );
    }

    return filelist;
}
/*
QByteArray ArchiveZip::extract(const QString &file) const
{
    QBuffer buf;

    if ( extract( file, &buf) )
        return QByteArray();

    return buf.buffer();
}
*/
bool ArchiveZip::extract(const QString &file, QIODevice * out) const
{
    // Retrieve the file size
    struct zip_stat fileinfo;

    // http://www.nih.at/libzip/zip_stat.html
    if ( zip_stat( m_zip, file.toUtf8().constData(), 0, &fileinfo) != 0 )
    {
        m_errorMsg = "file not found";
        return false;
    }

    // Make sure the size field is valid
    if ( (fileinfo.valid & ZIP_STAT_SIZE) == 0 || (fileinfo.valid & ZIP_STAT_INDEX) == 0 )
    {
        m_errorMsg = "not a valid file";
        return false;
    }

    // Open the file
    struct zip_file * zgh = zip_fopen_index( m_zip, fileinfo.index, 0 );

    if ( !zgh )
    {
        m_errorMsg = "cannot open zip file";
        return false;
    }

    // Extract the content
    char buf[32768];
    unsigned int offset = 0;

    while ( offset < fileinfo.size )
    {
        int toread = qMin( sizeof(buf), fileinfo.size - offset );
        int ret = zip_fread( zgh, buf, toread );

        if ( ret != toread )
        {
            m_errorMsg = "extraction failed";
            zip_fclose( zgh );
            return false;
        }

        out->write( buf, ret );
        offset += ret;
    }

    zip_fclose( zgh );
    return true;
}
