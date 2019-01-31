#ifndef __SERVERDEFINITION_H__
#define __SERVERDEFINITION_H__

#include <QString>

struct ServerDefinition
{
    QString poolName;
    QString name;
    QString host;
    QString ip;
    QString port;
    QString rconSecret;
    QString joinSecret;
    QString authenticationKey;
    QString gameServerLoginToken;
};

#endif
