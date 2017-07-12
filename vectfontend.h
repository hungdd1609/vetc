#ifndef VECTFONTEND_H
#define VECTFONTEND_H

#include <QNetworkSession>
#include <QObject>
#include <QTcpSocket>

class QTcpSocket;
class QNetworkSession;


//------------------------------------------------------------------
enum COMMAND{
    CONNECT,
    CONNECT_RESP,
    SHAKE,
    SHAKE_RESP,
    CHECKIN,
    CHECKIN_RESP,
    COMMIT,
    COMMIT_RESP,
    ROLLBACK,
    ROLLBACK_RESP,
    TERMINATE,
    TERMINATE_RESP,
    CHARGE,
    CHARGE_RESP
};

enum COMMAND_LENGTH{
    LENGTH_CONNECT         = 44,
    LENGTH_CONNECT_RESP    = 20,
    LENGTH_SHAKE           = 16,
    LENGTH_SHAKE_RESP      = 20,
    LENGTH_CHECKIN         = 98,
    LENGTH_CHECKIN_RESP    = 82,
    LENGTH_COMMIT          = 78,
    LENGTH_COMMIT_RESP     = 20,
    LENGTH_ROLLBACK        = 70,
    LENGTH_ROLLBACK_RESP   = 20,
    LENGTH_TERMINATE       = 16,
    LENGTH_TERMINATE_RESP  = 20,
    LENGTH_CHARGE          = 48,
    LENGTH_CHARGE_RESP     = 74
};
//------------------------------------------------------------------
struct Header{
  int MsgLength;
  int CommandId;
  int RequestId;
  int SessionId;
};

struct ConnectMsg{
    Header header;
    char Username[10];
    char Password[10];
    int Station;
    int Timeout;
};

struct ShakeMsg{
    Header header;
};

struct TerminateMsg{
    Header header;
};

struct CheckinMsg{
    Header header;
    char Etag[24];
    int Station;
    int Lane;
    char Plate[10];
    char TID[24];
    char HashValue[16];
};



struct EncryptMsg{
    int MsgLength;
    QByteArray MsgContent;
};

class VectFontEnd : public QObject
{
    Q_OBJECT
public:
    VectFontEnd(QString host, int port, QString username, QString passwd, int stationId);
    void sendConnect();
    void sendShake();
    void sendTerminate();
    void sendCheckin();


private slots:
    void slotReadFortune();
    void slotRequestNewFortune();
    void slotDisplayError(QAbstractSocket::SocketError socketError);
    void slotSessionOpened();
    void slotSendConnect();

private:
    QNetworkSession *networkSession;
    QTcpSocket *tcpSocket;
    QString currentFortune;    
    quint16 blockSize;

    QString host;
    int port;

    QString username;
    QString passwd;
    QString etag;
    QString plate;
    QString tId;
    QString hashValue;
    int stationId;
    int requestId;
    int sessionId;
    int lane;

    EncryptMsg encryptMsg;

    QByteArray encrypt(QByteArray input);


signals:
    void exportFortune(QString);
    void exportRequestNewFortune();
    void exportError(QString);
    void exportSessionOpened();


};

#endif // VECTFONTEND_H
