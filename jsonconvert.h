#ifndef __JSONCONVERT_H__
#define __JSONCONVERT_H__

#include <nlohmann/json.hpp>
#include <QString>
#include <QHostAddress>
#include "serverinfo.h"
#include "serverdefinition.h"

using json = nlohmann::json;

void to_json(json& j, const QString& s);
void from_json(const json& j, QString& s);

void to_json(json& j, const QHostAddress& hostAddress);
void from_json(const json& j, QHostAddress& hostAddress);

void to_json(json& j, const ServerInfo& info);
void from_json(const json& j, ServerInfo& info);

void to_json(json& j, const ServerDefinition& def);
void from_json(const json& j, ServerDefinition& def);

#endif
