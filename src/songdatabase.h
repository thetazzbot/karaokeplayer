#ifndef SONGDATABASE_H
#define SONGDATABASE_H

#include <QObject>

class SongDatabase : public QObject
{
    public:
        SongDatabase();

        void    startScanning();
};

#endif // SONGDATABASE_H
