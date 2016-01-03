#ifndef CLD2_LANGDETECT_H
#define CLD2_LANGDETECT_H

// C++ API wrapper returns language code according to ISO 639-3 with letters in bits 24-0, big-engian.
//
// For example, for English it would return \x00ENG or \x00 \x45 \x4E \x47
#define MAKE_LANG(a,b,c)    ((int) (((a) << 16)|((b)<<8)|c))

extern "C" int cld2_detect_language( const char* buffer, int buffer_length, bool* is_reliable );

typedef int (cld2_detect_language_pfn)( const char* buffer, int buffer_length, bool* is_reliable );

#endif // CLD2_LANGDETECT_H
