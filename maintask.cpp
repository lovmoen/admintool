#include "maintask.h"
#include "loghandler.h"
#include "customitems.h"
#include "query.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <QTimer>

using json = nlohmann::json;

extern QList<ServerInfo *> serverList;
extern QRegularExpression actionRegex;
extern QRegularExpression chatRegex;
extern QStringList blueTeams;
extern QStringList redTeams;

MainTask::MainTask(QObject *parent) :
    QObject(parent)
{
    pPlayerQuery = NULL;
    pRulesQuery = NULL;
    commandIter = new QMutableListIterator<QString>(this->commandHistory);
    sayIter = new QMutableListIterator<QString>(this->sayHistory);

    pLogHandler = new LogHandler(NULL, this);
    pLogHandler->createBind(u16logPort);

    this->HookEvents();
    this->ConnectSlots();

    this->updateTimer = new QTimer(this);
    connect(this->updateTimer, &QTimer::timeout, this, &MainTask::TimedUpdate);
    this->updateTimer->start(1000);
}

MainTask::~MainTask()
{
    serverList.clear();
    delete pLogHandler;
    delete sayIter;
    delete commandIter;
}

bool MainTask::handleSignal(int signal)
{
    displayMessage(ErrorLevel::Information, "Handling signal", QString("SIGNAL: %1").arg(signal));

    if (signal == SignalHandler::SIG_INT)
    {
        displayMessage(ErrorLevel::Information, "Interrupted", "Exiting...");
        emit finished();
        return true;
    }

    return false;
}

void MainTask::initialize()
{
    displayMessage(ErrorLevel::Information, "Main task", "Starting...");

    // Initialize code goes here
    std::ifstream serversFile("servers.json");

    json j = json::parse(serversFile, nullptr, false);
    if (j.is_discarded())
    {
        cout << "Error parsing servers.json" << endl;
    }
    else
    {
        if (j.size() >= 1)
        {
            json server = j.at(0);
            cout << server["Name"].get<std::string>().c_str() << endl;
            addServerEntry(server["Name"].get<std::string>().c_str());
        }
    }

    displayMessage(ErrorLevel::Information, "Main task", "Initialized");
}

void MainTask::displayMessage(ErrorLevel level, QString title, QString message)
{
    QString levelString = "INFO: ";

    switch (level)
    {
        case ErrorLevel::Information:
            break;
        case ErrorLevel::Warning:
            levelString = "WARNING: ";
            break;
        case ErrorLevel::Error:
            levelString = "ERROR: ";
            break;
        case ErrorLevel::Critical:
            levelString = "CRITICAL (will exit): ";
            break;
    }

    cout << levelString.toStdString() << title.toStdString() << ": " << message.toStdString() << endl;

    if (level == ErrorLevel::Critical)
        emit finished();
}

AddServerError MainTask::CheckServerList(QString server)
{
    QStringList address = server.split(":");
    bool ok;

    if(address.size() == 1)
    {
        address.append("27015");
    }
    else if (address.size() != 2)
    {
        return AddServerError_Invalid;
    }

    QString ip = address.at(0);
    quint16 port = address.at(1).toInt(&ok);
    QHostAddress addr;
    AddServerError ret = AddServerError_None;

    if(!port || !ok)
    {
        return AddServerError_Invalid;
    }
    else if(!addr.setAddress(ip))
    {
        //Check if it is a hostname
        QUrl url = QUrl::fromUserInput(ip);
        if(url != QUrl())
        {
            ret = AddServerError_Hostname;
        }
        else
        {
            return AddServerError_Invalid;
        }
    }

    for(int i = 0; i < serverList.size(); i++)
    {
        //Check if the ip or hostname already exists.
        if(serverList.at(i)->hostPort == server)
        {
            return AddServerError_AlreadyExists;
        }
    }

    return ret;
}

ServerInfo *MainTask::AddServerToList(QString server, AddServerError *pError)
{
    AddServerError error = CheckServerList(server);

    if(pError != nullptr)
    {
        *pError = error;
    }

    if(error != AddServerError_None && error != AddServerError_Hostname)
    {
        return nullptr;
    }

    bool isIP = (error == AddServerError_None);
    QueryState state = (error == AddServerError_None) ? QueryRunning : QueryResolving;

    ServerInfo *info = new ServerInfo(server, state, isIP);

    serverList.append(info);

    if(isIP)
    {
        InfoQuery *infoQuery = new InfoQuery(NULL, this);
        infoQuery->query(&info->host, info->port, NULL, info);
    }
    else
    {
        //Resolve Address
        HostQueryResult *res = new HostQueryResult(info, NULL, this, NULL);
        QHostInfo::lookupHost(info->hostname, res, SLOT(HostInfoResolved(QHostInfo)));
    }
    return info;
}

void MainTask::parseLogLine(QString line, ServerInfo *info)
{
    if(!info)
        return;

    if(line.length() == 0)
    {
        return;
    }

    QString logLine = QString("L%1").arg(line);

    cout << "LOG: " << logLine.toStdString() << endl;

    while(info->logOutput.size() > 100)
        info->logOutput.removeFirst();

    info->logOutput.append(logLine);

    //Check if it is a chat event, display and save if so
    QStringList captures = actionRegex.match(logLine).capturedTexts();

    if(captures.length() == 9) //Player action 2 players
    {
        info->logHashTable.insert(captures.at(1), PlayerLogInfo(captures.at(2).toUInt(), captures.at(3)));
        info->logHashTable.insert(captures.at(5), PlayerLogInfo(captures.at(6).toUInt(), captures.at(7)));
    }
    else
    {
        captures = chatRegex.match(logLine).capturedTexts();

        if(captures.length() == 7)//We have 6, 0 = whole line. Ignore console say messages.
        {
            QString chatLine;
            if(captures.at(3) != "Console")
            {
                QString start = "<font>";
                if(captures.at(5) == "say_team")
                {
                    QString team = captures.at(4);
                    if(blueTeams.contains(team))
                    {
                        start = QString("<font style='color:#32a0f0;'>(%1 TEAM) ").arg(captures.at(4));
                    }
                    else if(redTeams.contains(team))
                    {
                        start = QString("<font style='color:#ff333a;'>(%1 TEAM) ").arg(captures.at(4));
                    }
                    else
                    {
                        start = QString("<font>(%1 TEAM) ").arg(captures.at(4));
                    }
                }

                chatLine = QString("%1%2&lt;%3&gt; : %4</font><br>").arg(start, captures.at(1).toHtmlEscaped(), captures.at(3), captures.at(6).toHtmlEscaped());
            }
            else
            {
               chatLine = QString("&lt;%1&gt;&lt;%1&gt; : %2<br>").arg(captures.at(3), captures.at(6));
            }

            cout << "CHAT: " << chatLine.toStdString() << endl;

            while(info->chatOutput.size() > 100)
                info->chatOutput.removeFirst();

            info->chatOutput.append(chatLine);
        }

        if(captures.length() >= 5)
        {
            if(captures.at(3) != "Console")
            {
                info->logHashTable.insert(captures.at(1), PlayerLogInfo(captures.at(2).toUInt(), captures.at(3)));
            }
        }
    }
}

void MainTask::ServerInfoReady(InfoReply *reply, ServerTableIndexItem *indexCell, ServerInfo* serverInfo)
{
    ServerInfo *info = indexCell ? indexCell->GetServerInfo() : serverInfo;

    if(reply)
    {
        info->lastPing = reply->ping;

        while(info->pingList.length() >= 1000)
        {
            info->pingList.removeFirst();
        }

        info->pingList.append(reply->ping);

        quint64 totalPing = 0;
        quint16 totalPings = 0;

        for(int i = 0; i < info->pingList.length(); i++)
        {
            if(info->pingList.at(i) == 2000)//only count completed pings
                continue;

            totalPing += info->pingList.at(i);
            totalPings++;
        }

        if(totalPings)
        {
            info->avgPing = totalPing/totalPings;
        }
    }

    if(reply && reply->success)
    {
        bool appIdChanged = info->appId != reply->appId;

        info->vac = reply->vac;
        info->appId = reply->appId;
        info->os = reply->os;
        info->tags = reply->tags;
        info->rawServerId = reply->rawServerId;
        info->protocol = reply->protocol;
        info->version = reply->version;
        info->currentMap = reply->map;
        info->gameName = reply->gamedesc;
        info->type = reply->type;
        info->serverNameRich = reply->hostnameRich;
        info->playerCount = QString("%1 (%3)/%2").arg(QString::number(reply->players), QString::number(reply->maxplayers), QString::number(reply->bots));
        info->haveInfo = true;
        info->serverID = reply->serverID;
        info->queryState = QuerySuccess;
        info->maxPlayers = reply->maxplayers;
        info->currentPlayers = reply->players;

        cout << info->serverNameRich.toStdString() << " " << info->gameName.toStdString() << " " << (int)info->currentPlayers << "/" << (int)info->maxPlayers << " players - Ping: " << info->lastPing << "ms" << endl;

        delete reply;
    }
    else
    {
        cout << info->hostname.toStdString() << " failed" << endl;

        if(reply)
            delete reply;
    }
}

void MainTask::PlayerInfoReady(QList<PlayerInfo> *list, ServerTableIndexItem *indexCell)
{
    ServerInfo *info = indexCell->GetServerInfo();
 
    for(int i = 0; i < list->size(); i++)
    {
        cout << "Player: " << list->at(i).name.toStdString() << " score " << list->at(i).score << " time " << list->at(i).time;

        if(info->logHashTable.contains(list->at(i).name))
        {
            cout << " SteamID " << (info->logHashTable.value(list->at(i).name).steamID).toStdString();
        }

        cout << endl;
    }

    if(list)
    {
        delete list;
    }
}

void MainTask::RulesInfoReady(QList<RulesInfo> *list, ServerTableIndexItem *indexCell)
{
    ServerInfo *info = indexCell->GetServerInfo();

    if(info->haveInfo)
    {
        list->append(RulesInfo("vac", QString::number(info->vac)));
        list->append(RulesInfo("version", info->version));
        list->append(RulesInfo("appID", QString::number(info->appId)));
        list->append(RulesInfo("os", info->os));

        if(info->rawServerId != 0)
        {
           list->append(RulesInfo("steamID64", QString::number(info->rawServerId)));
        }
    }

    for(int i = 0; i < list->size(); i++)
    {
        cout << "Rule: " << list->at(i).name.toStdString() << ": " << list->at(i).value.toStdString() << endl;
    }

    if(list)
    {
        delete list;
    }
}

void MainTask::RconAuthReady(ServerInfo *info, QList<QueuedCommand>queuedcmds)
{
    if(!info->rcon->isAuthed)
    {
        displayMessage(ErrorLevel::Warning, "RCON error", QString("Failed to authenticate %1").arg(info->hostPort));
        return;
    }
    else
    {
        info->rcon->execCommand("echo Welcome user!", false);
        QueuedCommand queuedcmd;
        foreach(queuedcmd, queuedcmds)
        {
            if(queuedcmd.commandType == QueuedCommandType::GetLogCommand)//Get log
            {
                info->rcon->execCommand(queuedcmd.command, false);
                pLogHandler->addServer(info);
            }
            else if(queuedcmd.commandType == QueuedCommandType::ContextCommand)
            {
                info->rcon->execCommand(queuedcmd.command, false);
            }
            else //command
            {
                this->runCommand(info, queuedcmd.command);
            }
        }
    }
}

void MainTask::RconOutput(ServerInfo *info, QByteArray result)
{
    if(result.length() == 0)
    {
        return;
    }

    QString resultString(result);
    cout << "RCON output: " << resultString.toStdString() << endl;

    if(info && !result.isEmpty())
    {
        while(info->rconOutput.size() > 100)
            info->rconOutput.removeFirst();

        info->rconOutput.append(result);
    }
}

void MainTask::AddRconHistory(QString chat)
{
    while(this->commandHistory.size() > 30)
        this->commandHistory.removeLast();

    this->commandHistory.prepend(chat);
    this->commandIter->toFront();

    cout << "Added to RCON history: " << chat.toStdString() << endl;
}

void MainTask::AddChatHistory(QString txt)
{
    while(this->sayHistory.size() > 30)
        this->sayHistory.removeLast();

    this->sayHistory.prepend(txt);
    this->sayIter->toFront();

    cout << "Added to chat history: " << txt.toStdString() << endl;
}

#define UPDATE_TIME 15

void MainTask::TimedUpdate()
{
    static int run = 1;

    if(run % UPDATE_TIME == 0)
    {
        for(int i = 0; i < serverList.size(); i++)
        {
            ServerInfo *info = serverList.at(i);

            if(info->queryState == QueryResolveFailed || info->queryState == QueryResolving || info->queryState == QueryRunning)
                continue;

            InfoQuery *infoQuery = new InfoQuery(NULL, this);

            info->cleanHashTable();

            infoQuery->query(&info->host, info->port, NULL, info);
        }

        // if(run % 60 == 0)
        // {
        //     this->UpdateSelectedItemInfo(false, true);
        //     run = 1;
        // }
        // else
        // {
        //     this->UpdateSelectedItemInfo(false, false);
        //     run++;
        // }
    }
    else
    {
        // for(int i = 0; i < this->ui->playerTable->rowCount(); i++)
        // {
        //     PlayerTimeTableItem *item = (PlayerTimeTableItem *)this->ui->playerTable->item(i, 3);

        //     item->updateTime(item->getTime()+1.0);
        // }
        run++;
    }
}

void MainTask::ConnectSlots()
{

}

void MainTask::HookEvents()
{

}

void MainTask::runCommand(ServerInfo *info, QString command)
{
    if(command.length() != 0)
    {
        info->rcon->execCommand(command);
        info->rconOutput.append(QString("] %1\n").arg(command));
    }
}

void MainTask::addServerEntry(QString server)
{
    if(!server.isEmpty())
    {
        AddServerError error;
        this->AddServerToList(server, &error);

        if(error == AddServerError_None || error == AddServerError_Hostname)
        {
        }
        else if(error == AddServerError_AlreadyExists)//Valid ip but exists.
        {
            displayMessage(ErrorLevel::Warning, "Add server", "Server already exists");
        }
        else
        {
            displayMessage(ErrorLevel::Warning, "Add server", "Invalid IP or Port");
        }
    }
}

void MainTask::sendChat(const QString& message)
{

}

void MainTask::passwordUpdated(const QString &)
{

}

void MainTask::rconLogin()
{

}

void MainTask::getLog()
{

}
