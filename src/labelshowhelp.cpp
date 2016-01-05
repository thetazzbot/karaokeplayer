#include <QWhatsThis>

#include "labelshowhelp.h"

LabelShowHelp::LabelShowHelp( QWidget *parent )
    : QLabel(parent)
{
    connect( this, SIGNAL(linkActivated(QString)), this, SLOT(clicked()) );

    if ( !buddy() )
        qDebug("WARNING: help widget %s has no buddy!", qPrintable(objectName()) );
}

void LabelShowHelp::clicked()
{
    // Link is clicked - do we have a buddy?
    QString text = buddy()->whatsThis();

    QWhatsThis::showText ( mapToParent( pos() ), text );
}
