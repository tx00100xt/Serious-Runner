#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QDir>
#include <QDebug>
#include <QPixmap>
#include <QBitmap>
#include <QProcess>
#include <QFileInfo>
#include <QTableView>
#include <QtSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QDesktopServices>
#include <QAction>
#include <QMenu>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include <QLineEdit>
#include "Classes/Downloader.h"
#include "Classes/FindInHomeDir.h"
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Model: public QSqlQueryModel
{
  QVariant data(const QModelIndex &index, int role) const
  {
    if (role==Qt::TextAlignmentRole)
      return Qt::AlignCenter;
    return QSqlQueryModel::data(index, role);
  }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    // Get remote file size
    void RequestFileSize(const QString &url);
    QNetworkAccessManager *m_netmanager;
    QNetworkReply *m_reply;

public slots:
     void FileSize();

signals:
    void signalTestGameDataInstall();

protected:
    // Minimized to tray
    void closeEvent(QCloseEvent *event);

private slots:
    // slot for trayicon activated
    void slotIconActivated(QSystemTrayIcon::ActivationReason reason);
    void slotCloseFromTray();

    // slots for signals
    void slotCurrentUnpackFile(QString strCurrentFile);
    void slotUnpackEnd();
    void slotCopyDir(QString strCurrentFile);
    void slotCopyDirEnd();
    void slotOnDownloadEnd();
    void slotTestGameInstall();
    void slotTestGameDataInstall();
    void slotSetDownloadLabel(QString strLabel);

    // Metod's to start downloading
    void DownloadLevel(QString strSqlTable, QString strGameDirPath);
    void DownloadMod(QString strSqlTable);

    // Slots to update download progress
    void slotOnUpdateProgressBar(qint64 bytesReceived, qint64 bytesTotal);

    // Download from MediaFire
    void GetDynamicUrlModDataFromMediaFire(QString strUrlMediaFire);
    void onFinishModDataMediaFire(QNetworkReply *reply);
    void GetDynamicUrlLevelDataFromMediaFire(QString strUrlMediaFire);
    void onFinishLevelMediaFire(QNetworkReply *reply);

    // ********
    // Ui slots
    // ********

    // Change screenshots ant row index slots
    void on_tableWidget_fe_official_pressed(const QModelIndex &index);
    void on_tableWidget_se_official_pressed(const QModelIndex &index);
    void on_table_fe_bestmaps_pressed(const QModelIndex &index);
    void on_table_se_bestmaps_pressed(const QModelIndex &index);
    void on_table_fe_usersmaps_pressed(const QModelIndex &index);
    void on_table_se_usersmaps_pressed(const QModelIndex &index);
    void on_table_se_hqmaps_pressed(const QModelIndex &index);
    void on_table_fe_mods_pressed(const QModelIndex &index);
    void on_table_se_mods_pressed(const QModelIndex &index);
    void on_tabWidget_internal_fe_currentChanged(int index);
    void on_tabWidget_internal_se_currentChanged(int index);

    // Start levels and mods slits
    void on_pushButton_fe_bestmaps_start_level_clicked();
    void on_pushButton_fe_official_start_level_clicked();
    void on_pushButton_fe_usersmap_start_level_clicked();
    void on_pushButton_fe_mods_sttart_mod_clicked();
    void on_pushButton_se_official_start_level_clicked();
    void on_pushButton_se_bestmaps_start_level_clicked();
    void on_pushButton_se_hqmaps_start_level_clicked();
    void on_pushButton_se_usersmaps_start_level_clicked();
    void on_pushButton_se_mods_start_mod_clicked();

    // Main page slots
    void on_pushButton_install_clicked();
    void on_pushButton_install_gamedata_clicked();
    void on_pushButton_update_db_clicked();

    // Download conten slots
    void on_pushButton_fe_bestmaps_download_clicked();
    void on_pushButton_fe_usersmap_download_clicked();
    void on_pushButton_fe_mods_download_clicked();
    void on_pushButton_se_bestmaps_download_clicked();
    void on_pushButton_se_hqmaps_download_clicked();
    void on_pushButton_se_usersmaps_download_clicked();
    void on_pushButton_se_mods_download_clicked();

    // XPLUS state changed slots
    void on_checkBox_fe_official_use_xplus_stateChanged(int arg);
    void on_checkBox_fe_bestmaps_use_xplus_stateChanged(int arg);
    void on_checkBox_fe_usersmap_use_xplus_stateChanged(int arg);
    void on_checkBox_se_official_use_xplus_stateChanged(int arg);
    void on_checkBox_se_bestmaps_use_xplus_stateChanged(int arg);
    void on_checkBox_se_hqmaps_use_xplus_stateChanged(int arg);
    void on_checkBox_se_usersmaps_use_xplus_stateChanged(int arg);

    // Difficulity combobox slots
    void on_comboBox_fe_official_difficulty_currentIndexChanged(int index);
    void on_comboBox_fe_bestmaps_difficulty_currentIndexChanged(int index);
    void on_comboBox_fe_usersmaps_difficulty_currentIndexChanged(int index);
    void on_comboBox_fe_mods_difficulty_currentIndexChanged(int index);
    void on_comboBox_se_official_difficulty_currentIndexChanged(int index);
    void on_comboBox_se_bestmaps_difficulty_currentIndexChanged(int index);
    void on_comboBox_se_hqmaps_difficulty_currentIndexChanged(int index);
    void on_comboBox_se_usersmaps_difficulty_currentIndexChanged(int index);   
    void on_comboBox_se_mods_difficulty_currentIndexChanged(int index);

    // Cancel download
    // void on_pushButton_add_user_map_clicked();
    void on_pushButton_fe_bestmaps_download_cancel_clicked();
    void on_pushButton_fe_usersmap_download_cancel_clicked();
    void on_pushButton_fe_mods_download_cancel_clicked();
    void on_pushButton_se_bestmaps_download_cancel_clicked();
    void on_pushButton_se_hqmaps_download_cancel_clicked();
    void on_pushButton_se_usersmaps_download_cancel_clicked();
    void on_pushButton_se_mods_download_cancel_clicked();

    // Themes change
    void on_comboBox_themes_currentIndexChanged(int index);
    // Downloads Server change
    void on_comboBox_server_currentIndexChanged(int index);

private:
    // ui
    Ui::MainWindow *ui;
    QMenu   *menu;
    QAction *viewWindow;
    QAction *hideWindow;
    QAction *quitAction;
    QSystemTrayIcon *trayIcon;
    // Internal funcrions
    void  SetVars();
    bool  SearchDB();
    void  ReScanLocalDB();
    void  HideProgressBars();
    void  HideLabelsText();
    bool  TestDBupdateNeed();
    void  MakeDifficultyScript(int iDifficulty, QString strPath);
    void  StartLevel(QString strSqlTable, int iXPLUS, int iDifficulty);
    void  StartMod(QString strSqlTable, int iDifficulty);
    void  FillTable(QString strSqlTable, QString strTableView);
    void  ForceFillingAllTable();
    void  MsgBox(int iType, QString strMessage);
    void  SleepMSecs(int iMsecs);
    void  SetImagesFromSql(QString strSqlTable,QString strLabelImg1,QString strLabelImg2);
    void  FE_UseXPLUS(int arg, int iTable, bool bInstalled);
    void  SE_UseXPLUS(int arg, int iTable, bool bInstalled);
    void  GetDistroFlag();
    void  SetComboBoxText();
    QString exec(const char* cmd);

    // Used classes and vars
    Downloader    m_downloader; // download class
#ifdef PLATFORM_UNIX
    FindInHomeDir m_findinhome;
#endif
    CopyDir       m_copydir;
    QProcess      *process;
    QMessageBox   *msgBox;
    QString       strMSG;

    // Path's
    QString strExecDirPath;
    QString strGameDirFEpath;
    QString strGameDirSEpath;
    QString strHomeDirPath;
    QString strRunnerDBPath;
    QString strRunnerDirPath;
    QString strEngineDirPath;

    // Path's suffix
#ifdef PLATFORM_WIN32
    QString strRunnerDirPathSuffix  = + "/AppData/Local/Serious-Runner";
#else
    QString strRunnerDirPathSuffix  = "/.local/share/Serious-Runner";
    QString strEngineDirPathSuffix  = "/.local/share/Serious-Engine";
#endif

    // Flags
    bool bUseSystemPath             = false;
    bool bFirstRun                  = false;
    bool bInstallGameOk             = false;
    bool bInstallGameDataOk         = false;
    bool bFE_XPLUSInstallOk         = false;
    bool bSE_XPLUSInstallOk         = false;
    bool bCloseFromTray             = false;
    bool bUpdateDB                  = false;
    int  iDistoFlag                 = 0;
    int  iTheme                     = 0;
    int  iServer                    = 0;
    int  iGameIndex                 = 0;

    // Indexes for tables and progress bar
    int iTableRowIndex;
    int iProgressBarIndex           = 0;
    int iUnpackProgressIndex        = 0;
    int iCopyDirProgressIndex       = 0;

    // Remote DB size
    int iRemoteSizeDB               = 0;

    // XPLUS and Difficulity
    int iFE_StartWithXplus[3]       = {0,0,0};
    int iSE_StartWithXplus[4]       = {0,0,0,0};
    int iFE_StartWithDifficulty[4]  = {0,0,0,0};
    int iSE_StartWithDifficulty[5]  = {0,0,0,0,0};

    // MediaFire urls
    QString strUrlMediaFire[2];

    // DB tables for Status scan
    QString strDBTablesForScan[7]   = {"fe_bestmaps", "fe_usersmaps", "fe_mods", "se_bestmaps", "se_hqmaps", "se_usersmaps", "se_mods"};
    // Tables middle Names
    QString strTablesMiddleNames[9] = {"fe_official", "fe_bestmaps", "fe_usersmaps", "fe_mods", "se_official", "se_bestmaps", "se_hqmaps", "se_usersmaps", "se_mods"};

    // Games gro files
    QString strFEfiles[6] {"1_00c.gro", "1_00c_Logo.gro", "1_00c_scripts.gro", "1_00_ExtraTools.gro", "1_00_music.gro", "1_04_patch.gro"};
    QString strSEfiles[8] {"1_04_patch.gro", "1_07_tools.gro", "SE1_00.gro", "SE1_00_Extra.gro", "SE1_00_ExtraTools.gro", "SE1_00_Levels.gro", "SE1_00_Logo.gro", "SE1_00_Music.gro"};

    // Download url
    QString strRunnerDBurl          = "https://raw.githubusercontent.com/tx00100xt/Serious-Runner/main/DB/seriousrunner.db";
    QString strGameSettingsUrl      = "https://raw.githubusercontent.com/tx00100xt/serioussam-maps/main/Settings/SeriousSamSettings.tar.xz";
    //QString strGameSettingsUrl    = "https://archive.org/download/serious-sam-settings/SeriousSamSettings.tar.xz";
    //QString strRunnerDBurl        = "https://archive.org/download/seriousrunner/seriousrunner.db";

};
#endif // MAINWINDOW_H
