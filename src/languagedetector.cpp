#include <QDir>
#include <QApplication>

#include "languagedetector.h"

struct language_map
{
    int code;
    const char * name;
};

static const struct language_map languageMap[] =
{
    { MAKE_LANG( 'E', 'N', 'G' ), "English" },
    { MAKE_LANG( 'D', 'A', 'N' ), "Danish" },
    { MAKE_LANG( 'N', 'L', 'D' ), "Dutch" },
    { MAKE_LANG( 'F', 'I', 'N' ), "Finnish" },
    { MAKE_LANG( 'F', 'R', 'A' ), "French" },
    { MAKE_LANG( 'D', 'E', 'U' ), "German" },
    { MAKE_LANG( 'H', 'E', 'B' ), "Hebrew" },
    { MAKE_LANG( 'I', 'T', 'A' ), "Italian" },
    { MAKE_LANG( 'J', 'P', 'N' ), "Japanese" },
    { MAKE_LANG( 'K', 'O', 'R' ), "Korean" },
    { MAKE_LANG( 'N', 'O', 'R' ), "Norwegian" },
    { MAKE_LANG( 'P', 'O', 'L' ), "Polish" },
    { MAKE_LANG( 'P', 'O', 'R' ), "Portuguese" },
    { MAKE_LANG( 'R', 'U', 'S' ), "Russian" },
    { MAKE_LANG( 'S', 'P', 'A' ), "Spanish" },
    { MAKE_LANG( 'S', 'W', 'E' ), "Swedish" },
    { MAKE_LANG( 'Z', 'H', 'O' ), "Chinese" },
    { MAKE_LANG( 'C', 'E', 'S' ), "Czech" },
    { MAKE_LANG( 'E', 'L', 'L' ), "Modern Greek" },
    { MAKE_LANG( 'I', 'S', 'L' ), "Icelandic" },
    { MAKE_LANG( 'L', 'A', 'V' ), "Latvian" },
    { MAKE_LANG( 'L', 'I', 'T' ), "Lithuanian" },
    { MAKE_LANG( 'R', 'O', 'N' ), "Romanian" },
    { MAKE_LANG( 'H', 'U', 'N' ), "Hungarian" },
    { MAKE_LANG( 'E', 'S', 'T' ), "Estonian" },
    { MAKE_LANG( 'B', 'U', 'L' ), "Bulgarian" },
    { MAKE_LANG( 'H', 'R', 'V' ), "Croatian" },
    { MAKE_LANG( 'S', 'R', 'P' ), "Serbian" },
    { MAKE_LANG( 'G', 'L', 'E' ), "Irish" },
    { MAKE_LANG( 'G', 'L', 'G' ), "Galician" },
    { MAKE_LANG( 'T', 'G', 'L' ), "Tagalog" },
    { MAKE_LANG( 'T', 'U', 'R' ), "Turkish" },
    { MAKE_LANG( 'U', 'K', 'R' ), "Ukrainian" },
    { MAKE_LANG( 'H', 'I', 'N' ), "Hindi" },
    { MAKE_LANG( 'M', 'K', 'D' ), "Macedonian" },
    { MAKE_LANG( 'B', 'E', 'N' ), "Bengali" },
    { MAKE_LANG( 'I', 'N', 'D' ), "Indonesian" },
    { MAKE_LANG( 'L', 'A', 'T' ), "Latin" },
    { MAKE_LANG( 'M', 'S', 'A' ), "Malay" },
    { MAKE_LANG( 'M', 'A', 'L' ), "Malayalam" },
    { MAKE_LANG( 'C', 'Y', 'M' ), "Welsh" },
    { MAKE_LANG( 'N', 'E', 'P' ), "Nepali" },
    { MAKE_LANG( 'T', 'E', 'L' ), "Telugu" },
    { MAKE_LANG( 'S', 'Q', 'I' ), "Albanian" },
    { MAKE_LANG( 'T', 'A', 'M' ), "Tamil" },
    { MAKE_LANG( 'B', 'E', 'L' ), "Belarusian" },
    { MAKE_LANG( 'J', 'A', 'V' ), "Javanese" },
    { MAKE_LANG( 'O', 'C', 'I' ), "Occitan" },
    { MAKE_LANG( 'U', 'R', 'D' ), "Urdu" },
    { MAKE_LANG( 'B', 'I', 'H' ), "Bihari" },
    { MAKE_LANG( 'G', 'U', 'J' ), "Gujarati" },
    { MAKE_LANG( 'T', 'H', 'A' ), "Thai" },
    { MAKE_LANG( 'A', 'R', 'A' ), "Arabic" },
    { MAKE_LANG( 'C', 'A', 'T' ), "Catalan" },
    { MAKE_LANG( 'E', 'P', 'O' ), "Esperanto" },
    { MAKE_LANG( 'E', 'U', 'S' ), "Basque" },
    { MAKE_LANG( 'I', 'N', 'A' ), "Interlingua" },
    { MAKE_LANG( 'K', 'A', 'N' ), "Kannada" },
    { MAKE_LANG( 'P', 'A', 'N' ), "Panjabi" },
    { MAKE_LANG( 'G', 'L', 'A' ), "Scottish Gaelic" },
    { MAKE_LANG( 'S', 'W', 'H' ), "Swahili" },
    { MAKE_LANG( 'S', 'L', 'V' ), "Slovenian" },
    { MAKE_LANG( 'M', 'A', 'R' ), "Marathi" },
    { MAKE_LANG( 'M', 'L', 'T' ), "Maltese" },
    { MAKE_LANG( 'V', 'I', 'E' ), "Vietnamese" },
    { MAKE_LANG( 'F', 'R', 'Y' ), "Western Frisian" },
    { MAKE_LANG( 'S', 'L', 'K' ), "Slovak" },
    { MAKE_LANG( 'Z', 'H', 'O' ), "Chinese" },
    { MAKE_LANG( 'F', 'A', 'O' ), "Faroese" },
    { MAKE_LANG( 'S', 'U', 'N' ), "Sundanese" },
    { MAKE_LANG( 'U', 'Z', 'B' ), "Uzbek" },
    { MAKE_LANG( 'A', 'M', 'H' ), "Amharic" },
    { MAKE_LANG( 'A', 'Z', 'E' ), "Azerbaijani" },
    { MAKE_LANG( 'K', 'A', 'T' ), "Georgian" },
    { MAKE_LANG( 'T', 'I', 'R' ), "Tigrinya" },
    { MAKE_LANG( 'F', 'A', 'S' ), "Persian" },
    { MAKE_LANG( 'B', 'O', 'S' ), "Bosnian" },
    { MAKE_LANG( 'S', 'I', 'N' ), "Sinhala" },
    { MAKE_LANG( 'N', 'N', 'O' ), "Norwegian Nynorsk" },
    { MAKE_LANG( 'X', 'H', 'O' ), "Xhosa" },
    { MAKE_LANG( 'Z', 'U', 'L' ), "Zulu" },
    { MAKE_LANG( 'G', 'R', 'N' ), "Guarani" },
    { MAKE_LANG( 'S', 'O', 'T' ), "Southern Sotho" },
    { MAKE_LANG( 'T', 'U', 'K' ), "Turkmen" },
    { MAKE_LANG( 'K', 'I', 'R' ), "Kirghiz" },
    { MAKE_LANG( 'B', 'R', 'E' ), "Breton" },
    { MAKE_LANG( 'T', 'W', 'I' ), "Twi" },
    { MAKE_LANG( 'Y', 'I', 'D' ), "Yiddish" },
    { MAKE_LANG( 'S', 'O', 'M' ), "Somali" },
    { MAKE_LANG( 'U', 'I', 'G' ), "Uighur" },
    { MAKE_LANG( 'K', 'U', 'R' ), "Kurdish" },
    { MAKE_LANG( 'M', 'O', 'N' ), "Mongolian" },
    { MAKE_LANG( 'H', 'Y', 'E' ), "Armenian" },
    { MAKE_LANG( 'L', 'A', 'O' ), "Lao" },
    { MAKE_LANG( 'S', 'N', 'D' ), "Sindhi" },
    { MAKE_LANG( 'R', 'O', 'H' ), "Romansh" },
    { MAKE_LANG( 'A', 'F', 'R' ), "Afrikaans" },
    { MAKE_LANG( 'L', 'T', 'Z' ), "Luxembourgish" },
    { MAKE_LANG( 'M', 'Y', 'A' ), "Burmese" },
    { MAKE_LANG( 'K', 'H', 'M' ), "Khmer" },
    { MAKE_LANG( 'B', 'O', 'D' ), "Tibetan" },
    { MAKE_LANG( 'D', 'I', 'V' ), "Dhivehi" },
    { MAKE_LANG( 'C', 'H', 'R' ), "Cherokee" },
    { MAKE_LANG( 'S', 'Y', 'R' ), "Syriac" },
    { MAKE_LANG( 'L', 'I', 'F' ), "Limbu" },
    { MAKE_LANG( 'O', 'R', 'Y' ), "Oriya" },
    { MAKE_LANG( 'A', 'S', 'M' ), "Assamese" },
    { MAKE_LANG( 'C', 'O', 'S' ), "Corsican" },
    { MAKE_LANG( 'I', 'L', 'E' ), "Interlingue" },
    { MAKE_LANG( 'K', 'A', 'Z' ), "Kazakh" },
    { MAKE_LANG( 'L', 'I', 'N' ), "Lingala" },
    { MAKE_LANG( 'P', 'S', 'T' ), "Pashto" },
    { MAKE_LANG( 'Q', 'U', 'E' ), "Quechua" },
    { MAKE_LANG( 'S', 'N', 'A' ), "Shona" },
    { MAKE_LANG( 'T', 'G', 'K' ), "Tajik" },
    { MAKE_LANG( 'T', 'A', 'T' ), "Tatar" },
    { MAKE_LANG( 'T', 'O', 'N' ), "Tonga" },
    { MAKE_LANG( 'Y', 'O', 'R' ), "Yoruba" },
    { MAKE_LANG( 'M', 'R', 'I' ), "Maori" },
    { MAKE_LANG( 'W', 'O', 'L' ), "Wolof" },
    { MAKE_LANG( 'A', 'B', 'K' ), "Abkhazian" },
    { MAKE_LANG( 'A', 'A', 'R' ), "Afar" },
    { MAKE_LANG( 'A', 'Y', 'M' ), "Aymara" },
    { MAKE_LANG( 'B', 'A', 'K' ), "Bashkir" },
    { MAKE_LANG( 'B', 'I', 'S' ), "Bislama" },
    { MAKE_LANG( 'D', 'Z', 'O' ), "Dzongkha" },
    { MAKE_LANG( 'F', 'I', 'J' ), "Fijian" },
    { MAKE_LANG( 'K', 'A', 'L' ), "Kalaallisut" },
    { MAKE_LANG( 'H', 'A', 'U' ), "Hausa" },
    { MAKE_LANG( 'H', 'A', 'T' ), "Haitian" },
    { MAKE_LANG( 'I', 'P', 'K' ), "Inupiaq" },
    { MAKE_LANG( 'I', 'K', 'U' ), "Inuktitut" },
    { MAKE_LANG( 'K', 'A', 'S' ), "Kashmiri" },
    { MAKE_LANG( 'K', 'I', 'N' ), "Kinyarwanda" },
    { MAKE_LANG( 'M', 'L', 'G' ), "Malagasy" },
    { MAKE_LANG( 'N', 'A', 'U' ), "Nauru" },
    { MAKE_LANG( 'O', 'R', 'M' ), "Oromo" },
    { MAKE_LANG( 'R', 'U', 'N' ), "Rundi" },
    { MAKE_LANG( 'S', 'M', 'O' ), "Samoan" },
    { MAKE_LANG( 'S', 'A', 'G' ), "Sango" },
    { MAKE_LANG( 'S', 'A', 'N' ), "Sanskrit" },
    { MAKE_LANG( 'S', 'S', 'W' ), "Swati" },
    { MAKE_LANG( 'T', 'S', 'O' ), "Tsonga" },
    { MAKE_LANG( 'T', 'S', 'N' ), "Tswana" },
    { MAKE_LANG( 'V', 'O', 'L' ), "Volapuk" },
    { MAKE_LANG( 'Z', 'H', 'A' ), "Zhuang" },
    { MAKE_LANG( 'K', 'H', 'A' ), "Khasi" },
    { MAKE_LANG( 'S', 'C', 'O' ), "Scots" },
    { MAKE_LANG( 'L', 'U', 'G' ), "Ganda" },
    { MAKE_LANG( 'G', 'L', 'V' ), "Manx" },
    { MAKE_LANG( 'S', 'R', 'P' ), "Serbian" },
    { MAKE_LANG( 'A', 'K', 'A' ), "Akan" },
    { MAKE_LANG( 'I', 'B', 'O' ), "Igbo" },
    { MAKE_LANG( 'H', 'A', 'W' ), "Hawaiian" },
    { MAKE_LANG( 'C', 'E', 'B' ), "Cebuano" },
    { MAKE_LANG( 'E', 'W', 'E' ), "Ewe" },
    { MAKE_LANG( 'G', 'A', 'A' ), "Ga" },
    { MAKE_LANG( 'H', 'M', 'N' ), "Hmong" },
    { MAKE_LANG( 'K', 'R', 'I' ), "Krio" },
    { MAKE_LANG( 'L', 'O', 'Z' ), "Lozi" },
    { MAKE_LANG( 'N', 'E', 'W' ), "Newari" },
    { MAKE_LANG( 'N', 'Y', 'A' ), "Nyanja" },
    { MAKE_LANG( 'O', 'S', 'S' ), "Ossetian" },
    { MAKE_LANG( 'P', 'A', 'M' ), "Pampanga" },
    { MAKE_LANG( 'N', 'S', 'O' ), "Pedi" },
    { MAKE_LANG( 'R', 'A', 'J' ), "Rajasthani" },
    { MAKE_LANG( 'C', 'R', 'S' ), "Seselwa" },
    { MAKE_LANG( 'T', 'U', 'M' ), "Tumbuka" },
    { MAKE_LANG( 'V', 'E', 'N' ), "Venda" },
    { MAKE_LANG( 'W', 'A', 'R' ), "Waray" },
    { 0, 0 }
};


LanguageDetector::LanguageDetector()
{
}

bool LanguageDetector::init()
{
    m_libcld.setFileName( "libcld2" );

    if ( !m_libcld.load() )
    {
        m_libcld.setFileName( QApplication::applicationDirPath() + QDir::separator() + "libcld2" );

        if ( !m_libcld.load() )
        {
            m_errorString = m_libcld.errorString();
            return false;
        }
    }

    m_pfn = (cld2_detect_language_pfn*) m_libcld.resolve("cld2_detect_language");

    if ( !m_pfn )
    {
        m_errorString = QString("Cannot resolve function cld2_detect_language in a shared library %1").arg( m_libcld.fileName());
        return false;
    }

    return true;
}

int LanguageDetector::detectLanguage(const QByteArray &data)
{
    bool isreliable;
    return m_pfn( data.constData(), data.size(), &isreliable );
}

QString LanguageDetector::languageFromCode(int code)
{
    for ( const struct language_map * l = languageMap; l->name; l++ )
        if ( l->code == code )
            return QString::fromUtf8( l->name );

    return "Unknown";
}
