#ifndef THREAD_H
#define THREAD_H

#include <QObject>
#include <QThread>
#include "rcon.h"
#include "customitems.h"

class RulesInfo;
class InfoReply;
class PlayerInfo;
class LogHandler;
class ServerInfo;

class Worker : public QThread
{
    Q_OBJECT

public slots:
    void getServerInfo(QHostAddress *host, quint16 port, ServerTableIndexItem *item, ServerInfo* info);
    void getPlayerInfo(QHostAddress *host, quint16 port, ServerTableIndexItem *item);
    void getRulesInfo(QHostAddress *host, quint16 port, ServerTableIndexItem *item);
    void setupUPnP(LogHandler *);

signals:
    void serverInfoReady(InfoReply *reply, ServerTableIndexItem *item, ServerInfo* info);
    void playerInfoReady(QList<PlayerInfo> *, ServerTableIndexItem *item);
    void rulesInfoReady(QList<RulesInfo> *, ServerTableIndexItem *item);
    void UPnPReady();
};

#endif // THREAD_H
