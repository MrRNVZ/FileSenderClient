#ifndef CLIENT_H
#define CLIENT_H

#include <QApplication>
#include <QBoxLayout>
#include <QComboBox>
#include <QCryptographicHash>
#include <QFileDialog>
#include <QGroupBox>
#include <QIcon>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QString>
#include <QTextEdit>
#include <QTime>
#include <QTcpSocket>
#include <QWidget>

#define CLIENT_SEND_PART                1
#define CLIENT_SEND_ASK_FILEINFOLIST    2
#define CLIENT_SEND_ASK_FILE            3
#define CLIENT_GET_PART_NOTIFY          1
#define CLIENT_GET_INFO                 2
#define CLIENT_GET_FILEINFOLIST         3
#define CLIENT_GET_FILE                 4
#define CLIENT_GET_WRONG_HASH           5
#define CLIENT_FILE_STORE               "downloads/"
#define CLIENT_MAX_FILE_SIZE_MB         100


class Client : public QWidget
{
    Q_OBJECT

public:
    explicit Client(QWidget *parent = 0);
    ~Client();

public slots:
    void connectToServer();
    void connectCancel();
    void error(QAbstractSocket::SocketError error);
    void connected();
    void fileDialog();
    void upload();
    void askForAvailableFiles();
    void askForFile();
    void readServerAnswer();

private:
    QHBoxLayout     *hbltConnect;
    QVBoxLayout     *vbltConnect;
    QGroupBox       *gbxConnect;
    QHBoxLayout     *hbltFileUpload;
    QVBoxLayout     *vbltFileUpload;
    QGroupBox       *gbxFileUpload;
    QHBoxLayout     *hbltFileListUpdate;
    QVBoxLayout     *vbltFileDownload;
    QGroupBox       *gbxFileDownload;
    QVBoxLayout     *vbltInfo;
    QGroupBox       *gbxInfo;
    QVBoxLayout     *vltMain;
    QTcpSocket      *socket;
    QLineEdit       *txtHost;
    QLineEdit       *txtPort;
    QPushButton     *btnConnect;
    QPushButton     *btnDisconnect;
    QLineEdit       *txtFileUpload;
    QPushButton     *btnSelectFileUpload;
    QPushButton     *btnUpload;
    QProgressBar    *barUpload;
    QComboBox       *cmbSelectFileDownload;
    QPushButton     *btnDownloadUpdate;
    QPushButton     *btnDownload;
    QProgressBar    *barDownload;
    QTextEdit       *txtInfo;
    QPushButton     *btnInfoClear;
};

#endif // CLIENT_H
