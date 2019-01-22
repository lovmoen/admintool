#include "mainwindow.h"
#include "util.h"
#include <QFile>

#include <maxminddb.h>

ServerInfo::ServerInfo(QString server, QueryState state, bool isIP)
{
    this->appId = -1;
    this->rconPassword = "";
    this->saveRcon = false;
    this->rcon = nullptr;
    this->vac = 0;
    this->version = "";
    this->os = "";
    this->tags = "";
    this->haveInfo = false;
    this->queryState = state;

    QStringList address = server.split(":");
    if (address.count() == 1)
    {
        address.append("27015");
    }

    if(isIP)
    {
        this->host = QHostAddress(address.at(0));
        this->GetCountryFlag();
    }
    else
    {
        this->hostname = address.at(0);
    }
    this->hostPort = server;
    this->port = address.at(1).toInt();
}

bool ServerInfo::isEqual(ServerInfo *other) const
{
    return (this->host == other->host && this->port == other->port);
}

void ServerInfo::cleanHashTable()
{
    QList<QString> keys = this->logHashTable.keys();
    for(int i = 0; i < keys.length(); i++)
    {
        PlayerLogInfo info = this->logHashTable.value(keys.at(i));
        if(info.time+(1000*60*30) < QDateTime::currentMSecsSinceEpoch())//remove the key if its older than 30 minutes
        {
            this->logHashTable.remove(keys.at(i));
        }
    }
}

void ServerInfo::GetCountryFlag()
{
    GetCountryName();

    if (countryName.isNull() || countryName.isEmpty())
    {
        qDebug() << "Country name not set.";
        return;
    }

    QString flagPath = QString(":/icons/icons/countries/%1.png").arg(countryName);
    if (QFile::exists(flagPath))
    {
        countryFlag.load(flagPath);
    }
    else
    {
        qDebug() << "Flag icon does not exist at " << flagPath << ".";
    }
}

void ServerInfo::GetCountryName()
{
    countryName = "";
    if(!this->host.toString().isEmpty())
    {
        MMDB_s mmdb;
        int status = MMDB_open(BuildPath("GeoLite2-Country.mmdb").toUtf8().data(), MMDB_MODE_MMAP, &mmdb);
        if (status == MMDB_SUCCESS)
        {
            int gai_error, mmdb_error;
            MMDB_lookup_result_s results = MMDB_lookup_string(&mmdb, this->host.toString().toLatin1().data(), &gai_error, &mmdb_error);
            if (gai_error == 0 && mmdb_error == MMDB_SUCCESS && results.found_entry)
            {
                MMDB_entry_data_s entry_data;
                int res = MMDB_get_value(&results.entry, &entry_data, "country", "iso_code", NULL);
                if (res == MMDB_SUCCESS && entry_data.has_data && entry_data.type == MMDB_DATA_TYPE_UTF8_STRING)
                {
                    countryName = QString(QByteArray::fromRawData(entry_data.utf8_string, entry_data.data_size)).toLower();
                }
                else
                {
                    qDebug() << "Bad entry. MMDBerror " << MMDB_strerror(res) << ", HasData: " << entry_data.has_data << ", DataType: " << entry_data.utf8_string;
                }
            }
            else
            {
                qDebug() << "Lookup failure. gai: " << gai_error << ", MMDBerror " << MMDB_strerror(mmdb_error) << " (" << mmdb_error << ")";
            }

            MMDB_close(&mmdb);
        }
        else
        {
            qDebug() << "Failed to open MaxMind db (" << MMDB_strerror(status) << ")";
        }
    }
}

void HostQueryResult::HostInfoResolved(QHostInfo hostInfo)
{
    QHostAddress addr;
    foreach(addr, hostInfo.addresses())
    {
        if(!addr.isNull() && addr.protocol() == QAbstractSocket::IPv4Protocol)
        {
            this->info->host = addr;
            this->info->queryState = QueryRunning;

            if (this->mainWindow)
            {
                this->mainWindow->CreateTableItemOrUpdate(id->row(), kBrowserColHostname, id->tableWidget(), this->info);
                this->info->GetCountryFlag();
            }
            else if (this->mainTask)
            {
                this->info->GetCountryName();
            }

            InfoQuery *infoQuery = new InfoQuery(this->mainWindow, this->mainTask);
            infoQuery->query(& this->info->host,  this->info->port, this->id, this->info);
            this->deleteLater();
            return;
        }
    }
    
    info->queryState = QueryResolveFailed;

    if (this->mainWindow)
        this->mainWindow->CreateTableItemOrUpdate(id->row(), kBrowserColHostname, id->tableWidget(), this->info);

    this->deleteLater();
}
