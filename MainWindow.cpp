#include "MainWindow.h"
#include "qnetworkreply.h"
#include "ui_MainWindow.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QSplashScreen>
#include "Classes/Types.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Tray installation
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/Icons/seriousrunner.png"));
    trayIcon->setToolTip("Serious Runner" "\n"
                         "This program that allows you to download\n"
                         "content for the game created by users and run it.");
    // Tray menu
    menu = new QMenu(this);
    viewWindow = new QAction(tr("Show window"), this);
    hideWindow = new QAction(tr("Hide window"), this);
    quitAction = new QAction(tr("Exit"), this);

    // connect click signals on menu items to the corresponding slots.
    // The first menu item expands the application from the tray,
    // and the second menu item terminates the application.
    connect(viewWindow, SIGNAL(triggered()), this, SLOT(show()));
    connect(hideWindow, SIGNAL(triggered()), this, SLOT(hide()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(slotCloseFromTray()));
    menu->addAction(viewWindow);
    menu->addAction(hideWindow);
    menu->addAction(quitAction);

    // Set context menu to icon
    // and show the application icon in the tray
    trayIcon->setContextMenu(menu);
    trayIcon->show();

    // connect the icon click signal to the tray icon click handler
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(slotIconActivated(QSystemTrayIcon::ActivationReason)));

    // Splash Screen
    QPixmap pixmap(":/Images/runner-splash.png");
    QSplashScreen splash(pixmap, Qt::WindowStaysOnTopHint);
    splash.show();
    SleepMSecs(100);

    // Program initialization
    SetVars();
    RequestFileSize(strRunnerDBurl);
    HideProgressBars();
    HideLabelsText();
    SearchDB();
    slotTestGameInstall();
    slotTestGameDataInstall();
    ReScanLocalDB();
    splash.close();

    // Downloader connect
    connect(&m_downloader, &Downloader::signalUpdateDownloadProgress, this, &MainWindow::slotOnUpdateProgressBar);
    connect(&m_downloader, &Downloader::signalCurrentUnpackFile, this, &MainWindow::slotCurrentUnpackFile);
    connect(&m_downloader, &Downloader::signalUnpackEnd, this, &MainWindow::slotUnpackEnd);
    connect(&m_downloader, &Downloader::signalDownloadProgressEnd, this, &MainWindow::slotOnDownloadEnd);
    connect(&m_downloader, &Downloader::signalCopyDir, this, &MainWindow::slotCopyDir);
    connect(&m_downloader, &Downloader::signalCopyDirEnd, this, &MainWindow::slotCopyDirEnd);
    connect(&m_downloader, &Downloader::signalTestGameInstall, this, &MainWindow::slotTestGameInstall);
    connect(&m_downloader, &Downloader::signalSetDownloadLabel, this, &MainWindow::slotSetDownloadLabel);

    // CopyDir connect
    connect(&m_copydir, &CopyDir::signalCopyDir, this, &MainWindow::slotCopyDir);
    connect(&m_copydir, &CopyDir::signalCopyDirEnd, this, &MainWindow::slotCopyDirEnd);

    // TestGameDataInstall
    connect(this, &MainWindow::signalTestGameDataInstall, this, &MainWindow::slotTestGameDataInstall);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ********************************************************************************************
// ************************************ Tray methods ******************************************
// ********************************************************************************************

// Method that handles the application window close event
void MainWindow::closeEvent(QCloseEvent *event)
{
    if(this->isVisible() && !bCloseFromTray){
        event->ignore();
        this->hide();
        QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(QSystemTrayIcon::Information);
        trayIcon->showMessage("Serious Runner",
            tr("The application is minimized to tray. In order to, "
                "maximize the application window, click on the application icon in the tray"), icon, 2000);
    }
}

// Method that handles clicking on the application tray icon
void MainWindow::slotIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason){
    case QSystemTrayIcon::Trigger:
        if(!this->isVisible()){
            this->show();
        } else {
                this->hide();
        }
        break;
    default:
        break;
    }
}

// Set flags for close programm from tray
void MainWindow::slotCloseFromTray(){
    bCloseFromTray = true;
    close();
}

// ********************************************************************************************
// *********************************** Initialisations ****************************************
// ********************************************************************************************

// Set vars
void MainWindow::SetVars(){
    // Get paths
    strHomeDirPath       = QDir::homePath();
    strExecDirPath       = QCoreApplication::applicationFilePath();
    // Set pathd
    strRunnerDirPath     = strHomeDirPath + strRunnerDirPathSuffix;
    strEngineDirPath     = strHomeDirPath + strEngineDirPathSuffix;
    strGameDirFEpath     = strRunnerDirPath + "/SamTFE";
    strGameDirSEpath     = strRunnerDirPath + "/SamTSE";
    strRunnerDBPath      = strRunnerDirPath + "/DB/seriousrunner.db";
    // Test system install
    if(strExecDirPath.contains("/usr/bin")) {
        bUseSystemPath = true;
    }
    if(QFile(strGameDirFEpath + "/Mods/XPLUS/XPlus_All.gro").exists()) {
        bFE_XPLUSInstallOk = true;
    }
    if(QFile(strGameDirSEpath + "/Mods/XPLUS/z_ChainsawHD.gro").exists()) {
        bSE_XPLUSInstallOk = true;
    }
    // !!! Comment it out if you want to run mods bypassing the menu
    ui->comboBox_fe_mods_difficulty->hide();
    ui->comboBox_se_mods_difficulty->hide();
    // get os flag (by name)
    GetDistroFlag();
    // get saved theme
    QString strTheme;
    QString strFileName = strRunnerDirPath +"/Theme.ini";
    QFile inputFile(strFileName);
    if (inputFile.open(QIODevice::ReadOnly))
    {
       QTextStream in(&inputFile);
       while (!in.atEnd())
       {
          strTheme = in.readLine();
       }
       inputFile.close();
       iTheme = strTheme.toInt();
    }
    // set combo box
    SetComboBoxText();
}

// Get Disto name and set flag for SQl download table
void MainWindow::GetDistroFlag()
{
    QString strOS=exec("cat /etc/*release|grep NAME");
    // MsgBox(INFO, strOS); // For Debug

    // Set Distro flag
    if(strOS.contains("FreeBSD")) {
        iDistoFlag = FREEBSD;
    }else if(strOS.contains("Alpine")) {
        iDistoFlag = ALPINE;
    }else if(strOS.contains("Linux")) {
        iDistoFlag = LINUX;
    }else{
        iDistoFlag = OTHER;
    }
    // MsgBox(INFO, QString::number(iDistoFlag)); // For Debug
}

// Set combo box text on center, because on Arch linux we have bug with text on combobox
void MainWindow::SetComboBoxText()
{
    QString strCcomboBoxName;
    for (int i = 0 ; i < 9 ; ++i) {
        // Example: comboBox_fe_official_difficulty
        strCcomboBoxName = "comboBox_" +strTablesMiddleNames[i] + "_difficulty";
        MainWindow::findChild<QComboBox*>(strCcomboBoxName)->clear();
        MainWindow::findChild<QComboBox*>(strCcomboBoxName)->setEditable(true);
        MainWindow::findChild<QComboBox*>(strCcomboBoxName)->lineEdit()->setReadOnly(true);
        MainWindow::findChild<QComboBox*>(strCcomboBoxName)->lineEdit()->setAlignment(Qt::AlignCenter);
        MainWindow::findChild<QComboBox*>(strCcomboBoxName)->lineEdit()->setFont(QFont ("Sans Serif", 12));
        MainWindow::findChild<QComboBox*>(strCcomboBoxName)->setFont(QFont ("Sans Serif", 12));
        MainWindow::findChild<QComboBox*>(strCcomboBoxName)->addItem("Tourist");
        MainWindow::findChild<QComboBox*>(strCcomboBoxName)->addItem("Easy");
        MainWindow::findChild<QComboBox*>(strCcomboBoxName)->addItem("Normal");
        MainWindow::findChild<QComboBox*>(strCcomboBoxName)->addItem("Hard");
        MainWindow::findChild<QComboBox*>(strCcomboBoxName)->addItem("Serious");
        // MsgBox(INFO,strCcomboBoxName);// For Debug
        for (int j = 0 ; j <  MainWindow::findChild<QComboBox*>(strCcomboBoxName)->count() ; ++j) {
            MainWindow::findChild<QComboBox*>(strCcomboBoxName)->setItemData(j, Qt::AlignCenter, Qt::TextAlignmentRole);
        }
    }
    // Create themes combo box
    ui->comboBox_themes->clear();
    ui->comboBox_themes->setEditable(true);
    ui->comboBox_themes->lineEdit()->setReadOnly(true);
    ui->comboBox_themes->lineEdit()->setAlignment(Qt::AlignCenter);
    ui->comboBox_themes->lineEdit()->setFont(QFont ("Sans Serif", 12));
    ui->comboBox_themes->setFont(QFont ("Sans Serif", 12));
    // themes
    ui->comboBox_themes->addItem("Default");
    ui->comboBox_themes->addItem("Amoled");
    ui->comboBox_themes->addItem("Aqua.");
    ui->comboBox_themes->addItem("ConsoleStyle");
    ui->comboBox_themes->addItem("ElegantDark");
    ui->comboBox_themes->addItem("MacOS");
    ui->comboBox_themes->addItem("ManjaroMix");
    ui->comboBox_themes->addItem("MaterialDark");
    ui->comboBox_themes->addItem("NeonButtons");
    ui->comboBox_themes->addItem("Ubuntu");
    ui->comboBox_themes->setCurrentIndex(iTheme);
    // set theme
    on_comboBox_themes_currentIndexChanged(iTheme);
}

// Test DB update need?
bool MainWindow::TestDBupdateNeed()
{
    int iLocalDBsize = 0;
    QFile scanFile(strRunnerDBPath);
    if(scanFile.open(QIODevice::ReadOnly)){
        iLocalDBsize = scanFile.size();
        scanFile.close();
    }
    if(iLocalDBsize == iRemoteSizeDB){
        return true;
    }
    return false;
}

// Search and install DB
bool MainWindow::SearchDB(){
    if (!QFile(strRunnerDBPath).exists()){
        if (!QDir(strRunnerDirPath).exists()){
            QDir().mkdir(strRunnerDirPath);
        }
        if (!QDir(strRunnerDirPath + "/DB").exists()){
            QDir().mkdir(strRunnerDirPath + "/DB");
        }
        QString strDBpath;
        QString strDBfilename = "seriousrunner.db";
        if (bUseSystemPath){
            strDBpath = "/usr/share/seriousrunner/DB";
        } else {
            strDBpath = m_findinhome.FindFileInHomeDir("seriousrunner.db");
            if (strDBpath == ""){
                QMessageBox::critical(nullptr, "Serious Runner", "Serious Runner did not find the database in the home directory.\n"
                    "Please place file seriousrunner.db somewhere in your home directory and restart the program.");
                return false;
            }
        }
        QString strDBpathWithFile = strDBpath + "/" + strDBfilename;
        if (!QFile(strDBpathWithFile).exists()){
            QMessageBox::critical(nullptr, "Serious Runner", "Serious Runner did not find the database seriousrunner.db\n"
                "Please place file seriousrunner.db somewhere in your home directory and restart the program.");
            return false;
        } else {
            if(!QFile::copy(strDBpathWithFile, strRunnerDBPath)){
                QMessageBox::critical(nullptr, "Serious Runner", "Serious Runner was unable to copy the database to the home directory.\n"
                    "Please place file seriousrunner.db in '" + strRunnerDBPath + "' and restart the program.");
                return false;
            } else {
                QMessageBox::information(nullptr, "Serious Runner", "Install Serious Runner DB Done!");
                QPixmap pixmap(":/Images/ok.png");
                ui->label_db_status_image->setPixmap(pixmap);
            }
        }
        bFirstRun = true;
    } else {
        QPixmap pixmap(":/Images/ok.png");
        ui->label_db_status_image->setPixmap(pixmap);
    }
    return true;
}

// Hide Progress Bars
void MainWindow::HideProgressBars()
{
    // Reset value
    ui->progressBar_install->setValue(0);
    ui->progressBar_fe_bestmaps_download->setValue(0);
    ui->progressBar_fe_usersmaps_download->setValue(0);
    ui->progressBar_fe_mods_download->setValue(0);
    ui->progressBar_se_bestmaps_download->setValue(0);
    ui->progressBar_se_hqmaps_download->setValue(0);
    ui->progressBar_se_usersmaps_download->setValue(0);
    ui->progressBar_se_mods_download->setValue(0);
    // Hide
    ui->progressBar_install->hide();
    ui->progressBar_fe_bestmaps_download->hide();
    ui->progressBar_fe_usersmaps_download->hide();
    ui->progressBar_fe_mods_download->hide();
    ui->progressBar_se_bestmaps_download->hide();
    ui->progressBar_se_hqmaps_download->hide();
    ui->progressBar_se_usersmaps_download->hide();
    ui->progressBar_se_mods_download->hide();
}

// Hide Labels Text
void MainWindow::HideLabelsText()
{
    // Reset Labels text
    ui->label_progress->setText("");
    ui->label_fe_mod_download->setText("");
    ui->label_fe_bestmaps_download->setText("");
    ui->label_fe_usersmaps_download->setText("");
    ui->label_se_bestmaps_download->setText("");
    ui->label_se_hqmaps_download->setText("");
    ui->label_se_usersmaps_download->setText("");
    ui->label_se_mod_download->setText("");
}

// Rescan Local DB when programm start and after download
void MainWindow::ReScanLocalDB()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(strRunnerDBPath);
    if(!db.open()){
        MsgBox(ERROR, "Error: Open SQL databese Failed!");
        return;
    }
    QSqlQuery qry(db);
    qry.setForwardOnly(true);

    QString strFile, strStatus, strStatusInDB;
    for(int i = 0; i < 7; i++){
        qry.prepare("SELECT COUNT(*) FROM " + strDBTablesForScan[i]);
        if (!qry.exec()){
            MsgBox(ERROR, "Error: " + qry.lastError().text());
            db.close();
            return;
        }
        qry.next();
        int iRows = qry.value(0).toInt();
        for(int j = 1; j <= iRows; j++){
            QString strIndex, strSelect;
            if(strDBTablesForScan[i].contains("maps")){
                strIndex  = "level_num";
                strSelect = "level_gro,size,Status";
            } else {
                strIndex  = "mod_num";
                strSelect = "mod_testfile,testfile_size,Status";
            }
                qry.prepare("Select " + strSelect + "  FROM " + strDBTablesForScan[i] + " WHERE " + strIndex + " = :index_num");
                qry.bindValue(":index_num", j);
                if (!qry.exec()){
                    MsgBox(ERROR, "Error: " + qry.lastError().text());
                }
                qint64 iSizeInDB = 0;
                QString strTestFile, strStatus, strMD5sum;
                while (qry.next()) {
                    strTestFile = qry.value(0).toString();
                    iSizeInDB = qry.value(1).toInt();
                    strStatusInDB = qry.value(2).toString();
                }
                if(strDBTablesForScan[i].contains("fe_")){
                    strFile = strRunnerDirPath + "/SamTFE/" + strTestFile;
                } else {
                    strFile = strRunnerDirPath + "/SamTSE/" + strTestFile;
                }
                qint64 iSize = 0;
                QFile scanFile(strFile);
                if (scanFile.open(QIODevice::ReadOnly)){
                    iSize = scanFile.size();
                    scanFile.close();
                }
                if((iSize !=0 ) && (iSizeInDB == iSize)){
                    strStatus = "Installed";
                } else {
                    strStatus = "---";
                }
                if((strStatus.contains("---") && strStatusInDB.contains("Installed")) || (strStatus.contains("Installed") && strStatusInDB.contains("---"))){
                    qry.first();
                    qry.prepare("UPDATE " + strDBTablesForScan[i] + " SET Status = :Status"
                                " WHERE " + strIndex + " = :index_num");
                    qry.bindValue(":index_num", j);
                    qry.bindValue(":Status", strStatus);
                    if (!qry.exec()){
                        MsgBox(ERROR, "Error: " + qry.lastError().text());
                        db.close();
                        return;
                    }
                }
        }
    }
    db.close();
}

// ********************************************************************************************
// ************************************ connect slots *****************************************
// ********************************************************************************************

// Show unpack progress
void MainWindow::slotCurrentUnpackFile(QString strCurrentFile)
{
    switch(iUnpackProgressIndex)
    {
        case INSTALL_UNPACKPROGRESS:
              ui->label_progress->setText(strCurrentFile);
            break;
        case FE_MODS_UNPACKPROGRESS:
              ui->label_fe_mod_download->setText(strCurrentFile);
            break;
        case SE_MODS_UNPACKPROGRESS:
              ui->label_se_mod_download->setText(strCurrentFile);
            break;
        default:
            break;
    }
    SleepMSecs(50);
}

// Show Copy Dir progress
void MainWindow::slotCopyDir(QString strCurrentFile)
{
    switch(iCopyDirProgressIndex)
    {
        case INSTALL_COPYPROGRESS:
              ui->label_progress->setText(strCurrentFile);
            break;
        case FE_MODS_COPYPROGRESS:
              ui->label_fe_mod_download->setText(strCurrentFile);
            break;
        case SE_MODS_COPYPROGRESS:
              ui->label_se_mod_download->setText(strCurrentFile);
            break;
        default:
            break;
    }
    SleepMSecs(50);
}

// Show download progress
void MainWindow::slotOnUpdateProgressBar(qint64 bytesReceived, qint64 bytesTotal)
{

    switch(iProgressBarIndex)
    {
        case INSTALL_PBAR:
              ui->progressBar_install->setMaximum(bytesTotal);
              ui->progressBar_install->setValue(bytesReceived);
            break;
        case FE_BESTMAPS_PBAR:
              ui->progressBar_fe_bestmaps_download->setMaximum(bytesTotal);
              ui->progressBar_fe_bestmaps_download->setValue(bytesReceived);
            break;
        case FE_USERSMAPS_PBAR:
              ui->progressBar_fe_usersmaps_download->setMaximum(bytesTotal);
              ui->progressBar_fe_usersmaps_download->setValue(bytesReceived);
            break;
        case FE_MODS_PBAR:
              ui->progressBar_fe_mods_download->setMaximum(bytesTotal);
              ui->progressBar_fe_mods_download->setValue(bytesReceived);
            break;
        case SE_BESTMAPS_PBAR:
              ui->progressBar_se_bestmaps_download->setMaximum(bytesTotal);
              ui->progressBar_se_bestmaps_download->setValue(bytesReceived);
            break;
        case SE_HQMAPS_PBAR:
              ui->progressBar_se_hqmaps_download->setMaximum(bytesTotal);
              ui->progressBar_se_hqmaps_download->setValue(bytesReceived);
            break;
        case SE_USERSMAPS_PBAR:
              ui->progressBar_se_usersmaps_download->setMaximum(bytesTotal);
              ui->progressBar_se_usersmaps_download->setValue(bytesReceived);
            break;
        case SE_MODS_PBAR:
              ui->progressBar_se_mods_download->setMaximum(bytesTotal);
              ui->progressBar_se_mods_download->setValue(bytesReceived);
            break;
        default:
            break;
    }
}

// Set Download Label
void MainWindow::slotSetDownloadLabel(QString strLabel)
{
    switch(iProgressBarIndex)
    {
        case INSTALL_PBAR:
              ui->label_progress->setText(strLabel);
            break;
        case FE_BESTMAPS_PBAR:
              ui->label_fe_bestmaps_download->setText(strLabel);
            break;
        case FE_USERSMAPS_PBAR:
              ui->label_fe_usersmaps_download->setText(strLabel);
            break;
        case FE_MODS_PBAR:
              ui->label_fe_mod_download->setText(strLabel);
            break;
        case SE_BESTMAPS_PBAR:
              ui->label_se_bestmaps_download->setText(strLabel);
            break;
        case SE_HQMAPS_PBAR:
              ui->label_se_hqmaps_download->setText(strLabel);
            break;
        case SE_USERSMAPS_PBAR:
              ui->label_se_usersmaps_download->setText(strLabel);
            break;
        case SE_MODS_PBAR:
              ui->label_se_mod_download->setText(strLabel);
            break;
        default:
            break;
    }
}

// Test game installation
void MainWindow::slotTestGameInstall()
{
    bInstallGameOk = true;
    if(!QFile().exists(strGameDirFEpath + "/Bin/libEngine.so") || !QFile().exists(strGameDirSEpath + "/Bin/libEngineMP.so")){
        bInstallGameOk = false;
    }
    if(!QFile().exists(strGameDirFEpath + "/VirtualTrees/BasicVirtualTree.vrt") || !QFile().exists(strGameDirSEpath + "/VirtualTrees/BasicVirtualTree.vrt")){
        bInstallGameOk = false;
    }
    if(bInstallGameOk){
        QPixmap pixmap(":/Images/ok.png");
        ui->label_install_game_image->setPixmap(pixmap);
    }else{
        QMessageBox::warning(nullptr, "Serious Runner",  "Please repeat the installation of the game");
    }
}

// Test game data installation
void MainWindow::slotTestGameDataInstall()
{ 
    bInstallGameDataOk = true;
    if(!QFile().exists(strGameDirFEpath + "/1_00_music.gro") || !QFile().exists(strGameDirSEpath + "/SE1_00_Levels.gro")){
        bInstallGameDataOk = false;
    }
    if(!QFile().exists(strGameDirFEpath + "/1_04_patch.gro") || !QFile().exists(strGameDirSEpath + "/1_04_patch.gro")){
        bInstallGameDataOk = false;
    }
    if(bInstallGameDataOk){
        QPixmap pixmap(":/Images/ok.png");
        ui->label_install_gamedata_image->setPixmap(pixmap);
    }else{
        QMessageBox::warning(nullptr, "Serious Runner",  "Please repeat the searh data of the game");
    }
}

// Hide labels, rescan DB and fill tables after unpack archive
void MainWindow::slotUnpackEnd()
{
    HideLabelsText();
    if (!bUpdateDB){
        ReScanLocalDB();
    }
    ForceFillingAllTable();
    m_downloader.locked_cancel = false;
}

// Hide progressbars,rescan DB and fill tables after download
void MainWindow::slotOnDownloadEnd()
{
    HideProgressBars();
    SleepMSecs(50);
    if (bUpdateDB){
        MsgBox(INFO, "The program will restart");
        qApp->quit();
        QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
        slotCloseFromTray();
    }
    slotUnpackEnd();
}

// Hide labels after Copy Dir
void MainWindow::slotCopyDirEnd()
{
     HideLabelsText();
}

// ********************************************************************************************
// ******************************** Main page bottons slots ***********************************
// ********************************************************************************************

void MainWindow::on_pushButton_install_clicked()
{
    if(bInstallGameOk){
        MsgBox(INFO, "Game already installed");
        return;
    }
    if(m_downloader.locked_action) {return;}

    // Get download link from sql
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(strRunnerDBPath);
    if(!db.open()){
        MsgBox(ERROR, "Error: Open SQL databese Failed!");
        return;
    }
    QSqlQuery qry(db);
    qry.setForwardOnly(true);
    qry.prepare("Select download_link_bin,distro FROM game_bin WHERE os_num = :os_num");
    qry.bindValue(":os_num", iDistoFlag + 1);
    if (!qry.exec()){
        MsgBox(ERROR, "Error: " + qry.lastError().text());
        db.close();
        return;
    }
    QString strDownloadUrlBin, strDistro;
    while (qry.next()) {
        strDownloadUrlBin = qry.value(0).toString();
        strDistro = qry.value(1).toString();
    }
    db.close();

    // Try download
    iProgressBarIndex = INSTALL_PBAR;
    iUnpackProgressIndex = INSTALL_UNPACKPROGRESS;
    iCopyDirProgressIndex  = INSTALL_COPYPROGRESS;
    ui->progressBar_install->setValue(0);
    ui->progressBar_install->show();
    m_downloader.GetGameSettings( strRunnerDirPath, strDownloadUrlBin, strGameSettingsUrl);
}

void MainWindow::on_pushButton_install_gamedata_clicked()
{
    if(!bInstallGameOk){
        MsgBox(WARN, "Please install the game before installing game data");
        return;
    }
    if(bInstallGameDataOk){
        MsgBox(INFO, "Game data already installed");
        return;
    }
    if(m_downloader.locked_action) {return;}
    iProgressBarIndex = INSTALL_PBAR;
    iUnpackProgressIndex = INSTALL_UNPACKPROGRESS;
    iCopyDirProgressIndex  = INSTALL_COPYPROGRESS;

    QString strPathFE = m_findinhome.FindFileInHomeDir(strFEfiles[4]);
    QString strPathSE = m_findinhome.FindFileInHomeDir(strSEfiles[5]);

    // Copy game data from found directory after search
    m_copydir.CopyAndReplaceFolderContents(strPathFE + "/Help", strRunnerDirPath + "/SamTFE/Help", false);
    m_copydir.CopyAndReplaceFolderContents(strPathFE + "/Levels", strRunnerDirPath + "/SamTFE/Levels", false);
    m_copydir.CopyAndReplaceFolderContents(strPathSE + "/Help", strRunnerDirPath + "/SamTSE/Help", false);
    m_copydir.CopyAndReplaceFolderContents(strPathSE + "/Levels", strRunnerDirPath + "/SamTSE/Levels", false);

    // Copy gro files from found directory after search
    for(int i = 0; i < 6; i++){
       if(QFile(strPathFE + "/" + strFEfiles[i]).exists()){
            QFile::copy(strPathFE + "/" + strFEfiles[i], strRunnerDirPath + "/SamTFE/" + strFEfiles[i]);
       } else {
           ui->label_progress->setText("");
           MsgBox(ERROR, "File: " + strPathFE + "/" + strFEfiles[i] + " not found!");
           return;
       }
    }
    for(int i = 0; i < 8; i++){
        if(QFile(strPathSE + "/" + strSEfiles[i]).exists()){
             QFile::copy(strPathSE + "/" + strSEfiles[i], strRunnerDirPath + "/SamTSE/" + strSEfiles[i]);
        } else {
            ui->label_progress->setText("");
            MsgBox(ERROR, "File: " + strPathSE + "/" + strSEfiles[i] + " not found!");
            return;
        }
    }
    emit signalTestGameDataInstall();
    ui->label_progress->setText("");
    MsgBox(INFO, "Search and Install Game Data Done!");
    QPixmap pixmap(":/Images/ok.png");
    ui->label_install_gamedata_image->setPixmap(pixmap);
}

void MainWindow::on_pushButton_update_db_clicked()
{
    if(TestDBupdateNeed()) {
        MsgBox(INFO, "The latest version of the database is installed. No update required!");
        return;
    }
    if(iRemoteSizeDB == 0) {
        MsgBox(WARN, "Connection problem, try again later!");
        return;
    }
    //MsgBox(INFO,"Database update available: " + QString::number(iRemoteSizeDB));

    if(m_downloader.locked_action) {return;}
    MsgBox(WARN, "Attention! after loading the new database, you will need application\n"
                 "restart. The first start after that will take some time. Be patient."
                 "And don't! close the applation. Wait for the download to finish.");

    // Backup DB
    QDateTime date = QDateTime::currentDateTime();
    QString formattedTime = date.toString("-yyyyMMdd_hh-mm-ss.db");
    QFile::rename(strRunnerDBPath, strRunnerDirPath + "/DB/seriousrunner." + formattedTime);

    bUpdateDB = true;
    // Set bars and download DB
    iProgressBarIndex = INSTALL_PBAR;
    iUnpackProgressIndex = INSTALL_UNPACKPROGRESS;
    iCopyDirProgressIndex  = INSTALL_COPYPROGRESS;
    ui->progressBar_install->show();
    m_downloader.GetDB( strRunnerDirPath + "/DB", strRunnerDBurl);
}

// ********************************************************************************************
// ******************************* Get remote file size ***************************************
// ********************************************************************************************

void MainWindow::RequestFileSize(const QString &url)
{
    QNetworkRequest req;
    m_netmanager = new QNetworkAccessManager(this);
    req.setUrl(QUrl(url));
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    m_reply = m_netmanager->get(req);
    connect(m_reply, SIGNAL(metaDataChanged()), this, SLOT(FileSize()));
}

void MainWindow::FileSize()
{
    iRemoteSizeDB = m_reply->header(QNetworkRequest::ContentLengthHeader).toInt();
    m_reply->deleteLater();
    m_netmanager->deleteLater();
}

// ********************************************************************************************
// ********************************* Set images grom DB ***************************************
// ********************************************************************************************

// Set images
void MainWindow::SetImagesFromSql(QString strSqlTable,QString strLabelImg1,QString strLabelImg2)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(strRunnerDBPath);
    if(!db.open()){
        MsgBox(ERROR, "Open SQL databese Failed!");
        return;
    }
    QSqlQuery qry(db);
    qry.setForwardOnly(true);
    if(strSqlTable.contains("_mods")) {
        qry.prepare("SELECT img1,img2  FROM " + strSqlTable + " WHERE mod_num = :index_num");
    } else {
        qry.prepare("SELECT img1,img2  FROM " + strSqlTable + " WHERE level_num = :index_num");
    }
    qry.bindValue(":index_num", iTableRowIndex);
    if (!qry.exec()){
        MsgBox(ERROR, "Error: " + qry.lastError().text());
        db.close();
        return;
    }
    QByteArray outByteArray1;
    QByteArray outByteArray2;
    while (qry.next()) {
        outByteArray1 = qry.value( 0 ).toByteArray();
        outByteArray2 = qry.value( 1 ).toByteArray();
    }
    db.close();
    QPixmap outPixmap1 = QPixmap();
    QPixmap outPixmap2 = QPixmap();
    outPixmap1.loadFromData( outByteArray1 );
    outPixmap2.loadFromData( outByteArray2 );
    MainWindow::findChild<QLabel*>(strLabelImg1)->setPixmap(outPixmap1); MainWindow::findChild<QLabel*>(strLabelImg1)->show();
    MainWindow::findChild<QLabel*>(strLabelImg2)->setPixmap(outPixmap2); MainWindow::findChild<QLabel*>(strLabelImg2)->show();
}

// FE screenies fill
void MainWindow::on_tableWidget_fe_official_pressed(const QModelIndex &index)
{
    QItemSelectionModel* selectionModel = ui->tableWidget_fe_official->selectionModel();
    QModelIndexList selected = selectionModel->selectedRows(); iTableRowIndex = index.row() + 1;
    SetImagesFromSql("fe_official", "label_fe_official_image1", "label_fe_official_image2");
}

void MainWindow::on_table_fe_bestmaps_pressed(const QModelIndex &index)
{
    QItemSelectionModel* selectionModel = ui->table_fe_bestmaps->selectionModel();
    QModelIndexList selected = selectionModel->selectedRows(); iTableRowIndex = index.row() + 1;
    SetImagesFromSql("fe_bestmaps", "label_fe_bestmaps_image1", "label_fe_bestmaps_image2");
}

void MainWindow::on_table_fe_usersmaps_pressed(const QModelIndex &index)
{
    QItemSelectionModel* selectionModel = ui->table_fe_usersmaps->selectionModel();
    QModelIndexList selected = selectionModel->selectedRows(); iTableRowIndex = index.row() + 1;
    SetImagesFromSql("fe_usersmaps", "label_fe_usersmaps_image1", "label_fe_usersmaps_image2");
}

void MainWindow::on_table_fe_mods_pressed(const QModelIndex &index)
{
    QItemSelectionModel* selectionModel = ui->table_fe_mods->selectionModel();
    QModelIndexList selected = selectionModel->selectedRows(); iTableRowIndex = index.row() + 1;
    SetImagesFromSql("fe_mods", "label_fe_mods_image1", "label_fe_mods_image2");
}

// SE screenies fill
void MainWindow::on_tableWidget_se_official_pressed(const QModelIndex &index)
{
    QItemSelectionModel* selectionModel = ui->tableWidget_se_official->selectionModel();
    QModelIndexList selected = selectionModel->selectedRows(); iTableRowIndex = index.row() + 1;
    SetImagesFromSql("se_official", "label_se_official_image1", "label_se_official_image2");
}

void MainWindow::on_table_se_bestmaps_pressed(const QModelIndex &index)
{
    QItemSelectionModel* selectionModel = ui->table_se_bestmaps->selectionModel();
    QModelIndexList selected = selectionModel->selectedRows(); iTableRowIndex = index.row() + 1;
    SetImagesFromSql("se_bestmaps", "label_se_bestmaps_image1", "label_se_bestmaps_image2");
}

void MainWindow::on_table_se_hqmaps_pressed(const QModelIndex &index)
{
    QItemSelectionModel* selectionModel = ui->table_se_hqmaps->selectionModel();
    QModelIndexList selected = selectionModel->selectedRows(); iTableRowIndex = index.row() + 1;
    SetImagesFromSql("se_hqmaps", "label_se_hqmaps_image1", "label_se_hqmaps_image2");
}

void MainWindow::on_table_se_usersmaps_pressed(const QModelIndex &index)
{
    QItemSelectionModel* selectionModel = ui->table_se_usersmaps->selectionModel();
    QModelIndexList selected = selectionModel->selectedRows(); iTableRowIndex = index.row() + 1;
    SetImagesFromSql("se_usersmaps", "label_se_usersmaps_image1", "label_se_usersmaps_image2");
}

void MainWindow::on_table_se_mods_pressed(const QModelIndex &index)
{
    QItemSelectionModel* selectionModel = ui->table_se_mods->selectionModel();
    QModelIndexList selected = selectionModel->selectedRows(); iTableRowIndex = index.row() + 1;
    SetImagesFromSql("se_mods", "label_se_mods_image1", "label_se_mods_image2");
}

// ********************************************************************************************
// ********************************** Tab changed slots ***************************************
// ********************************************************************************************

void MainWindow::on_tabWidget_internal_fe_currentChanged(int index)
{
    switch(index)
    {
        case 1:
            FillTable("fe_bestmaps", "table_fe_bestmaps");
            break;
        case 2:
            FillTable("fe_usersmaps", "table_fe_usersmaps");
            break;
        case 3:
            FillTable("fe_mods", "table_fe_mods");
            break;
        default:
            break;
    }
}

void MainWindow::on_tabWidget_internal_se_currentChanged(int index)
{
    switch(index)
    {
        case 1:
            FillTable("se_bestmaps", "table_se_bestmaps");
            break;
        case 2:
            FillTable("se_hqmaps", "table_se_hqmaps");
            break;
        case 3:
            FillTable("se_usersmaps", "table_se_usersmaps");
            break;
        case 4:
            FillTable("se_mods", "table_se_mods");
            break;
        default:
            break;
    }
}

// ********************************************************************************************
// ******************************* Filling Tables from DB *************************************
// ********************************************************************************************

// Filling table from DB
void MainWindow::FillTable(QString strSqlTable, QString strTableView)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(strRunnerDBPath);
    if(!db.open()){
        MsgBox(ERROR, "Error: Open SQL databese Failed!");
        return;
    }
    QSqlQuery qry(db);
    qry.prepare("Select Name,Type,Author,Status FROM " + strSqlTable);
    qry.exec();
    Model *model = new Model;
    model->setQuery(qry);
    MainWindow::findChild<QTableView*>(strTableView)->setModel(model);;
    db.close();
}

// Filling all tables from DB
void MainWindow::ForceFillingAllTable()
{
     FillTable("fe_bestmaps", "table_fe_bestmaps");
     FillTable("fe_usersmaps", "table_fe_usersmaps");
     FillTable("fe_mods", "table_fe_mods");
     FillTable("se_bestmaps", "table_se_bestmaps");
     FillTable("se_hqmaps", "table_se_hqmaps");
     FillTable("se_usersmaps", "table_se_usersmaps");
     FillTable("se_mods", "table_se_mods");
}

// ********************************************************************************************
// **************************** Make difficulty script for game  ******************************
// ********************************************************************************************

void  MainWindow::MakeDifficultyScript(int iDifficulty, QString strPath)
{
    const QString strString = "gam_iStartDifficulty = " + QString::number(iDifficulty - 1) + "; // difficulty (-1=tourist, 0=easy, 1=normal, 2=hard, 3=serious";
    QFile qFile(strPath);
    if (qFile.open(QIODevice::WriteOnly)) {
      QTextStream out(&qFile); out << strString;
      qFile.close();
    }
    return;
}

// ********************************************************************************************
// *********************************** Start level slots **************************************
// ********************************************************************************************

void MainWindow::StartLevel(QString strSqlTable, int iXPLUS, int iDifficulty)
{
    if(!bInstallGameOk || !bInstallGameDataOk){
        MsgBox(WARN, "Please install the game before starting the level");
        return;
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(strRunnerDBPath);
    if(!db.open()){
        MsgBox(ERROR, "Error: Open SQL databese Failed!");
        return;
    }
    QSqlQuery qry(db);
    qry.setForwardOnly(true);
    qry.prepare("Select level_file,Status FROM " + strSqlTable + " WHERE level_num = :level_num");
    qry.bindValue(":level_num", iTableRowIndex);
    if (!qry.exec()){
        MsgBox(ERROR, "Error: " + qry.lastError().text());
        db.close();
        return;
    }
    QString strLevelFile, strStatus;
    while (qry.next()) {
        strLevelFile = qry.value(0).toString();
        strStatus = qry.value(1).toString();
    }
    db.close();

    if(!strStatus.contains("Installed")){
        MsgBox(WARN, "Before starting this level, you need to download it.");
        return;
    }

    QString strProgram;
    QStringList arguments;
    if(strSqlTable.contains("fe_")) {
        strProgram = strGameDirFEpath + "/Bin/SeriousSam";
        MakeDifficultyScript(iDifficulty, strGameDirFEpath + "/Scripts/Game_difficulty.ini");
        if(iXPLUS) {
            arguments << "+game" << "XPLUS";
         }
    } else {
        MakeDifficultyScript(iDifficulty, strGameDirSEpath + "/Scripts/Game_difficulty.ini");
        strProgram = strGameDirSEpath + "/Bin/SeriousSam";
        if(iXPLUS) {
           arguments << "+game" << "XPLUS";
        }
    }
    arguments << "+level" << strLevelFile;
    arguments << "+script" << "Scripts/Game_difficulty.ini";

    QProcess *Process = new QProcess(this);
    Process->start(strProgram, arguments);
    this->hide();
}

// Start FE level
void MainWindow::on_pushButton_fe_official_start_level_clicked()
{
    StartLevel("fe_official", iFE_StartWithXplus[FE_OFFICIAL], iFE_StartWithDifficulty[FE_OFFICIAL]);
}

void MainWindow::on_pushButton_fe_bestmaps_start_level_clicked()
{
    StartLevel("fe_bestmaps", iFE_StartWithXplus[FE_BESTMAPS], iFE_StartWithDifficulty[FE_BESTMAPS]);
}

void MainWindow::on_pushButton_fe_usersmap_start_level_clicked()
{
    StartLevel("fe_usersmaps", iFE_StartWithXplus[FE_USERSMAPS], iFE_StartWithDifficulty[FE_USERSMAPS]);
}

// Start SE level
void MainWindow::on_pushButton_se_official_start_level_clicked()
{
    StartLevel("se_official", iSE_StartWithXplus[SE_OFFICIAL], iSE_StartWithDifficulty[SE_OFFICIAL]);
}

void MainWindow::on_pushButton_se_bestmaps_start_level_clicked()
{
    StartLevel("se_bestmaps", iSE_StartWithXplus[SE_BESTMAPS], iSE_StartWithDifficulty[SE_BESTMAPS]);
}

void MainWindow::on_pushButton_se_hqmaps_start_level_clicked()
{
    StartLevel("se_hqmaps", iSE_StartWithXplus[SE_HQMAPS], iSE_StartWithDifficulty[SE_HQMAPS]);
}

void MainWindow::on_pushButton_se_usersmaps_start_level_clicked()
{
    StartLevel("se_usersmaps", iSE_StartWithXplus[SE_USERSMAPS],iSE_StartWithDifficulty[SE_USERSMAPS] );
}

// ********************************************************************************************
// ************************************* Start Mod slots **************************************
// ********************************************************************************************

void MainWindow::StartMod(QString strSqlTable, int iDifficulty)
{
    if(!bInstallGameOk || !bInstallGameDataOk){
        MsgBox(WARN, "Please install the game before starting the mod");
        return;
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(strRunnerDBPath);
    if(!db.open()){
        MsgBox(ERROR, "Error: Open SQL databese Failed!");
        return;
    }
    QSqlQuery qry(db);
    qry.setForwardOnly(true);
    qry.prepare("Select mod_internal_name,Status,start_level FROM " + strSqlTable + " WHERE mod_num = :index_num");
    qry.bindValue(":index_num", iTableRowIndex);
    if (!qry.exec()){
        MsgBox(ERROR, "Error: " + qry.lastError().text());
        db.close();
        return;
    }
    QString strStartedMod, strStatus, strLevelFile;
    while (qry.next()) {
        strStartedMod = qry.value(0).toString();
        strStatus = qry.value(1).toString();
        strLevelFile = qry.value(2).toString();
    }
    db.close();

    if(!strStatus.contains("Installed")){
        MsgBox(WARN, "Before starting this mod, you need to download it.");
        return;
    }

    QString strProgram;
    if(strSqlTable.contains("fe_")) {
        strProgram = strGameDirFEpath + "/Bin/SeriousSam";        
        MakeDifficultyScript(iDifficulty, strGameDirFEpath + "/Scripts/Game_difficulty.ini");
    } else {
        strProgram = strGameDirSEpath + "/Bin/SeriousSam";       
        MakeDifficultyScript(iDifficulty, strGameDirSEpath + "/Scripts/Game_difficulty.ini");
    }
    QStringList arguments;
    arguments << "+game" << strStartedMod;
    // !!! Uncomment it out if you want to run mods bypassing the menu
    //arguments << "+level" << strLevelFile;
    arguments << "+script" << "Scripts/Game_difficulty.ini";

    QProcess *Process = new QProcess(this);
    Process->start(strProgram, arguments);
    this->hide();
}

void MainWindow::on_pushButton_fe_mods_sttart_mod_clicked()
{
    StartMod("fe_mods", iFE_StartWithDifficulty[FE_MODS]);
}

void MainWindow::on_pushButton_se_mods_start_mod_clicked()
{
    StartMod("se_mods", iSE_StartWithDifficulty[SE_MODS]);
}

// ********************************************************************************************
// ******************************* Levels Download slots **************************************
// ********************************************************************************************

void MainWindow::DownloadLevel(QString strSqlTable, QString strGameDirPath)
{
    if(!bInstallGameOk || !bInstallGameDataOk){
        MsgBox(WARN, "Please install the game before download the level");
        HideProgressBars();
        return;
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(strRunnerDBPath);
    if(!db.open()){
        MsgBox(ERROR, "Error: Open SQL databese Failed!");
        return;
    }
    QSqlQuery qry(db);
    qry.setForwardOnly(true);
    qry.prepare("Select level_download_link,Status FROM " + strSqlTable + " WHERE level_num = :level_num");
    qry.bindValue(":level_num", iTableRowIndex);
    if (!qry.exec()){
        MsgBox(ERROR, "Error: " + qry.lastError().text());
        db.close();
        return;
    }
    QString strDownloadLink, strStatus;
    while (qry.next()) {
        strDownloadLink= qry.value(0).toString();
        strStatus = qry.value(1).toString();
    }
    db.close();

    if(strStatus.contains("Installed")){
        MsgBox(INFO, "This level already installed.");
        HideProgressBars();
        return;
    }
    m_downloader.GetLevel(strGameDirPath, strDownloadLink);
}

void MainWindow::on_pushButton_fe_bestmaps_download_clicked()
{
    if(m_downloader.locked_action) {return;}
    iProgressBarIndex = FE_BESTMAPS_PBAR;
    ui->progressBar_fe_bestmaps_download->setValue(0);
    ui->progressBar_fe_bestmaps_download->show();
    DownloadLevel("fe_bestmaps", strGameDirFEpath);
}

void MainWindow::on_pushButton_fe_usersmap_download_clicked()
{
    if(m_downloader.locked_action) {return;}
    iProgressBarIndex = FE_USERSMAPS_PBAR;
    ui->progressBar_fe_usersmaps_download->setValue(0);
    ui->progressBar_fe_usersmaps_download->show();
    DownloadLevel("fe_usersmaps", strGameDirFEpath);
}

void MainWindow::on_pushButton_se_bestmaps_download_clicked()
{
    if(m_downloader.locked_action) {return;}
    iProgressBarIndex = SE_BESTMAPS_PBAR;
    ui->progressBar_se_bestmaps_download->setValue(0);
    ui->progressBar_se_bestmaps_download->show();
    DownloadLevel("se_bestmaps", strGameDirSEpath);
}

void MainWindow::on_pushButton_se_hqmaps_download_clicked()
{
    if(m_downloader.locked_action) {return;}
    iProgressBarIndex = SE_HQMAPS_PBAR;
    ui->progressBar_se_hqmaps_download->setValue(0);
    ui->progressBar_se_hqmaps_download->show();
    DownloadLevel("se_hqmaps", strGameDirSEpath);
}

void MainWindow::on_pushButton_se_usersmaps_download_clicked()
{
    if(m_downloader.locked_action) {return;}
    iProgressBarIndex = SE_USERSMAPS_PBAR;
    ui->progressBar_se_usersmaps_download->setValue(0);
    ui->progressBar_se_usersmaps_download->show();
    DownloadLevel("se_usersmaps", strGameDirSEpath);
}

// ********************************************************************************************
// ******************************* Mods Download slots ****************************************
// ********************************************************************************************

void MainWindow::DownloadMod(QString strSqlTable)
{
    if(!bInstallGameOk || !bInstallGameDataOk){
        MsgBox(WARN, "Please install the game before download the mod");
        HideProgressBars();
        return;
    }

    // Choose distro
    QString strSqlUrlBin;
    switch(iDistoFlag )
    {
        case FREEBSD:
            strSqlUrlBin = "mod_download_link_bsd_bin";
            break;
        case ALPINE:
            strSqlUrlBin = "mod_download_link_alpine_bin";
            break;
        case LINUX:
            strSqlUrlBin = "mod_download_link_lnx_bin";
            break;
        case OTHER:
            strSqlUrlBin = "mod_download_link_lnx_bin";
            break;
        default:
            strSqlUrlBin = "mod_download_link_lnx_bin";
            break;
    }

    // Get download link from sql
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(strRunnerDBPath);
    if(!db.open()){
        MsgBox(ERROR, "Error: Open SQL databese Failed!");
        return;
    }
    QSqlQuery qry(db);
    qry.setForwardOnly(true);
    qry.prepare("Select " + strSqlUrlBin + ",mod_download_link_data,Status FROM " + strSqlTable + " WHERE mod_num = :mod_num");
    qry.bindValue(":mod_num", iTableRowIndex);
    if (!qry.exec()){
        MsgBox(ERROR, "Error: " + qry.lastError().text());
        db.close();
        return;
    }
    QString strDownloadLinkBin, strDownloadLinkData, strStatus;
    while (qry.next()) {
        strDownloadLinkBin = qry.value(0).toString();
        strDownloadLinkData = qry.value(1).toString();
        strStatus = qry.value(2).toString();
    }
    db.close();

    if(strStatus.contains("Installed")){
        MsgBox(INFO, "This Mod already installed.");
        HideProgressBars();
        return;
    }
    if(strSqlTable.contains("fe")){
        m_downloader.GetMod( strRunnerDirPath, "/SamTFE", strDownloadLinkBin, strDownloadLinkData);
    } else {
        m_downloader.GetMod( strRunnerDirPath, "/SamTSE", strDownloadLinkBin, strDownloadLinkData);
    }

}

void MainWindow::on_pushButton_fe_mods_download_clicked()
{
    if(m_downloader.locked_action) {return;}
    iProgressBarIndex = FE_MODS_PBAR;
    iUnpackProgressIndex = FE_MODS_UNPACKPROGRESS;
    iCopyDirProgressIndex  = FE_MODS_COPYPROGRESS;
    ui->progressBar_fe_mods_download->setValue(0);
    ui->progressBar_fe_mods_download->show();
    DownloadMod("fe_mods");
}

void MainWindow::on_pushButton_se_mods_download_clicked()
{
    if(m_downloader.locked_action) {return;}
    iProgressBarIndex = SE_MODS_PBAR;
    iUnpackProgressIndex = SE_MODS_UNPACKPROGRESS;
    iCopyDirProgressIndex  = SE_MODS_COPYPROGRESS;
    ui->progressBar_se_mods_download->setValue(0);
    ui->progressBar_se_mods_download->show();
    DownloadMod("se_mods");
}

// ********************************************************************************************
// ****************************** Download_cancel slots ***************************************
// ********************************************************************************************

void MainWindow::on_pushButton_fe_bestmaps_download_cancel_clicked()
{
    if(m_downloader.locked_cancel || !m_downloader.locked_action) {return;}
    m_downloader.slotCancelDownload();
    slotOnDownloadEnd();
}

void MainWindow::on_pushButton_fe_usersmap_download_cancel_clicked()
{
    if(m_downloader.locked_cancel || !m_downloader.locked_action) {return;}
    m_downloader.slotCancelDownload();
    slotOnDownloadEnd();
}

void MainWindow::on_pushButton_fe_mods_download_cancel_clicked()
{
    if(m_downloader.locked_cancel || !m_downloader.locked_action) {return;}
    m_downloader.slotCancelDownload();
    slotOnDownloadEnd();
}

void MainWindow::on_pushButton_se_bestmaps_download_cancel_clicked()
{
    if(m_downloader.locked_cancel || !m_downloader.locked_action) {return;}
    m_downloader.slotCancelDownload();
    slotOnDownloadEnd();
}

void MainWindow::on_pushButton_se_hqmaps_download_cancel_clicked()
{
   if(m_downloader.locked_cancel || !m_downloader.locked_action) {return;}
    m_downloader.slotCancelDownload();
    slotOnDownloadEnd();
}

void MainWindow::on_pushButton_se_usersmaps_download_cancel_clicked()
{
    if(m_downloader.locked_cancel || !m_downloader.locked_action) {return;}
    m_downloader.slotCancelDownload();
    slotOnDownloadEnd();
}

void MainWindow::on_pushButton_se_mods_download_cancel_clicked()
{
    if(m_downloader.locked_cancel || !m_downloader.locked_action) {return;}
    m_downloader.slotCancelDownload();
    slotOnDownloadEnd();
}

// ********************************************************************************************
// ****************************** Set difficulty slots ****************************************
// ********************************************************************************************

void MainWindow::on_comboBox_fe_official_difficulty_currentIndexChanged(int index)
{
    iFE_StartWithDifficulty[FE_OFFICIAL]  = index;
}

void MainWindow::on_comboBox_fe_bestmaps_difficulty_currentIndexChanged(int index)
{
    iFE_StartWithDifficulty[FE_BESTMAPS]  = index;
}

void MainWindow::on_comboBox_fe_usersmaps_difficulty_currentIndexChanged(int index)
{
    iFE_StartWithDifficulty[FE_USERSMAPS]  = index;
}

void MainWindow::on_comboBox_fe_mods_difficulty_currentIndexChanged(int index)
{
    iFE_StartWithDifficulty[FE_MODS]  = index;
}

void MainWindow::on_comboBox_se_official_difficulty_currentIndexChanged(int index)
{
    iSE_StartWithDifficulty[SE_OFFICIAL]  = index;
}

void MainWindow::on_comboBox_se_bestmaps_difficulty_currentIndexChanged(int index)
{
    iSE_StartWithDifficulty[SE_BESTMAPS]  = index;
}

void MainWindow::on_comboBox_se_hqmaps_difficulty_currentIndexChanged(int index)
{
    iSE_StartWithDifficulty[SE_HQMAPS]  = index;
}

void MainWindow::on_comboBox_se_usersmaps_difficulty_currentIndexChanged(int index)
{
    iSE_StartWithDifficulty[SE_USERSMAPS]  = index;
}

void MainWindow::on_comboBox_se_mods_difficulty_currentIndexChanged(int index)
{
    iSE_StartWithDifficulty[SE_MODS]  = index;    
}

// ********************************************************************************************
// *********************************** Use XPLUS slots ****************************************
// ********************************************************************************************

void MainWindow::FE_UseXPLUS(int arg, int iTable, bool bInstalled)
{
    if(!bInstalled) {
        MsgBox(INFO,"You can download XPLUS Mod before using!");
    }
    if(!arg){
        iFE_StartWithXplus[iTable] = FALSE;
    } else {
        iFE_StartWithXplus[iTable] = TRUE;
    }
}

void MainWindow::SE_UseXPLUS(int arg, int iTable, bool bInstalled)
{
    if(!bInstalled) {
        MsgBox(INFO,"You can download XPLUS Mod before using!");
    }
    if(!arg){
        iSE_StartWithXplus[iTable] = FALSE;
    } else {
        iSE_StartWithXplus[iTable] = TRUE;
    }
}
void MainWindow::on_checkBox_fe_official_use_xplus_stateChanged(int arg)
{
    FE_UseXPLUS(arg, FE_OFFICIAL, bFE_XPLUSInstallOk);
}

void MainWindow::on_checkBox_fe_bestmaps_use_xplus_stateChanged(int arg)
{
    FE_UseXPLUS(arg, FE_BESTMAPS, bFE_XPLUSInstallOk);
}

void MainWindow::on_checkBox_fe_usersmap_use_xplus_stateChanged(int arg)
{
    FE_UseXPLUS(arg, FE_USERSMAPS, bFE_XPLUSInstallOk);
}

void MainWindow::on_checkBox_se_official_use_xplus_stateChanged(int arg)
{
    SE_UseXPLUS(arg, SE_OFFICIAL, bSE_XPLUSInstallOk);
}

void MainWindow::on_checkBox_se_bestmaps_use_xplus_stateChanged(int arg)
{
    SE_UseXPLUS(arg, SE_BESTMAPS, bSE_XPLUSInstallOk);
}

void MainWindow::on_checkBox_se_hqmaps_use_xplus_stateChanged(int arg)
{
    SE_UseXPLUS(arg, SE_HQMAPS, bSE_XPLUSInstallOk);
}

void MainWindow::on_checkBox_se_usersmaps_use_xplus_stateChanged(int arg)
{
    SE_UseXPLUS(arg, SE_USERSMAPS, bSE_XPLUSInstallOk);
}

// ********************************************************************************************
// *********************************** Add user map slot **************************************
// ********************************************************************************************
void MainWindow::on_comboBox_themes_currentIndexChanged(int index)
{
    QString strTheme;
    switch(index)
    {
        case DEFAULTTHEME:
          window()->setStyleSheet(""); //return;
          break;
        case AMOLED:
          strTheme = ":/Themes/AMOLED.qss";
          break;
        case AQUA:
          strTheme = ":/Themes/Aqua.qss";
          break;
        case CONSOLESTYLE:
          strTheme = ":/Themes/ConsoleStyle.qss";
          break;
        case ELEGANTDARK:
          strTheme = ":/Themes/ElegantDark.qss";
          break;
        case MACOSTHEME:
          strTheme = ":/Themes/MacOS.qss";
          break;
        case MANJAROMIX:
          strTheme = ":/Themes/ManjaroMix.qss";
          break;
        case MATERIALDARKE:
          strTheme = ":/Themes/MaterialDark.qss";
          break;
        case NEONBUTTONS:
          strTheme = ":/Themes/NeonButtons.qss";
          break;
        case UBUNTUTHEME:
          strTheme = ":/Themes/Ubuntu.qss";
          break;
        default:
          window()->setStyleSheet(""); //return;
          break;
    }
    // set theme
    if(index != 0){
        QFile file(strTheme);
        if(file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            window()->setStyleSheet(file.readAll());
            file.close();
        }
    }
    QString filename = strRunnerDirPath + "/Theme.ini";
    QFile file(filename);
    if (file.open(QIODevice::ReadWrite)) {
        QTextStream stream(&file);
        stream << QString::number(index) << Qt::endl;
    }
    file.close();
}

/*
void MainWindow::on_pushButton_add_user_map_clicked()
{
    QDesktopServices::openUrl(QUrl("https://github.com/tx00100xt/Serious-Runner/issues", QUrl::TolerantMode));
}
*/

// ********************************************************************************************
// *************************************** Message Box ****************************************
// ********************************************************************************************

void MainWindow::MsgBox(int iType, QString strMessage)
{
    msgBox = new QMessageBox(this);
    switch(iType)
    {
        case INFO:
             msgBox->setIconPixmap(QPixmap(":/Images/ok.png"));
            break;
        case WARN:
             msgBox->setIconPixmap(QPixmap(":/Images/warning.png"));
            break;
        case ERROR:
             msgBox->setIconPixmap(QPixmap(":/Images/error.png"));
            break;
        default:
            break;
    }
    msgBox->setText(strMessage);
    msgBox->setWindowTitle("Serious Runner");
    msgBox->exec();
}

// ********************************************************************************************
// ***************************************** Sleep ********************************************
// ********************************************************************************************

void MainWindow::SleepMSecs(int iMsecs)
{
    QTime dieTime= QTime::currentTime().addMSecs(iMsecs); // 50
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

// ********************************************************************************************
// *************** Execute a command and get the output of the command ************************
// ********************************************************************************************

QString MainWindow::exec(const char* cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return QString::fromUtf8(result.c_str());//result;
}
// ********************************************************************************************
// ************************************** The End *********************************************
// ********************************************************************************************

