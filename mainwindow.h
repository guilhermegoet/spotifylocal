#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QReadWriteLock>
//#include <QMediaPlayer>
//#include <QMediaPlaylist>

#include "SpotifySearch.h"

#include <qoauth2authorizationcodeflow.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void granted();

    void authStatusChanged (QAbstractOAuth::Status status);
    void on_actionGrant_triggered();

    void newTrackSearchFound(QString trackID, QString trackName, QString previewURL);

    void on_actionGet_User_Information_triggered();

    void on_actionGet_Playlists_triggered();

    void on_pbRemove_clicked();

    void on_pbPlay_clicked();

    void on_pbPause_clicked();

    void on_pbRemoveMusic_clicked();

    void on_pbSearch_clicked();

    void on_pbPlaylisttLoad_clicked();

    void on_pbAddMusic_clicked();

    void on_pbAdd_clicked();

    void on_pbPlaylistSave_clicked();

private:

    void restartTrackSearch();

    void restartPlaylist();

    void inicializarIcons();

private:
    Ui::MainWindow *ui;

    QReadWriteLock lockModelPlaylist;
    QStandardItemModel *m_viewModelPlaylist;
    QOAuth2AuthorizationCodeFlow spotify;

    QReadWriteLock lockModelSearch;
    QStandardItemModel *m_viewModelSearch;

    SpotifySearch *m_instSpotifySearch;

    QMediaPlayer *m_playerPreview;
    QMediaPlaylist *m_playlistPreview;

    bool isGranted;
    QString userName;

    enum TWMusicSearch
    {
        TW_MUSIC, TW_ID
    };

    enum TWPLAYLIST_COLUMN
    {
        TW_PL_NAME = 0,
        TW_PL_ID = 1,
        TW_PL_URL = 2
    };
};

#endif // MAINWINDOW_H
