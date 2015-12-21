#include "settings.h"
#include "logger.h"
#include "util.h"
#include "eventcontroller_lirc.h"

EventController_LIRC::EventController_LIRC(QObject *parent) :
    QObject(parent)
{
    m_socket.connectToServer( pSettings->lircDevicePath );

    connect( &m_socket, SIGNAL(connected()), this, SLOT(connected()) );
    connect( &m_socket, SIGNAL(disconnected()), this, SLOT(disconnected()) );
    connect( &m_socket, SIGNAL(readyRead()), this, SLOT(readyRead()) );
    connect( &m_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(error(QLocalSocket::LocalSocketError)) );

    readMap();
}

void EventController_LIRC::connected()
{
    Logger::debug( "LIRC: connected" );
}

void EventController_LIRC::disconnected()
{
    Logger::debug( "LIRC: disconnected" );

    // Reconnect
    m_socket.connectToServer( pSettings->lircDevicePath );
}

void EventController_LIRC::error(QLocalSocket::LocalSocketError socketError)
{

}

void EventController_LIRC::readyRead()
{
    m_buffer.append( m_socket.readAll() );

    // Proceed the entries in the buffer
    int pos;

    while ( (pos = m_buffer.indexOf( '\n' ) ) != -1 )
    {
        QByteArray str = m_buffer.left( pos );

        Logger::debug( QString("LIRC: received from remote %1") .arg( str.data() ) );
        m_buffer.remove( 0, pos + 1 );

        // Parse the data
        QByteArrayList data = str.split( ' ' );

        if ( data.size() != 4 )
            continue;

        // Skip followup commands
        if ( data[1] != "00" )
            continue;

        const char * keyname = data[2].data();
        int eventcode = EventController::eventByName( keyname );

        if ( eventcode == -1 )
        {
            if ( !m_eventMap.contains( keyname ) )
            {
                Logger::debug( QString("LIRC: no translation found for key '%1', ignoring") .arg( keyname ) );
                continue;
            }

            eventcode = m_eventMap[keyname];
            Logger::debug( QString("LIRC: translation: key %1 is translated to action %2") .arg( keyname ) .arg(eventcode) );
        }
        else
            Logger::debug( QString("LIRC: key %1 generates action %2") .arg( keyname ) .arg(eventcode) );

        emit lircEvent( eventcode );
    }
}

void EventController_LIRC::readMap()
{
    QFile f( pSettings->lircMappingFile );

    if ( !f.open( QIODevice::ReadOnly ) )
    {
        pController->warning( tr("Can't open LIRC mapping file %1: remote might not work") .arg( pSettings->lircMappingFile ) );
        return;
    }

    while ( !f.atEnd() )
    {
        QByteArray line = f.readLine();

        if ( line.isEmpty() || line.startsWith( '#') )
            continue;

        // REMOTE <command>
        int i = Util::indexOfSpace( line );

        if ( i == -1 )
            continue;

        QByteArray cmd = line.left( i );
        QByteArray event = line.mid( i + 1 ).trimmed();
        int eventcode = EventController::eventByName( event.data() );

        if ( eventcode == -1 )
        {
            pController->warning( tr("Invalid event name %1") .arg( event.data() ) );
            continue;
        }

        m_eventMap[ cmd ] = eventcode;
    }
}
