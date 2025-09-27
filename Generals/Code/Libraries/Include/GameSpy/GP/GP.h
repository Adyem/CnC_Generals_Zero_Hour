#pragma once

#include <cstdint>

using GPProfile = int;
using GPResult = int;

constexpr GPResult GP_NO_ERROR = 0;
constexpr GPResult GP_MEMORY_ERROR = 1;
constexpr GPResult GP_PARAMETER_ERROR = 2;
constexpr GPResult GP_NETWORK_ERROR = 3;
constexpr GPResult GP_SERVER_ERROR = 4;

constexpr int GP_FIREWALL = 0;
constexpr int GP_BLOCKING = 0;
constexpr int GP_NON_BLOCKING = 1;

constexpr int GP_STATUS_OFFLINE = 0;
constexpr int GP_STATUS_ONLINE = 1;
constexpr int GP_CHATTING = 2;
constexpr int GP_ONLINE = GP_STATUS_ONLINE;

constexpr int GP_GENERAL = 0;
constexpr int GP_PARSE = 1;
constexpr int GP_NOT_LOGGED_IN = 2;
constexpr int GP_BAD_SESSKEY = 3;
constexpr int GP_DATABASE = 4;
constexpr int GP_NETWORK = 5;
constexpr int GP_FORCED_DISCONNECT = 6;
constexpr int GP_CONNECTION_CLOSED = 7;
constexpr int GP_LOGIN = 8;
constexpr int GP_LOGIN_TIMEOUT = 9;
constexpr int GP_LOGIN_BAD_NICK = 10;
constexpr int GP_LOGIN_BAD_EMAIL = 11;
constexpr int GP_LOGIN_BAD_PASSWORD = 12;
constexpr int GP_LOGIN_BAD_PROFILE = 13;
constexpr int GP_LOGIN_PROFILE_DELETED = 14;
constexpr int GP_LOGIN_CONNECTION_FAILED = 15;
constexpr int GP_LOGIN_SERVER_AUTH_FAILED = 16;
constexpr int GP_NEWUSER = 17;
constexpr int GP_NEWUSER_BAD_NICK = 18;
constexpr int GP_NEWUSER_BAD_PASSWORD = 19;
constexpr int GP_UPDATEUI = 20;
constexpr int GP_UPDATEUI_BAD_EMAIL = 21;
constexpr int GP_NEWPROFILE = 22;
constexpr int GP_NEWPROFILE_BAD_NICK = 23;
constexpr int GP_NEWPROFILE_BAD_OLD_NICK = 24;
constexpr int GP_UPDATEPRO = 25;
constexpr int GP_UPDATEPRO_BAD_NICK = 26;
constexpr int GP_ADDBUDDY = 27;
constexpr int GP_ADDBUDDY_BAD_FROM = 28;
constexpr int GP_ADDBUDDY_BAD_NEW = 29;
constexpr int GP_ADDBUDDY_ALREADY_BUDDY = 30;
constexpr int GP_AUTHADD = 31;
constexpr int GP_AUTHADD_BAD_FROM = 32;
constexpr int GP_AUTHADD_BAD_SIG = 33;
constexpr int GP_STATUS = 34;
constexpr int GP_BM = 35;
constexpr int GP_BM_NOT_BUDDY = 36;
constexpr int GP_GETPROFILE = 37;
constexpr int GP_GETPROFILE_BAD_PROFILE = 38;
constexpr int GP_DELBUDDY = 39;
constexpr int GP_DELBUDDY_NOT_BUDDY = 40;
constexpr int GP_DELPROFILE = 41;
constexpr int GP_DELPROFILE_LAST_PROFILE = 42;
constexpr int GP_SEARCH = 43;
constexpr int GP_SEARCH_CONNECTION_FAILED = 44;

struct GPConnection;

using GPCallback = void(*)(GPConnection*, void*, void*);

enum GPCallbackType {
    GP_ERROR = 0,
    GP_RECV_BUDDY_REQUEST,
    GP_RECV_BUDDY_MESSAGE,
    GP_RECV_BUDDY_STATUS,
    GP_CALLBACK_COUNT
};

struct GPConnectResponseArg {
    GPResult result{GP_PARAMETER_ERROR};
    GPProfile profile{0};
};

struct GPErrorArg {
    GPResult result{GP_PARAMETER_ERROR};
    int errorCode{GP_GENERAL};
    const char* errorString{nullptr};
    int fatal{0};
};

struct GPRecvBuddyMessageArg {
    GPProfile profile{0};
    const char* message{nullptr};
};

struct GPRecvBuddyStatusArg {
    GPProfile profile{0};
    int index{0};
};

struct GPRecvBuddyRequestArg {
    GPProfile profile{0};
    const char* reason{nullptr};
};

struct GPConnection {
    GPProfile profile{0};
    bool connected{false};
    GPCallback callbacks[GP_CALLBACK_COUNT]{};
    void* callbackParams[GP_CALLBACK_COUNT]{};
    int status{GP_STATUS_OFFLINE};
};

inline GPResult gpInitialize(GPConnection* connection, int) {
    if (!connection) {
        return GP_PARAMETER_ERROR;
    }
    connection->connected = false;
    connection->profile = 0;
    connection->status = GP_STATUS_OFFLINE;
    for (int i = 0; i < GP_CALLBACK_COUNT; ++i) {
        connection->callbacks[i] = nullptr;
        connection->callbackParams[i] = nullptr;
    }
    return GP_NO_ERROR;
}

inline void gpSetCallback(GPConnection* connection, int callbackType, GPCallback callback, void* param) {
    if (!connection || callbackType < 0 || callbackType >= GP_CALLBACK_COUNT) {
        return;
    }
    connection->callbacks[callbackType] = callback;
    connection->callbackParams[callbackType] = param;
}

void gpConnect(GPConnection* connection,
               const char* nick,
               const char* email,
               const char* password,
               int firewall,
               int blocking,
               GPCallback callback,
               void* param);

void gpDisconnect(GPConnection* connection);
void gpDestroy(GPConnection* connection);
void gpProcess(GPConnection* connection);
void gpSetStatus(GPConnection* connection, int status, const char* statusString, const char* locationString);

#endif // GAMESPY_GP_GP_H
