#include "vectfontend.h"
#include <QNetworkConfigurationManager>
#include <QSettings>
#include <QTimer>
#include <QDebug>

#define RESQUEST_TIMEOUT 5

VectFontEnd::VectFontEnd(QString host, int port, QString username, QString passwd, int stationId) //: networkSession(0)
{


    QNetworkConfigurationManager manager;
    //if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired) {
    if (true){
        // Get saved network configuration
        QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
        settings.beginGroup(QLatin1String("QtNetwork"));
        const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
        settings.endGroup();

        // If the saved network configuration is not currently discovered use the system default
        QNetworkConfiguration config = manager.configurationFromIdentifier(id);
        if ((config.state() & QNetworkConfiguration::Discovered) !=
                QNetworkConfiguration::Discovered) {
            config = manager.defaultConfiguration();
        }

        networkSession = new QNetworkSession(config, this);
        connect(networkSession, SIGNAL(opened()), this, SLOT(slotSessionOpened()));

        networkSession->open();

        this->host = host;
        this->port = port;
        this->username = username;
        this->passwd = passwd;
        this->stationId = stationId;

        tcpSocket = new QTcpSocket(this);

        connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(slotReadFortune()));
        connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotDisplayError(QAbstractSocket::SocketError)));
        connect(tcpSocket, SIGNAL(connected()), this, SLOT(slotSendConnect()));

        requestId = 0;
        sessionId = 0;
        blockSize = 0;

        tcpSocket->abort();
        tcpSocket->connectToHost(this->host, this->port);
    }
}

void VectFontEnd::slotReadFortune()
{
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);

    if (blockSize == 0) {
        if (tcpSocket->bytesAvailable() < (int)sizeof(quint16))
            return;

        in >> blockSize;
    }

    if (tcpSocket->bytesAvailable() < blockSize)
        return;

    QString nextFortune;
    in >> nextFortune;

    if (nextFortune == currentFortune) {
        QTimer::singleShot(0, this, SLOT(slotRequestNewFortune()));
        return;
    }

    currentFortune = nextFortune;

    //emit fortune
    emit exportFortune(currentFortune);
}

void VectFontEnd::slotRequestNewFortune()
{
    blockSize = 0;
    tcpSocket->abort();
    tcpSocket->connectToHost(this->host, this->port);
    requestId = 0;
    sessionId = 0;

    emit exportRequestNewFortune();
}

void VectFontEnd::slotDisplayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        emit exportError("The host was not found. Please check the "
                         "host name and port settings.");
        break;
    case QAbstractSocket::ConnectionRefusedError:
        emit exportError("The connection was refused by the peer. "
                         "Make sure the fortune server is running, "
                         "and check that the host name and port "
                         "settings are correct.");
        break;
    default:
        emit exportError(tr("The following error occurred: %1.")
                         .arg(tcpSocket->errorString()));
    }
}

void VectFontEnd::slotSessionOpened()
{
    // Save the used configuration
    QNetworkConfiguration config = networkSession->configuration();
    QString id;
    if (config.type() == QNetworkConfiguration::UserChoice)
        id = networkSession->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
    else
        id = config.identifier();

    QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
    settings.beginGroup(QLatin1String("QtNetwork"));
    settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
    settings.endGroup();

    emit exportSessionOpened();
}


void VectFontEnd::slotSendConnect()
{
    sendConnect();
    sendShake();
}

void VectFontEnd::sendConnect()
{
    qDebug() << "VectFontEnd::sendConnect()";
    Header header;
    header.MsgLength = LENGTH_CONNECT;
    header.CommandId = CONNECT;
    header.RequestId = requestId++;
    header.SessionId = sessionId;

    ConnectMsg connectCmd;
    connectCmd.header = header;
    strncpy(connectCmd.Username, username.toUtf8().data(),sizeof(connectCmd.Username));
    strncpy(connectCmd.Password, passwd.toUtf8().data(),sizeof(connectCmd.Password));
    connectCmd.Station = stationId;
    connectCmd.Timeout = RESQUEST_TIMEOUT;

    QString sendMsg = QString("Sent CONNECT %1 %2 %3 %4 %5 %6 %7 %8")
            .arg(connectCmd.header.MsgLength)
            .arg(connectCmd.header.CommandId)
            .arg(connectCmd.header.RequestId)
            .arg(connectCmd.header.SessionId)
            .arg(connectCmd.Username)
            .arg(connectCmd.Password)
            .arg(connectCmd.Station)
            .arg(connectCmd.Timeout);

    QByteArray data;
    QDataStream in(&data, QIODevice::ReadWrite);

    in.setByteOrder(QDataStream::LittleEndian);
    in << connectCmd.header.MsgLength;
    in << connectCmd.header.CommandId;
    in << connectCmd.header.RequestId;
    in << connectCmd.header.SessionId;
    in.writeRawData(connectCmd.Username, 10);
    in.writeRawData(connectCmd.Password, 10);
    in << connectCmd.Station;
    in << connectCmd.Timeout;

    qDebug() << data.toHex();
    QByteArray encryptData = encrypt(data);
    tcpSocket->write(encryptData);
    qDebug() << sendMsg;
}

void VectFontEnd::sendShake()
{
    qDebug() << "VectFontEnd::sendShake()";
    Header header;
    header.MsgLength = LENGTH_SHAKE;
    header.CommandId = SHAKE;
    header.RequestId = requestId++;
    header.SessionId = sessionId;

    ShakeMsg shakeCmd;
    shakeCmd.header = header;

    QString sendMsg = QString("Sent SHAKE %1 %2 %3 %4")
            .arg(shakeCmd.header.MsgLength)
            .arg(shakeCmd.header.CommandId)
            .arg(shakeCmd.header.RequestId)
            .arg(shakeCmd.header.SessionId);

    QByteArray data;
    QDataStream in(&data, QIODevice::WriteOnly);

    in.setByteOrder(QDataStream::LittleEndian);
    in << shakeCmd.header.MsgLength;
    in << shakeCmd.header.CommandId;
    in << shakeCmd.header.RequestId;
    in << shakeCmd.header.SessionId;

    qDebug() << data.toHex();
    QByteArray encryptData = encrypt(data);
    tcpSocket->write(encryptData);
    qDebug() << sendMsg;
}

void VectFontEnd::sendTerminate()
{
    qDebug() << "VectFontEnd::sendTerminate()";
    Header header;
    header.MsgLength = LENGTH_TERMINATE;
    header.CommandId = TERMINATE;
    header.RequestId = requestId++;
    header.SessionId = sessionId;

    TerminateMsg terminateCmd;
    terminateCmd.header = header;

    QString sendMsg = QString("Sent Terminate %1 %2 %3 %4")
            .arg(terminateCmd.header.MsgLength)
            .arg(terminateCmd.header.CommandId)
            .arg(terminateCmd.header.RequestId)
            .arg(terminateCmd.header.SessionId);

    QByteArray data;
    QDataStream in(&data, QIODevice::WriteOnly);

    in.setByteOrder(QDataStream::LittleEndian);
    in << terminateCmd.header.MsgLength;
    in << terminateCmd.header.CommandId;
    in << terminateCmd.header.RequestId;
    in << terminateCmd.header.SessionId;

    qDebug() << data.toHex();
    QByteArray encryptData = encrypt(data);
    tcpSocket->write(encryptData);
    qDebug() << sendMsg;
}

void VectFontEnd::sendCheckin()
{
    qDebug() << "VectFontEnd::sendCheckin()";
    Header header;
    header.MsgLength = LENGTH_CHECKIN;
    header.CommandId = CHECKIN;
    header.RequestId = requestId++;
    header.SessionId = sessionId;

    CheckinMsg checkinCmnd;
    checkinCmnd.header = header;
    strncpy(checkinCmnd.Etag, etag.toUtf8().data(),sizeof(checkinCmnd.Etag));
    checkinCmnd.Station = stationId;
    checkinCmnd.Lane = lane;
    strncpy(checkinCmnd.Plate, plate.toUtf8().data(),sizeof(checkinCmnd.Plate));
    strncpy(checkinCmnd.TID, tId.toUtf8().data(),sizeof(checkinCmnd.TID));
    strncpy(checkinCmnd.HashValue, hashValue.toUtf8().data(),sizeof(checkinCmnd.HashValue));

    QString sendMsg = QString("Sent CHECKIN %1 %2 %3 %4 %5 %6 %7 %8 %9 %10")
            .arg(checkinCmnd.header.MsgLength)
            .arg(checkinCmnd.header.CommandId)
            .arg(checkinCmnd.header.RequestId)
            .arg(checkinCmnd.header.SessionId)
            .arg(checkinCmnd.Etag)
            .arg(checkinCmnd.Station)
            .arg(checkinCmnd.Lane)
            .arg(checkinCmnd.Plate)
            .arg(checkinCmnd.TID)
            .arg(checkinCmnd.HashValue);

    QByteArray data;
    QDataStream in(&data, QIODevice::ReadWrite);

    in.setByteOrder(QDataStream::LittleEndian);
    in << checkinCmnd.header.MsgLength;
    in << checkinCmnd.header.CommandId;
    in << checkinCmnd.header.RequestId;
    in << checkinCmnd.header.SessionId;
    in.writeRawData(checkinCmnd.Etag, sizeof(checkinCmnd.Etag));
    in << checkinCmnd.Station;
    in << checkinCmnd.Lane;
    in.writeRawData(checkinCmnd.Plate, sizeof(checkinCmnd.Plate));
    in.writeRawData(checkinCmnd.TID, sizeof(checkinCmnd.TID));
    in.writeRawData(checkinCmnd.HashValue, sizeof(checkinCmnd.HashValue));

    qDebug() << data.toHex();
    QByteArray encryptData = encrypt(data);
    tcpSocket->write(encryptData);
    qDebug() << sendMsg;

}


QByteArray VectFontEnd::encrypt(QByteArray input)
{
    encryptMsg.MsgLength = input.length() + sizeof(int);
    encryptMsg.MsgContent = input;


     QByteArray result;
     QDataStream resultStream(&result, QIODevice::WriteOnly);
     resultStream.setByteOrder(QDataStream::LittleEndian);
     resultStream << int(encryptMsg.MsgLength);
     resultStream.writeRawData(encryptMsg.MsgContent, encryptMsg.MsgContent.length());
     return result;
}
