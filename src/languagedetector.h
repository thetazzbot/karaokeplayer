#ifndef LANGUAGEDETECTOR_H
#define LANGUAGEDETECTOR_H

#include <QLibrary>
#include <QString>
#include "libcld2/cld2_api.h"


// Interface for libcld.so - it is 4Mb library, no need to keep it loaded all the time,
// as it is only used during scanning (and even there it is not really necessary)
class LanguageDetector
{
    public:
        LanguageDetector();

        bool    init();

        // Takes care about the language ID conversion too
        int     detectLanguage( const QByteArray& data );

        QString errorString() const { return m_errorString; }

        // This returns UTF-8 string
        static QString  languageFromCode( int code );
        static int      codeFromLanguage( const QString& language );

        // This returns all languages
        static QStringList  languages();

    private:
        QLibrary    m_libcld;
        cld2_detect_language_pfn *   m_pfn;
        QString     m_errorString;
};

#endif // LANGUAGEDETECTOR_H
