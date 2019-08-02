#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMessageBox>
#include <QtXml>
#include <QStandardItemModel>
#include <QJsonDocument>
#include <QDesktopServices>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QReadWriteLock>
#include <QJsonObject>
#include <QIcon>
#include "clientid.h"
#include "qoauthhttpserverreplyhandler.h"

bool isclicked = true;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    isGranted(false)
{
    ui->setupUi(this);

    auto replyHandler = new QOAuthHttpServerReplyHandler(8080, this);
    //replyHandler->setCallbackPath(QString("/callback/"));
    spotify.setReplyHandler(replyHandler);
    spotify.setAuthorizationUrl(QUrl("https://accounts.spotify.com/authorize"));
    spotify.setAccessTokenUrl(QUrl("https://accounts.spotify.com/api/token"));
    qDebug() << clientId;
    qDebug() << clientSecret;
    spotify.setClientIdentifier(clientId);
    spotify.setClientIdentifierSharedKey(clientSecret);
    spotify.setScope("user-read-private user-top-read playlist-read-private playlist-modify-public playlist-modify-private");
    //oauth2.setScope("identity read");

    inicializarIcons();


    connect(&spotify, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
             &QDesktopServices::openUrl);
    qDebug() << "depois do open url";

    connect(&spotify, &QOAuth2AuthorizationCodeFlow::statusChanged,
            this, &MainWindow::authStatusChanged);
    qDebug() << "depois do estado";

    connect(&spotify, &QOAuth2AuthorizationCodeFlow::granted,
            this, &MainWindow::granted);

    m_instSpotifySearch = new SpotifySearch();
    connect(m_instSpotifySearch, &SpotifySearch::newSearchTrack, this, &MainWindow::newTrackSearchFound);

    // Table With the track search result
    m_viewModelSearch = new QStandardItemModel(0, 4, this);
    ui->twMusicSearchResults->setModel(m_viewModelSearch);


    // Model that contain all playlists
    m_viewModelPlaylist = new QStandardItemModel(0, 3, this);
    ui->treeViewPlaylist->setModel(m_viewModelPlaylist);
    ui->treeViewPlaylist->resizeColumnToContents(0);
    ui->treeViewPlaylist->resizeColumnToContents(1);
    ui->treeViewPlaylist->resizeColumnToContents(2);

    m_playerPreview = new QMediaPlayer;
    m_playlistPreview = new QMediaPlaylist();

    restartTrackSearch();
    restartPlaylist();


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::inicializarIcons()
{
    QIcon iconPlay;
    iconPlay.addFile(QString::fromUtf8(":/webapi/play-button.png"), QSize(), QIcon::Normal, QIcon::Off);
    QIcon iconRemovePb;
    iconRemovePb.addFile(QString::fromUtf8(":/webapi/remove-list.png"), QSize(), QIcon::Normal, QIcon::Off);
    QIcon iconRemoveMusic;
    iconRemoveMusic.addFile(QString::fromUtf8(":/webapi/removemusic.png"), QSize(), QIcon::Normal, QIcon::Off);
    QIcon iconSave;
    iconSave.addFile(QString::fromUtf8(":/webapi/save-button.png"), QSize(), QIcon::Normal, QIcon::Off);
    QIcon iconLoad;
    iconLoad.addFile(QString::fromUtf8(":/webapi/playlist-folder.png"), QSize(), QIcon::Normal, QIcon::Off);
    QIcon iconSearch;
    iconSearch.addFile(QString::fromUtf8(":/webapi/search.png"), QSize(), QIcon::Normal, QIcon::Off);
    QIcon iconAddPb;
    iconAddPb.addFile(QString::fromUtf8(":/webapi/plus.png"), QSize(), QIcon::Normal, QIcon::Off);
    QIcon iconPauseIni;
    iconPauseIni.addFile(QString::fromUtf8(":/webapi/pause-play-button-inicio.png"), QSize(), QIcon::Normal, QIcon::Off);
    QIcon iconAddMusic;
    iconAddMusic.addFile(QString::fromUtf8(":/webapi/addmusic.png"), QSize(), QIcon::Normal, QIcon::Off);

    ui->pbAddMusic->setIcon(QIcon(iconAddMusic));
    ui->pbAddMusic->setIconSize(QSize(65, 65));
    ui->pbPlay->setIcon(QIcon(iconPlay));
    ui->pbPlay->setIconSize(QSize(65, 65));
    ui->pbPause->setIcon(QIcon(iconPauseIni));
    ui->pbPause->setIconSize(QSize(65, 65));
    ui->pbRemove->setIcon(QIcon(iconRemovePb));
    ui->pbRemove->setIconSize(QSize(65, 65));
    ui->pbRemoveMusic->setIcon(QIcon(iconRemoveMusic));
    ui->pbRemoveMusic->setIconSize(QSize(65, 65));
    ui->pbPlaylistSave->setIcon(QIcon(iconSave));
    ui->pbPlaylistSave->setIconSize(QSize(65, 65));
    ui->pbPlaylisttLoad->setIcon(QIcon(iconLoad));
    ui->pbPlaylisttLoad->setIconSize(QSize(65, 65));
    ui->pbAdd->setIcon(QIcon(iconAddPb));
    ui->pbAdd->setIconSize(QSize(25, 25));
    ui->pbSearch->setIcon(QIcon(iconSearch));
    ui->pbSearch->setIconSize(QSize(25, 25));

}


void MainWindow::granted ()
{
    qDebug() << "entou no granted";
    //ui->teOutput->appendPlainText("Signal granted received");

    QString token = spotify.token();
    //ui->teOutput->appendPlainText("Token: " + token);

    ui->actionGet_Playlists->setEnabled(true);
    ui->actionGet_User_Information->setEnabled(true);
    isGranted = true;
}

void MainWindow::authStatusChanged(QAbstractOAuth::Status status)
{
    qDebug() << "função do estado";
    QString s;
    if (status == QAbstractOAuth::Status::Granted)
        s = "granted";

    if (status == QAbstractOAuth::Status::TemporaryCredentialsReceived) {
        s = "temp credentials";
        //oauth2.refreshAccessToken();
    }

    ui->statuslabel->setText("Conectado");
}

void MainWindow::on_actionGrant_triggered()
{
    qDebug() << "Chama o Grant";
    spotify.grant();
}

void MainWindow::on_actionGet_User_Information_triggered()
{
    qDebug() << "Entrou aqui no get user";
    //ui->teOutput->appendPlainText("Loading User Informations");

    QUrl u ("https://api.spotify.com/v1/me");

    auto reply = spotify.get(u);

    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            //ui->teOutput->appendPlainText(reply->errorString());
            return;
        }
        const auto data = reply->readAll();
        //ui->teOutput->appendPlainText(data);

        const auto document = QJsonDocument::fromJson(data);
        const auto root = document.object();
        userName = root.value("id").toString();

        //ui->teOutput->appendPlainText("Username: " + userName);

        reply->deleteLater();
    });

}

void MainWindow::on_actionGet_Playlists_triggered()
{
    qDebug() << "Entrou aqui no get playlist";
    if (userName.length() == 0) return;

    QUrl u ("https://api.spotify.com/v1/users/" + userName + "/playlists");

    auto reply = spotify.get(u);

    connect (reply, &QNetworkReply::finished, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            //ui->teOutput->appendPlainText(reply->errorString());
            return;
        }

        const auto data = reply->readAll();

        //ui->teOutput->appendPlainText(data);

        reply->deleteLater();
    });

}

void MainWindow::on_pbSearch_clicked()
{
    QString strMusic = ui->leMusicSearchName->text();


    if( m_instSpotifySearch == Q_NULLPTR )
    {
        qDebug() << "m_instSpotifySearch null pointer.";
        return;
    }

    restartTrackSearch();

    m_instSpotifySearch->SearchTracksByName(&spotify ,strMusic);
}

void MainWindow::newTrackSearchFound(QString trackID, QString trackName, QString previewURL)
{
    lockModelSearch.lockForRead();

    int row = m_viewModelSearch->rowCount();

    QStandardItem *siName = new QStandardItem(trackName);
    QStandardItem *siID = new QStandardItem(trackID);
    QStandardItem *siPreviewURL = new QStandardItem(previewURL);

    m_viewModelSearch->setItem(row,0, siName);
    m_viewModelSearch->setItem(row,1, siID);
    m_viewModelSearch->setItem(row,2, siPreviewURL);


    lockModelSearch.unlock();
}


void MainWindow::on_pbAdd_clicked()
{
    QString strPlayListName = ui->inputPlaylistName->text();
    QString teste = strPlayListName.replace(" ","");
    if (teste == ""){
        QMessageBox msgBox;
        msgBox.setText("Nome em branco.");
        msgBox.setInformativeText("Favor escolher outro nome.");
        msgBox.setStandardButtons(QMessageBox::Close);
        msgBox.setDefaultButton(QMessageBox::Close);
        msgBox.exec();

        return;
    }


    lockModelPlaylist.lockForRead();

    QList<QStandardItem*> otherPlaylist;
    otherPlaylist = m_viewModelPlaylist->findItems(strPlayListName, Qt::MatchFlag::MatchExactly, 0);
    if(!otherPlaylist.isEmpty() )
    {
        QMessageBox msgBox;
        msgBox.setText("Nome da playlist já existe.");
        msgBox.setInformativeText("Favor escolher outro nome.");
        msgBox.setStandardButtons(QMessageBox::Close);
        msgBox.setDefaultButton(QMessageBox::Close);
        msgBox.exec();

        lockModelPlaylist.unlock();

        return;
    }

    int row = m_viewModelPlaylist->rowCount();
    QStandardItem *siPlaylist = new QStandardItem(strPlayListName);
    m_viewModelPlaylist->setItem(row, 0, siPlaylist);

    ui->statuslabel->setText("Playlist criada com sucesso");

    lockModelPlaylist.unlock();
}

void MainWindow::on_pbPlaylistSave_clicked()
{
    try
    {
        QFile file("Playlists.xml");
        if(!file.open(QIODevice::ReadWrite | QIODevice::Text) )
        {
            qDebug() << "Fail to open the Playlist.xml file.";
            ui->statuslabel->setText("Salvar o arquivo falhou. Não foi possível criar o arquivo para salvar a lista.");
        }
        else
        {
            QXmlStreamWriter xmlWriter(&file);
            xmlWriter.setAutoFormatting(true);
            xmlWriter.writeStartDocument();

            xmlWriter.writeStartElement("PlayLists");

            // Add  playlists
            lockModelPlaylist.lockForRead();

            int iPlaylistRows = m_viewModelPlaylist->rowCount();
            for( int iPlaylistRow = 0; iPlaylistRow < iPlaylistRows; ++iPlaylistRow)
            {
                QModelIndex indexPlaylist = m_viewModelPlaylist->index(iPlaylistRow,0,QModelIndex());
                QStandardItem *siPlaylist = m_viewModelPlaylist->itemFromIndex(indexPlaylist);
                qDebug() << "siPlaylist: " << siPlaylist->text();

                xmlWriter.writeStartElement("Playlist");
                xmlWriter.writeAttribute("name",siPlaylist->text() );

                // save all tracks
                int iTrackRows = siPlaylist->rowCount();
                for( int iTrackRow = 0; iTrackRow < iTrackRows; ++iTrackRow)
                {
                    QStandardItem *siTrackName;
                    QStandardItem *siTrackID;
                    QStandardItem *siTrackPreviewURL;

                    siTrackName = siPlaylist->child(iTrackRow, TW_PL_NAME);
                    qDebug() << "Track name: " << siTrackName->text();
                    siTrackID = siPlaylist->child(iTrackRow, TW_PL_ID);
                    qDebug() << "Track ID: " << siTrackID->text();
                    siTrackPreviewURL = siPlaylist->child(iTrackRow, TW_PL_URL);
                    qDebug() << "Track preview url: " << siTrackPreviewURL->text();


                    xmlWriter.writeStartElement("Track");

                    xmlWriter.writeAttribute("id", siTrackID->text());
                    xmlWriter.writeAttribute("name", siTrackName->text());
                    xmlWriter.writeAttribute("preview_url", siTrackPreviewURL->text());

                    xmlWriter.writeEndElement();

                }

                xmlWriter.writeEndElement();
            }

            xmlWriter.writeEndElement();

            file.close();

            if (isclicked !=false){
                qDebug() << "Playlist.xml saved with success.";
                ui->statuslabel->setText("Playlists salvas com sucesso.");
            }


            lockModelPlaylist.unlock();
        }

    } catch (...) {
        qDebug() << "Fail to open the Playlist.xml file.";
        ui->statuslabel->setText("Salvar o arquivo falhou. Não foi possível criar o arquivo para salvar a lista.");

        lockModelPlaylist.unlock();
    }
    isclicked = true;
}

void MainWindow::on_pbPlaylisttLoad_clicked()
{
    try
    {
        // Open the saved playlists
        QFile file("Playlists.xml");
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text) )
        {
            qDebug() << "Fail to open the Playlist.xml file.";

            ui->statuslabel->setText("Carregar playlist falhou. Não foi possível abrir o arquivo.");
        }
        else
        {
            restartPlaylist();

            QDomDocument doc;
            if(!doc.setContent(&file))
            {
                file.close();
                qDebug() << "Configuration file is no a valid XML.";
                ui->statuslabel->setText("Arquivo de playlist corrompido.");

                return;
            }

            lockModelPlaylist.lockForRead();

            QDomNodeList nodeListPlaylist = doc.elementsByTagName("Playlist");

            for (int iPlaylist = 0; iPlaylist < nodeListPlaylist.size(); iPlaylist++)
            {
                qDebug() <<nodeListPlaylist.item(0).nodeName();
                QDomNode nodePlaylist = nodeListPlaylist.item(iPlaylist);

                QString strPlayListName = nodePlaylist.attributes().namedItem("name").nodeValue();

                // add playlist on model
                QStandardItem *siPlaylist = new QStandardItem(strPlayListName);
                m_viewModelPlaylist->setItem(iPlaylist, 0, siPlaylist);

                QDomNodeList nodeListTracks = nodePlaylist.childNodes();
                for (int iTrack = 0; iTrack < nodeListTracks.size(); iTrack++)
                {
                    QDomNode nodeTrack = nodeListTracks.item(iTrack);
                    QString strTrackID = nodeTrack.attributes().namedItem("id").nodeValue();
                    QString strTrackName = nodeTrack.attributes().namedItem("name").nodeValue();
                    QString strTrackURL = nodeTrack.attributes().namedItem("preview_url").nodeValue();


                    QStandardItem *siTrackID = new QStandardItem(strTrackID);
                    QStandardItem *siTrackName = new QStandardItem(strTrackName);
                    QStandardItem *siTrackPreviewURL = new QStandardItem(strTrackURL);

                    siTrackName->setCheckable(true);

                    siPlaylist->setChild(iTrack, TW_PL_NAME, siTrackName);
                    siPlaylist->setChild(iTrack, TW_PL_ID, siTrackID);
                    siPlaylist->setChild(iTrack, TW_PL_URL, siTrackPreviewURL);
                }
            }

            file.close();

            qDebug() << "Playlist.xml load with success.";
            ui->statuslabel->setText("Playlists carregada com sucesso.");

            lockModelPlaylist.unlock();
        }

    } catch (...) {
        qDebug() << "Fail to open the Playlist.xml file.";
        ui->statuslabel->setText("Carregar playlist falhou. Não foi possível abrir o arquivo.");

        lockModelPlaylist.unlock();
    }
}


void MainWindow::on_pbAddMusic_clicked()
{
    try {

        QModelIndexList selectionPlaylist = ui->treeViewPlaylist->selectionModel()->selectedRows();
        if(selectionPlaylist.isEmpty())
        {
            QMessageBox msgBox;
            msgBox.setText("Inclusão de música na playlist.");
            msgBox.setInformativeText("Para incluir uma música, selecione antes uma playlist de destino.");
            msgBox.setStandardButtons(QMessageBox::Close);
            msgBox.setDefaultButton(QMessageBox::Close);
            msgBox.exec();

            return;
        }

        QModelIndexList selectionTracks = ui->twMusicSearchResults->selectionModel()->selectedRows();
        if(selectionTracks.isEmpty())
        {
            QMessageBox msgBox;
            msgBox.setText("Inclusão de música na playlist.");
            msgBox.setInformativeText("Selecione uma música para ser adicionada.");
            msgBox.setStandardButtons(QMessageBox::Close);
            msgBox.setDefaultButton(QMessageBox::Close);
            msgBox.exec();

            return;
        }

        // get the track name and id from search

        lockModelSearch.lockForRead();

        int trackRow = selectionTracks[0].row();
        QModelIndex index = m_viewModelSearch->index(trackRow,TW_PL_NAME,QModelIndex());
        QStandardItem *siTrackName = m_viewModelSearch->itemFromIndex(index)->clone();
        index = m_viewModelSearch->index(trackRow,TW_PL_ID,QModelIndex());
        QStandardItem *siTrackID = m_viewModelSearch->itemFromIndex(index)->clone();
        index = m_viewModelSearch->index(trackRow,TW_PL_URL,QModelIndex());
        QStandardItem *siTrackPreviewURL = m_viewModelSearch->itemFromIndex(index)->clone();

        lockModelSearch.unlock();

        // find the playlist selected.
        lockModelPlaylist.lockForRead();

        int playlistRow = selectionPlaylist[0].row();
        index = m_viewModelPlaylist->index(playlistRow,0,QModelIndex());
        qDebug() << m_viewModelPlaylist->data(index).toString();
        QStandardItem *siPlaylist = m_viewModelPlaylist->itemFromIndex(index);

        qDebug() << siPlaylist->text();

        int iTrackOnPlaylist = siPlaylist->rowCount();
        qDebug() << "Track on the current playlist: " << iTrackOnPlaylist;

        // test to check if the current track alread was included
        QStandardItem *siChildID;
        if( iTrackOnPlaylist>0 )
        {
            for( int iRow = 0; iRow < iTrackOnPlaylist;++iRow )
            {
                siChildID = siPlaylist->child(iRow, TW_PL_ID);
                if( siChildID == Q_NULLPTR)
                {
                    qDebug() << "siChildID is NULL.";
                    return ;
                }
                qDebug() << "Track ID on the list: " << siChildID->text();
                qDebug() << "Track ID on the search list: " << siTrackID->text();
                qDebug() << "Track Preview URL: " << siTrackPreviewURL->text();

                if(siChildID->text() == siTrackID->text())
                {
                    delete(siTrackName);
                    delete(siTrackID);
                    delete(siTrackPreviewURL);

                    QMessageBox msgBox;
                    msgBox.setText("Inclusão de música na playlist.");
                    msgBox.setInformativeText("Só é possível incluir uma música uma unica vez na playlist.");
                    msgBox.setStandardButtons(QMessageBox::Close);
                    msgBox.setDefaultButton(QMessageBox::Close);
                    msgBox.exec();

                    return;
                }
            }
        }

        siTrackName->setCheckable(true);

        siPlaylist->setChild(iTrackOnPlaylist, TW_PL_NAME, siTrackName);
        siPlaylist->setChild(iTrackOnPlaylist, TW_PL_ID, siTrackID);
        siPlaylist->setChild(iTrackOnPlaylist, TW_PL_URL, siTrackPreviewURL);

        lockModelPlaylist.unlock();
    } catch (...) {
       qDebug() << "Exception on on_pbAddMusic_clicked.";
       lockModelPlaylist.unlock();
    }
}

void MainWindow::on_pbRemove_clicked()
{
    QModelIndexList selection = ui->treeViewPlaylist->selectionModel()->selectedRows();

    QList<QStandardItem*> items ;
    int row;

    if(!selection.isEmpty())
    {
        row = selection[0].row();

        lockModelPlaylist.lockForRead();

        items = m_viewModelPlaylist->takeRow(row);

        foreach(QStandardItem *item, items)
        {
            delete(item);
        }

        lockModelPlaylist.unlock();
    }
    if(QFileInfo::exists("Playlists.xml"))
    {
        qDebug () << "file exists" ;
        QFile::remove("Playlists.xml");
        ui->statuslabel->setText("Playlist apagada");
        isclicked = false;
        on_pbPlaylistSave_clicked();
    }    else
    {
        qDebug () << "file does not exists" ;
    }
}


void MainWindow::on_pbRemoveMusic_clicked()
{

    try {

        QModelIndexList selectionPlaylist = ui->treeViewPlaylist->selectionModel()->selectedRows();
        if(selectionPlaylist.isEmpty())
        {
            QMessageBox msgBox;
            msgBox.setText("Remover música na playlist.");
            msgBox.setInformativeText("Selecione a música desejada para ser removida.");
            msgBox.setStandardButtons(QMessageBox::Close);
            msgBox.setDefaultButton(QMessageBox::Close);
            msgBox.exec();

            return;
        }

        // remove all tracks checked on the current playlist.
        lockModelPlaylist.lockForRead();

        int playlistRow = selectionPlaylist[0].row();
        QModelIndex index = m_viewModelPlaylist->index(playlistRow,0,QModelIndex());
        qDebug() << m_viewModelPlaylist->data(index).toString();


        QStandardItem *siPlaylist = m_viewModelPlaylist->itemFromIndex(index);

        int iTrackOnPlaylist = siPlaylist->rowCount();
        for( int iRowChild = iTrackOnPlaylist-1; iRowChild >= 0;--iRowChild )
        {
            QStandardItem *siChildID;
            siChildID = siPlaylist->child(iRowChild, TW_PL_NAME);
            qDebug() << siChildID->text();

            Qt::CheckState currentState = siChildID->checkState();
            if( currentState == Qt::CheckState::Checked )
            {
                QList<QStandardItem*> items ;
                items = siPlaylist->takeRow(iRowChild);

                foreach(QStandardItem *item, items)
                {
                    delete(item);
                }
            }
        }

        lockModelPlaylist.unlock();
    } catch (...) {
       qDebug() << "Exception on on_pbRemoveMusic_clicked.";
       lockModelPlaylist.unlock();
    }
    if(QFileInfo::exists("Playlists.xml"))
    {
        qDebug () << "file exists" ;
        QFile::remove("Playlists.xml");
        isclicked = false;
        on_pbPlaylistSave_clicked();
    }    else
    {
        qDebug () << "file does not exists" ;
    }
}

void MainWindow::on_pbPlay_clicked()
{

    ui->statuslabel->setText("Carregando Música.");

    if(m_playerPreview->state() != QMediaPlayer::PausedState)
        m_playerPreview->stop();

    QModelIndexList selection = ui->treeViewPlaylist->selectionModel()->selectedRows();
    if(selection.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText("Reproduzir preview das mpusicas.");
        msgBox.setInformativeText("Necessário selecione a playlist que deseja ouvir.");
        msgBox.setStandardButtons(QMessageBox::Close);
        msgBox.setDefaultButton(QMessageBox::Close);
        msgBox.exec();

        return;
    }

    m_playlistPreview = new QMediaPlaylist();

    QList<QStandardItem*> items ;
    int playlistRow = selection[0].row();
    lockModelPlaylist.lockForRead();

    QModelIndex index = m_viewModelPlaylist->index(playlistRow,0,QModelIndex());
    QStandardItem *siPlaylist = m_viewModelPlaylist->itemFromIndex(index);
    qDebug() << "siPlaylist: " << siPlaylist->text();

    int iTracksOnPlaylist = siPlaylist->rowCount();
    for( int iRowChild = 0; iRowChild < iTracksOnPlaylist;++iRowChild )
    {
        QStandardItem *siChildPreviewURL;
        siChildPreviewURL = siPlaylist->child(iRowChild, TW_PL_URL);
        qDebug() << siChildPreviewURL->text();

        m_playlistPreview->addMedia(QUrl(siChildPreviewURL->text()));
    }

    lockModelPlaylist.unlock();

    m_playerPreview->setPlaylist(m_playlistPreview);

    ui->statuslabel->setText("Iniciando preview.");
    m_playerPreview->play();

}

void MainWindow::on_pbPause_clicked()
{
    QIcon iconPausePause;
    iconPausePause.addFile(QString::fromUtf8(":/webapi/pause-play-button_pause.png"), QSize(), QIcon::Normal, QIcon::Off);
    QIcon iconPause;
    iconPause.addFile(QString::fromUtf8(":/webapi/pause-play-button.png"), QSize(), QIcon::Normal, QIcon::Off);
    if(m_playerPreview->state() != QMediaPlayer::PausedState)
    {
        m_playerPreview->pause();

        ui->pbPause->setIcon(QIcon(iconPausePause));
        ui->pbPause->setIconSize(QSize(65, 65));

    }
    else
    {
        m_playerPreview->play();

        ui->pbPause->setIcon(QIcon(iconPause));
        ui->pbPause->setIconSize(QSize(65, 65));
    }
}

void MainWindow::restartTrackSearch()
{
    lockModelSearch.lockForRead();

    QStringList strMusicHeadLables;
    strMusicHeadLables << "Musica" << "ID";
    m_viewModelSearch->clear();
    m_viewModelSearch->setHorizontalHeaderLabels(strMusicHeadLables);
    ui->twMusicSearchResults->resizeColumnsToContents();
    ui->twMusicSearchResults->hideColumn(2);

    lockModelSearch.unlock();
}

void MainWindow::restartPlaylist()
{
    lockModelPlaylist.lockForRead();

    QStringList strPlaylistHeadLables;
    strPlaylistHeadLables << "Nome" << "ID";
    m_viewModelPlaylist->clear();
    m_viewModelPlaylist->setHorizontalHeaderLabels(strPlaylistHeadLables);
    //ui->treeViewPlaylist->resize();
    ui->treeViewPlaylist->hideColumn(2);

    lockModelPlaylist.unlock();
}
