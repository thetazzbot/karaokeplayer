#include <QFileDialog>
#include <QFileInfo>

#include "languagedetector.h"
#include "settingsdialog.h"
#include "settings.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex( 0 );

    connect( ui->boxBackgroundType, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundChanged(int)) );
    connect( ui->boxCollectionDetectLang, SIGNAL(toggled(bool)), this, SLOT(collectionLanguageCheckboxChanged()) );

    connect( ui->btnBrowseCollection, SIGNAL(clicked()), this, SLOT(browseCollection()) );
    connect( ui->btnBrowseImages, SIGNAL(clicked()), this, SLOT(browseBackgroundImages()) );
    connect( ui->btnBrowseVideos, SIGNAL(clicked()), this, SLOT(browseBackgroundVideos()) );
    connect( ui->btnBrowseLIRCdevice, SIGNAL(clicked()), this, SLOT(browseLIRCdevice()) );
    connect( ui->btnBrowseLIRCmapping, SIGNAL(clicked()), this, SLOT(browseLIRCmapping()) );

    // Prepare the languages
    ui->boxCollectionLanguage->addItem( "No language" );
    ui->boxCollectionLanguage->addItems( LanguageDetector::languages() );

    // Init the parameters
    if ( !pSettings->songCollection.isEmpty() )
    {
        const Settings::Collection& col = pSettings->songCollection[0];

        ui->leCollectionPath->setText( col.rootPath );
        ui->boxColelctionZIP->setChecked( col.scanZips );
        ui->boxCollectionDetectLang->setChecked( col.detectLanguage );

        if ( col.defaultLanguage > 0 )
            ui->boxCollectionLanguage->setCurrentText( LanguageDetector::languageFromCode(col.defaultLanguage) );

        switch ( col.pathFormat )
        {
            case Settings::Collection::PATH_FORMAT_ARTIST_DASH_TITLE:
                ui->boxCollectionFileFormat->setCurrentIndex( 1 );
                break;

            case Settings::Collection::PATH_FORMAT_ARTIST_SLASH_TITLE:
                ui->boxCollectionFileFormat->setCurrentIndex( 0 );
                break;
        }
    }

    collectionLanguageCheckboxChanged();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::backgroundChanged(int idx)
{

}

void SettingsDialog::collectionLanguageCheckboxChanged()
{
    if ( ui->boxCollectionDetectLang->isChecked() )
        ui->lblCollectionLanguage->setText( tr("Set language if not detected to:") );
    else
        ui->lblCollectionLanguage->setText( tr("Make the language for all songs:") );
}

void SettingsDialog::browseCollection()
{
    QString e = QFileDialog::getExistingDirectory( 0, "Choose the karaoke root directory" );

    if ( e.isEmpty() )
        return;

    ui->leCollectionPath->setText( e );
}

void SettingsDialog::browseBackgroundImages()
{

}

void SettingsDialog::browseBackgroundVideos()
{

}

void SettingsDialog::browseLIRCdevice()
{

}

void SettingsDialog::browseLIRCmapping()
{

}
