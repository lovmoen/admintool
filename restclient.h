#ifndef __RESTCLIENT_H__
#define __RESTCLIENT_H__

#include <QObject>
#include <QThread>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class RestWorker : public QThread
{
    Q_OBJECT

private:
    QNetworkAccessManager* client;

public:
    RestWorker();
    ~RestWorker();

private:
    void HandleReply(QNetworkReply* reply);

private slots:
    //void error(QNetworkReply::NetworkError errorCode);
    void finished(QNetworkReply* reply);

public slots:
    void Post(QString url, QByteArray payload);

signals:
    void ResponseReady(bool success, QByteArray response);
};

class RestClient : public QObject
{
    Q_OBJECT

private:
    QThread workerThread;
    RestWorker* worker;

public:
    RestClient();
    ~RestClient();

public slots:
    void ResponseReady(bool success, QByteArray response);

signals:
    void Post(QString url, QByteArray payload);
    void RestResponseReady(bool success, QByteArray response);
};

#endif
