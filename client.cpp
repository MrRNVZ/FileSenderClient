#include "client.h"

Client::Client(QWidget *parent) :
    QWidget(parent)
{
    setWindowIcon(QIcon(":/Icons/FileSender.ico"));
    setWindowTitle("FileSenderClient");

    // Server connection UI elements
    txtHost = new QLineEdit;
    txtHost->setPlaceholderText("Host address");
    txtPort = new QLineEdit;
    txtPort->setPlaceholderText("Port");
    btnConnect = new QPushButton("Connect");
    btnDisconnect = new QPushButton("Disconnect");
    btnDisconnect->setEnabled(false);
    hbltConnect = new QHBoxLayout;
    hbltConnect->addWidget(btnConnect);
    hbltConnect->addWidget(btnDisconnect);
    vbltConnect = new QVBoxLayout;
    vbltConnect->addWidget(txtHost);
    vbltConnect->addWidget(txtPort);
    vbltConnect->addLayout(hbltConnect);
    gbxConnect = new QGroupBox("Connect to server");
    gbxConnect->setLayout(vbltConnect);
    connect(btnConnect, &QPushButton::clicked,
            this, &Client::connectToServer);
    connect(btnDisconnect, &QPushButton::clicked,
            this, &Client::connectCancel);

    // Upload files UI elements
    txtFileUpload = new QLineEdit;
    txtFileUpload->setPlaceholderText("File path");
    btnSelectFileUpload = new QPushButton("Select file");
    hbltFileUpload = new QHBoxLayout;
    hbltFileUpload->addWidget(txtFileUpload);
    hbltFileUpload->addWidget(btnSelectFileUpload);
    connect(btnSelectFileUpload, &QPushButton::clicked,
            this, &Client::fileDialog);
    btnUpload = new QPushButton("Upload");
    btnUpload->setEnabled(false);
    connect(btnUpload, &QPushButton::clicked,
            this, &Client::upload);
    barUpload = new QProgressBar;
    barUpload->setVisible(false);
    barUpload->setMinimum(0);
    barUpload->setValue(0);
    vbltFileUpload = new QVBoxLayout;
    vbltFileUpload->addLayout(hbltFileUpload);
    vbltFileUpload->addWidget(btnUpload);
    vbltFileUpload->addWidget(barUpload);
    gbxFileUpload = new QGroupBox("Upload File");
    gbxFileUpload->setLayout(vbltFileUpload);

    // Download files UI elements
    cmbSelectFileDownload = new QComboBox;
    btnDownloadUpdate = new QPushButton("Update server file list");
    btnDownloadUpdate->setEnabled(false);
    connect(btnDownloadUpdate, &QPushButton::clicked,
            this, &Client::askForAvailableFiles);
    btnDownload = new QPushButton("Download");
    btnDownload->setEnabled(false);
    connect(btnDownload, &QPushButton::clicked,
            this, &Client::askForFile);
    barDownload = new QProgressBar;
    barDownload->setVisible(false);
    barDownload->setMinimum(0);
    barDownload->setValue(0);
    hbltFileListUpdate = new QHBoxLayout;
    hbltFileListUpdate->addWidget(cmbSelectFileDownload);
    hbltFileListUpdate->addWidget(btnDownloadUpdate);
    vbltFileDownload = new QVBoxLayout;
    vbltFileDownload->addLayout(hbltFileListUpdate);
    vbltFileDownload->addWidget(btnDownload);
    vbltFileDownload->addWidget(barDownload);
    gbxFileDownload = new QGroupBox("Download File");
    gbxFileDownload->setLayout(vbltFileDownload);

    // Information field
    txtInfo = new QTextEdit;
    txtInfo->setReadOnly(true);
    btnInfoClear = new QPushButton("Clear");
    connect(btnInfoClear, &QPushButton::clicked,
            txtInfo, &QTextEdit::clear);
    vbltInfo = new QVBoxLayout;
    vbltInfo->addWidget(txtInfo);
    vbltInfo->addWidget(btnInfoClear);
    gbxInfo = new QGroupBox("Information window");
    gbxInfo->setLayout(vbltInfo);

    // Adding groups to main layout
    vltMain = new QVBoxLayout;
    vltMain->addWidget(gbxConnect);
    vltMain->addLayout(hbltConnect);
    vltMain->addWidget(gbxFileUpload);
    vltMain->addWidget(gbxFileDownload);
    vltMain->addWidget(gbxInfo);
    setLayout(vltMain);
}

Client::~Client()
{

}

void Client::connectToServer()
{
    // Create new socket
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected,
            this, &Client::connected);
    connect(socket, &QTcpSocket::disconnected,
            socket, &QTcpSocket::deleteLater);
    connect(socket, &QTcpSocket::readyRead,
            this, &Client::readServerAnswer);
//    connect(socket, &QTcpSocket::error,
//            this, &Client::error);
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(error(QAbstractSocket::SocketError)));

    // Change UI buttons availability
    btnConnect->setEnabled(false);
    btnDisconnect->setEnabled(true);

    // Connection to server
    socket->connectToHost(txtHost->text(), txtPort->text().toInt());
    socket->waitForConnected();
}

void Client::connectCancel()
{
    // Change UI buttons availability
    btnDisconnect->setEnabled(false);
    btnUpload->setEnabled(false);
    btnDownload->setEnabled(false);
    btnDownloadUpdate->setEnabled(false);
    // Disconnect from server
    socket->disconnectFromHost();
    if (socket->state() != QAbstractSocket::UnconnectedState)
        socket->waitForDisconnected();
    // Change UI buttons availability
    btnConnect->setEnabled(true);
}

void Client::error(QAbstractSocket::SocketError error)
{
    QString strError = "Error: ";
    switch(error)
    {
    case QAbstractSocket::ConnectionRefusedError:
        strError.append("The connection was refused.");
        break;
    case QAbstractSocket::RemoteHostClosedError:
        strError.append("The remote host is closed.");
        break;
    case QAbstractSocket::HostNotFoundError:
        strError.append("The host was not found.");
        break;
    default:
        strError.append(QString(socket->errorString()));
    }
    txtInfo->append(strError);
    btnConnect->setEnabled(true);
    btnDisconnect->setEnabled(false);
}

void Client::connected()
{
    askForAvailableFiles();
    btnUpload->setEnabled(true);
    btnDownload->setEnabled(true);
    btnDownloadUpdate->setEnabled(true);
}

void Client::fileDialog()
{
    txtFileUpload->setText(QFileDialog::getOpenFileName(this, "Select File", "",
                                                        "All files (*.*);;\
                                                        Image (*.jpg *.png, *.gif);;\
                                                        Music (*.mp3, *.aac, *.wma, *.flac, *.wav);;\
                                                        Text (*.txt);;\
                                                        Video (*avi, *flv, *mkv, *mov, *mp4, *ts)"\
    ));
}

void Client::upload()
{
    // Check if file was selected
    if (txtFileUpload->text().isEmpty())
    {
        txtInfo->append("No file selected.");
        return;
    }
    // Check connection is established
    if (socket->state() != QAbstractSocket::ConnectedState)
    {
        txtInfo->append("No connection to the server.");
        return;
    }

    // Try to open file to send it to server
    QFile file(txtFileUpload->text());
    if (file.open(QFile::ReadOnly))
    {
        // Check file size constraint
        if (file.size() > CLIENT_MAX_FILE_SIZE_MB * 1024 * 1024)
        {
            QMessageBox(QMessageBox::Critical, "File is too large",
                        QString("File size should not be more then %1MB").arg(CLIENT_MAX_FILE_SIZE_MB),
                        QMessageBox::Ok).exec();
            return;
        }

        // Create data stream on socket
        QDataStream out(socket);
        out.setVersion(QDataStream::Qt_5_10);

        // Read whole file
        QByteArray afile = file.readAll();
        file.close();
        // Cut path
        QString filename = txtFileUpload->text().right(txtFileUpload->text().length() - txtFileUpload->text().lastIndexOf("/") - 1);
        // Compute checksum
        QByteArray hash = QCryptographicHash::hash(afile, QCryptographicHash::Sha3_512);
        // Send file and info to server
        out << (quint64)afile.size() << qint8(CLIENT_SEND_PART) << filename << hash << afile;

        // Progressbar setup
        barUpload->setMaximum(afile.size());
        barUpload->setValue(0);
        barUpload->setVisible(true);
    }
    else
    {
        txtInfo->append("Unabled to open file.");
    }
}

void Client::askForAvailableFiles()
{
    // Check connection is established
    if (socket->state() == QAbstractSocket::UnconnectedState)
    {
        txtInfo->append("No connection to the server.");
        return;
    }

    // Send list of files request to server
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_10);
    out << quint64(0) << qint8(CLIENT_SEND_ASK_FILEINFOLIST);
    out.device()->seek(0);
    out << quint64(block.size() - sizeof(quint64));
    socket->write(block);
}

void Client::askForFile()
{
    // Check connection is established
    if (socket->state() == QAbstractSocket::UnconnectedState)
    {
        txtInfo->append("No connection to the server.");
        return;
    }

    // Send file request to server
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_10);
    out << quint64(0) << qint8(CLIENT_SEND_ASK_FILE) << cmbSelectFileDownload->currentText();
    socket->write(block);
}

void Client::readServerAnswer()
{
    // Read received data from server
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_10);
    quint64 blocksize;
    qint8 reason;
    QByteArray hash;
    QByteArray fileBuffer;
    quint64 uploadedBytes;
    QString msg;
    QString filename;
    QFile* file;
    QMessageBox* mbxWrongHash;

    int messageBoxResult;

    if ((quint64)socket->bytesAvailable() < sizeof(quint64))
    {
        return;
    }
    while (!in.atEnd())
    {
        in >> blocksize >> reason;
        int fileAmt;
        switch (reason)
        {
        case CLIENT_GET_PART_NOTIFY:
            in >> uploadedBytes;
            barUpload->setValue(uploadedBytes);
            break;
        case CLIENT_GET_INFO:
            in >> msg;
            txtInfo->append(QTime::currentTime().toString().append(" ") + msg);
            barUpload->setVisible(false);
            break;
        case CLIENT_GET_FILEINFOLIST:
            cmbSelectFileDownload->clear();
            in >> fileAmt;
            for (int i = 0; i < fileAmt; ++i)
            {
                filename.clear();
                in >> filename;
                cmbSelectFileDownload->addItem(filename);
            }
            break;
        case CLIENT_GET_FILE:
            if (!QDir::current().exists(CLIENT_FILE_STORE))
            {
                QDir::current().mkdir(CLIENT_FILE_STORE);
            }

            barDownload->setMaximum(blocksize);
            barDownload->setVisible(true);

            while(static_cast<quint64>(socket->bytesAvailable()) < blocksize - sizeof(qint8))
            {
                barDownload->setValue(socket->bytesAvailable());
                socket->waitForReadyRead(1000);
            }
            barDownload->setVisible(false);
            in >> filename >> hash >> fileBuffer;

            // Проверяем контрольную сумму скаченного файла
            if (hash == QCryptographicHash::hash(fileBuffer, QCryptographicHash::Sha3_512))
            {
                file = new QFile(QString(CLIENT_FILE_STORE).append(filename));
                if (file == NULL)
                {
                    txtInfo->append("Unabled to write file: " +
                                    QDir::currentPath() +
                                    "/" +
                                    QString(CLIENT_FILE_STORE).append(filename));
                    return;
                }
                if (file->open(QFile::WriteOnly))
                {
                    file->write(fileBuffer);
                }
                else
                {

                    txtInfo->append("Unabled to write file: " +
                                    QDir::currentPath() +
                                    "/" +
                                    QString(CLIENT_FILE_STORE).append(filename));

                    delete file;
                    return;
                }
                if (file != NULL)
                {
                    file->close();
                    delete file;
                }
                txtInfo->append(QTime::currentTime().toString() + " File \"" + filename + "\" downloaded.");
            }
            else
            {
                mbxWrongHash = new QMessageBox();
                mbxWrongHash->setText("Wrong hash");
                mbxWrongHash->setInformativeText("Server received file with wrong hash. The file contet may be damadged.\n\nSend file again?");
                mbxWrongHash->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                mbxWrongHash->setDefaultButton(QMessageBox::Yes);
                mbxWrongHash->setIcon(QMessageBox::Critical);
                messageBoxResult = mbxWrongHash->exec();
                delete mbxWrongHash;
                if (messageBoxResult == QMessageBox::Yes)
                {
                    askForFile();
                }
                else
                {
                    txtFileUpload->setText("");
                    barUpload->setVisible(false);
                }
            }
            break;
        case CLIENT_GET_WRONG_HASH:
            in >> msg;
            txtInfo->append(QTime::currentTime().toString() + " " + msg);
            mbxWrongHash = new QMessageBox();
            mbxWrongHash->setText("Wrong hash");
            mbxWrongHash->setInformativeText("Server received file with wrong hash. The file contet may be damadged.\n\nSend file again?");
            mbxWrongHash->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            mbxWrongHash->setDefaultButton(QMessageBox::Yes);
            mbxWrongHash->setIcon(QMessageBox::Critical);
            messageBoxResult = mbxWrongHash->exec();
            delete mbxWrongHash;
            if (messageBoxResult == QMessageBox::Yes)
            {
                upload();
            }
            else
            {
                txtFileUpload->setText("");
                barUpload->setVisible(false);
            }
            break;
        }
    }
}
