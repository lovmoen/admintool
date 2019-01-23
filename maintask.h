#ifndef __MAINTASK_H__
#define __MAINTASK_H__

#include <QtCore>
#include <iostream>
#include "signalhandler.h"
#include "enums.h"
#include "serverinfo.h"
#include "query.h"
#include "loghandler.h"

using namespace std;

class MainTask : public QObject, public SignalHandler
{
    Q_OBJECT

public:
    explicit MainTask(QObject *parent, QString serversFile, QUrl hubUrl);
    ~MainTask();

    bool handleSignal(int signal);
    void displayMessage(ErrorLevel level, QString title, QString message);

    AddServerError CheckServerList(QString server);
    ServerInfo *AddServerToList(QString server, AddServerError *error = nullptr);
    void parseLogLine(QString, ServerInfo *);
    PlayerQuery *pPlayerQuery;
    RulesQuery *pRulesQuery;
    quint16 u16logPort;
    bool showLoggingInfo;

protected:
    void closeEvent(QCloseEvent *event);

public slots:
    void initialize();

    void ServerInfoReady(InfoReply *, ServerTableIndexItem *, ServerInfo *);
    void PlayerInfoReady(QList<PlayerInfo> *, ServerTableIndexItem *);
    void RulesInfoReady(QList<RulesInfo> *, ServerTableIndexItem *);
    void RconAuthReady(ServerInfo *info, QList<QueuedCommand>queuedcmds);
    void RconOutput(ServerInfo *info, QByteArray res);
    void AddRconHistory(QString cmd);
    void AddChatHistory(QString txt);

private slots:
    void addServerEntry(QString server);
    void TimedUpdate();
    void sendChat(const QString& message);
    void passwordUpdated(const QString &);
    void rconLogin();
    void getLog();

signals:
    void finished();

private:
    QString serversFile;
    QUrl hubUrl;
    bool aboutToInterrupt;
    QTimer *updateTimer;
    void ConnectSlots();
    void HookEvents();
    void runCommand(ServerInfo *, QString);
    void rconLoginQueued(QList<QueuedCommand>);
    void connectToServer();
    LogHandler *pLogHandler;
    QList<QString> commandHistory;
    QList<QString> sayHistory;
    QMutableListIterator<QString> *commandIter;
    QMutableListIterator<QString> *sayIter;
};

#endif
