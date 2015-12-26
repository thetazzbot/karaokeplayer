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
