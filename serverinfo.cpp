#include "serverinfo.h"
#include "query.h"

ServerInfo::ServerInfo(QString server)
{
    this->isValid = false;
    this->appId = -1;
    this->rconOutput = "";
    this->rconPassword = "";
    this->saveRcon = false;
    this->rcon = NULL;

    QStringList address = server.split(":");
    bool ok;

    if(address.size() != 2)
        return;

    this->port = address.at(1).toInt(&ok);

    if(this->host.setAddress(address.at(0)) && port && ok)
    {
        this->ipPort = server;
        this->isValid = true;
    }
}

bool ServerInfo::isEqual(ServerInfo other) const
{
    return (this->host == other.host && this->port == other.port);
}
