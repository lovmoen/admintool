#ifndef LOGHANDLER_H
#define LOGHANDLER_H

#include "serverinfo.h"
#include <QUdpSocket>
#include <QThread>
#include <QNetworkAccessManager>

class Worker;

class LogHandler: public QObject
{
    Q_OBJECT
public:
    LogHandler(MainWindow *, MainTask*);
    ~LogHandler();
    void createBind(quint16);
    void removeServer(ServerInfo *);
    void addServer(ServerInfo *);
    bool isPrivateIP(QHostAddress address);
    QHostAddress *getAddressToLogTo(QHostAddress serverAddress);
    QString szPort;
    bool isBound;
    void setExternalIP(QString szAddress);
    void setInternalIP(QString szAddress);
private:
    void findLocalAddress();
    void createSocket();

signals:
    void setupUPnP(LogHandler *);

private slots:
    void socketReadyRead();
    void socketDisconnected();
    void apiFinished(QNetworkReply *);

public slots:
    void UPnPReady();

private:
    quint16 logPort;
    QUdpSocket *logsocket;
    QList<ServerInfo *> logList;
    MainWindow *pMain;
    MainTask* pMainTask;
    QThread workerThread;
    Worker *worker;
    QNetworkAccessManager *manager;
    // Grabbed for UPnP
    QHostAddress *pExternalIP;
    QHostAddress *pInternalIP;
    //Grabbed from network interfaces
    QHostAddress *pLocalAddress;
};

#endif // LOGHANDLER_H
