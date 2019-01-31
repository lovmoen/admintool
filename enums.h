#ifndef __ENUMS_H__
#define  __ENUMS_H__

enum ErrorLevel
{
    Information,
    Warning,
    Error,
    Critical
};

enum AddServerError
{
    AddServerError_None,
    AddServerError_Invalid,
    AddServerError_AlreadyExists,
    AddServerError_Hostname,
};

enum ContextTypes
{
    ContextTypeNone,
    ContextTypeSteamID,
    ContextTypeName
};


#endif
