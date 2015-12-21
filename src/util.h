#ifndef UTIL_H
#define UTIL_H

#include <QString>

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

    private:
        Util();
};

#endif // UTIL_H
