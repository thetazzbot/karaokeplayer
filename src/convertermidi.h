#ifndef CONVERTERMIDI_H
#define CONVERTERMIDI_H

#include <QObject>
#include <QQueue>
#include <QProcess>

class KaraokeSong;

// Converts MIDI to wav
class ConverterMIDI : public QObject
{
    Q_OBJECT

    public:
        explicit ConverterMIDI( QObject *parent = 0 );
        ~ConverterMIDI();

    signals:
        void    finished( QString midiFile, bool succeed );

    public slots:
        void    add( QString midiFile );

    private slots:
        void	convError( QProcess::ProcessError error );
        void	convFinished( int exitCode, QProcess::ExitStatus exitStatus );

    private:
        void    processNext();
        void    stop();

    private:
        QQueue< QString > m_queue;

        // For conversion
        QProcess         *  m_convProcess;

        QString             m_inputFile;
        QString             m_outputFile;
};

extern ConverterMIDI * pConverterMIDI;

#endif // CONVERTERMIDI_H
