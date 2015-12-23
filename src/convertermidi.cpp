#include "util.h"
#include "convertermidi.h"

ConverterMIDI * pConverterMIDI;

ConverterMIDI::ConverterMIDI(QObject *parent) :
    QObject(parent)
{
    m_convProcess = 0;
}

ConverterMIDI::~ConverterMIDI()
{
    delete m_convProcess;
}

void ConverterMIDI::add( QString midiFile )
{
    m_queue.append( midiFile );

    if ( !m_convProcess )
        processNext();
}

void ConverterMIDI::convError(QProcess::ProcessError)
{
    emit finished( m_inputFile, false );

    m_inputFile.clear();
    QFile::remove( m_outputFile );

    // Do not delete the object when we're in its slot!
    m_convProcess->deleteLater();
    m_convProcess = 0;

    processNext();
}

void ConverterMIDI::convFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    emit finished( m_inputFile, true );

    // Do not delete the object when we're in its slot!
    m_convProcess->deleteLater();
    m_convProcess = 0;

    m_inputFile.clear();
    processNext();
}

void ConverterMIDI::processNext()
{
    if ( m_queue.isEmpty() || m_convProcess )
        return;

    // Using Timidity
    m_convProcess = new QProcess( this );
    connect( m_convProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(convError(QProcess::ProcessError)) );
    connect( m_convProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(convFinished(int,QProcess::ExitStatus)) );

    m_inputFile = m_queue.takeFirst();
    m_outputFile = Util::cachedFile( m_inputFile );

    if ( QFile::exists(m_outputFile) )
    {
        // File is already there, nothing to convert
        processNext();
        return;
    }

    QStringList args;
    args << "-Ow" << m_inputFile << "-o" << m_outputFile;

    m_convProcess->start( "timidity", args );
}

void ConverterMIDI::stop()
{
    m_convProcess->kill();
    m_convProcess->deleteLater();
    m_convProcess = 0;

    QFile::remove( m_outputFile );
}
