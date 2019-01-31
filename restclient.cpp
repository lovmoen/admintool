#include <QThread>
#include "restclient.h"

RestWorker::RestWorker()
{
    client = new QNetworkAccessManager(this);
    connect(client, &QNetworkAccessManager::finished, this, &RestWorker::finished);
}

RestWorker::~RestWorker()
{
    client->deleteLater();
}

void RestWorker::finished(QNetworkReply* reply)
{
    HandleReply(reply);
}

void RestWorker::HandleReply(QNetworkReply* reply)
{
    if (!reply)
    {
        emit ResponseReady(false, QByteArray());
        return;
    }

    QByteArray response = reply->readAll();
    reply->deleteLater();

    emit ResponseReady(true, response);
}

void RestWorker::Post(QString url, QByteArray payload)
{
    QUrl requestUrl(url);
    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");
    QNetworkReply* reply = client->post(request, payload);

    if (reply->isFinished())
    {
        HandleReply(reply);
    }
}

RestClient::RestClient()
{
    worker = new RestWorker;
    worker->moveToThread(&workerThread);

    connect(&workerThread, &QThread::finished, worker, &RestWorker::deleteLater);
    connect(&workerThread, &QThread::finished, this, &RestClient::deleteLater);
    connect(this, &RestClient::Post, worker, &RestWorker::Post);
    connect(worker, &RestWorker::ResponseReady, this, &RestClient::ResponseReady);

    workerThread.start();
}

RestClient::~RestClient()
{
    workerThread.quit();
    workerThread.wait();
}

void RestClient::ResponseReady(bool success, QByteArray response)
{
    emit RestResponseReady(success, response);
    this->deleteLater();
}
