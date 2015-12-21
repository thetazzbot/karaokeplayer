#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>

class Logger : public QObject
{
    Q_OBJECT

    public:
        explicit Logger(QObject *parent = 0);

        static void debug( const QString& str );

    signals:

    public slots:


};

#endif // LOGGER_H
