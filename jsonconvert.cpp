#include "jsonconvert.h"

void to_json(json& j, const QString& s)
{
    j = s.toStdString();
}

void from_json(const json& j, QString& s)
{
    s = QString::fromStdString(j.get<std::string>());
}

void to_json(json& j, const QHostAddress& hostAddress)
{
    j = (unsigned int) hostAddress.toIPv4Address();
}

void from_json(const json& j, QHostAddress& hostAddress)
{
    hostAddress.setAddress(j.get<unsigned int>());
}

void to_json(json& j, const ServerInfo& info)
{
    j = json {
        { "poolName", info.poolName },
        { "name", info.name },
        { "joinSecret", info.joinSecret },
        { "authenticationKey", info.authenticationKey },
        { "gameServerLoginToken", info.gameServerLoginToken },
        { "vac", info.vac },
        { "protocol", info.protocol },
        { "tags", info.tags },
        { "version", info.version },
        { "os", info.os },
        { "appId", info.appId },
        { "richServerName", info.serverNameRich },
        { "currentMap", info.currentMap },
        { "nextMap", info.nextMap },
        { "ff", info.ff },
        { "timelimit", info.timelimit },
        { "host", info.hostname },
        { "mods", info.mods },
        { "playerCount", info.playerCount },
        { "gameName", info.gameName },
        { "gameType", info.type },
        { "serverId", info.serverID },
        { "ip", info.host },
        { "port", info.port },
        { "rawServerId", info.rawServerId },
        { "rconSecret", info.rconPassword },
        { "lastPing", info.lastPing },
        { "avgPing", info.avgPing },
        { "countryName", info.countryName },
        { "currentPlayers", info.currentPlayers },
        { "maxPlayers", info.maxPlayers }
    };
}

void from_json(const json& j, ServerInfo& info)
{
    j.at("poolName").get_to(info.poolName);
    j.at("name").get_to(info.name);
    j.at("joinSecret").get_to(info.joinSecret);
    j.at("authenticationKey").get_to(info.authenticationKey);
    j.at("gameServerLoginToken").get_to(info.gameServerLoginToken);
    j.at("vac").get_to(info.vac);
    j.at("protocol").get_to(info.protocol);
    j.at("tags").get_to(info.tags);
    j.at("version").get_to(info.version);
    j.at("os").get_to(info.os);
    j.at("appId").get_to(info.appId);
    j.at("richServerName").get_to(info.serverNameRich);
    j.at("currentMap").get_to(info.currentMap);
    j.at("nextMap").get_to(info.nextMap);
    j.at("ff").get_to(info.ff);
    j.at("timelimit").get_to(info.timelimit);
    j.at("host").get_to(info.hostname);
    j.at("mods").get_to(info.mods);
    j.at("playerCount").get_to(info.playerCount);
    j.at("gameName").get_to(info.gameName);
    j.at("gameType").get_to(info.type);
    j.at("serverId").get_to(info.serverID);
    j.at("ip").get_to(info.host);
    j.at("port").get_to(info.port);
    j.at("rawServerId").get_to(info.rawServerId);
    j.at("rconSecret").get_to(info.rconPassword);
    j.at("lastPing").get_to(info.lastPing);
    j.at("avgPing").get_to(info.avgPing);
    j.at("countryName").get_to(info.countryName);
    j.at("currentPlayers").get_to(info.currentPlayers);
    j.at("maxPlayers").get_to(info.maxPlayers);
}

void to_json(json& j, const ServerDefinition& serverDef)
{
    j = json {
        { "PoolName", serverDef.poolName },
        { "Name", serverDef.name },
        { "Host", serverDef.host },
        { "Ip", serverDef.ip },
        { "Port", serverDef.port },
        { "RconSecret", serverDef.rconSecret },
        { "JoinSecret", serverDef.joinSecret },
        { "AuthenticationKey", serverDef.authenticationKey },
        { "GameServerLoginToken", serverDef.gameServerLoginToken }
    };
}

void from_json(const json& j, ServerDefinition& serverDef)
{
    j.at("PoolName").get_to(serverDef.poolName);
    j.at("Name").get_to(serverDef.name);
    j.at("Host").get_to(serverDef.host);
    j.at("Ip").get_to(serverDef.ip);
    j.at("Port").get_to(serverDef.port);
    j.at("RconSecret").get_to(serverDef.rconSecret);
    j.at("JoinSecret").get_to(serverDef.joinSecret);
    j.at("AuthenticationKey").get_to(serverDef.authenticationKey);
    j.at("GameServerLoginToken").get_to(serverDef.gameServerLoginToken);
}
