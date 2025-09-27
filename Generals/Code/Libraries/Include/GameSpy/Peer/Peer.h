#pragma once

#include <cstddef>
#include <cstdint>

enum RoomType {
    TitleRoom = 0,
    GroupRoom,
    StagingRoom,
    NumRooms
};

enum MessageType {
    NormalMessage = 0,
    ActionMessage = 1,
    NoticeMessage = 2
};

using GServer = void*;
using SBServer = void*;

using PEERBool = int;
constexpr PEERBool PEERTrue = 1;
constexpr PEERBool PEERFalse = 0;

constexpr int PEER_IN_USE = 1;
constexpr int PEER_STOP_REPORTING = 0;
constexpr int PEER_FLAG_OP = 1 << 0;

enum PEERJoinResult {
    PEERJoinSuccess = 0,
    PEERJoinFailed = 1
};

struct PeerContext;
using PEER = PeerContext*;

struct PEERCallbacks {
    void (*disconnected)(PEER, const char*, void*);
    void (*readyChanged)(PEER, const char*, PEERBool, void*);
    void (*roomMessage)(PEER, RoomType, const char*, const char*, MessageType, void*);
    void (*playerMessage)(PEER, const char*, const char*, MessageType, void*);
    void (*gameStarted)(PEER, unsigned int, const char*, void*);
    void (*playerJoined)(PEER, RoomType, const char*, void*);
    void (*playerLeft)(PEER, RoomType, const char*, const char*, void*);
    void (*playerChangedNick)(PEER, RoomType, const char*, const char*, void*);
    void (*ping)(PEER, const char*, int, void*);
    void (*crossPing)(PEER, const char*, const char*, int, void*);
    void (*roomUTM)(PEER, RoomType, const char*, const char*, const char*, PEERBool, void*);
    void (*playerUTM)(PEER, const char*, const char*, const char*, PEERBool, void*);
    void (*GOABasic)(PEER, PEERBool, char*, int, void*);
    void (*GOAInfo)(PEER, PEERBool, char*, int, void*);
    void (*GOARules)(PEER, PEERBool, char*, int, void*);
    void (*GOAPlayers)(PEER, PEERBool, char*, int, void*);
    void* param;
};

using peerEnumPlayersCallback = void(*)(PEER, RoomType, const char*, int, void*);
using peerListingGamesCallback = void(*)(PEER, PEERBool, void*);

PEER peerInitialize(const PEERCallbacks* callbacks);
void peerShutdown(PEER peer);
void peerDisconnect(PEER peer);

void peerThink(PEER peer);
PEERBool peerIsConnected(PEER peer);

PEERBool peerSetTitle(PEER peer,
                      const char* title,
                      const char* secretKey,
                      const char* productID,
                      const char* uniques,
                      int maxPlayers,
                      const PEERBool* pingRooms,
                      const PEERBool* crossPingRooms);

void peerConnect(PEER peer,
                 const char* nick,
                 int profileID,
                 void (*nickError)(PEER, int, const char*, void*),
                 void (*connectCallback)(PEER, PEERBool, void*),
                 void* param,
                 PEERBool blocking);

void peerRetryWithNick(PEER peer, const char* nick);

void peerAuthenticateCDKey(PEER peer,
                           const char* cdKey,
                           void (*callback)(PEER, int, const char*, void*),
                           void* param,
                           PEERBool blocking);

void peerListGroupRooms(PEER peer,
                        const char* filter,
                        void (*callback)(PEER, PEERBool, int, GServer, const char*,
                                         int, int, int, int, void*),
                        void* param,
                        PEERBool blocking);

void peerJoinGroupRoom(PEER peer,
                       int groupID,
                       void (*callback)(PEER, PEERBool, PEERJoinResult, RoomType, void*),
                       void* param,
                       PEERBool blocking);

void peerJoinStagingRoom(PEER peer,
                         GServer server,
                         const char* password,
                         void (*callback)(PEER, PEERBool, PEERJoinResult, RoomType, void*),
                         void* param,
                         PEERBool blocking);

void peerCreateStagingRoom(PEER peer,
                           const char* gameName,
                           int maxPlayers,
                           const char* password,
                           void (*callback)(PEER, PEERBool, PEERJoinResult, RoomType, void*),
                           void* param,
                           PEERBool blocking);

void peerEnumPlayers(PEER peer,
                     RoomType room,
                     peerEnumPlayersCallback callback,
                     void* param);

void peerMessagePlayer(PEER peer,
                       const char* nick,
                       const char* message,
                       MessageType messageType);

void peerStartGame(PEER peer, const char* address, int flags);
void peerLeaveRoom(PEER peer, RoomType roomType, void* param);
void peerSetReady(PEER peer, PEERBool ready);

void peerSetUpdatesRoomChannel(PEER peer, const char* channel);
void peerStartListingGames(PEER peer, const char* filter, peerListingGamesCallback callback, void* param);
void peerStopListingGames(PEER peer);

void peerUTMRoom(PEER peer, RoomType roomType, const char* key, const char* value, PEERBool authenticate);
void peerUTMPlayer(PEER peer, const char* nick, const char* key, const char* value, PEERBool authenticate);

PEERBool peerGetPlayerInfoNoWait(PEER peer, const char* nick, unsigned int* ip, int* profileID);
unsigned int peerGetLocalIP(PEER peer);

void peerSetGlobalKeys(PEER peer, int numKeys, const char** keys, const char** values);
void peerSetGlobalWatchKeys(PEER peer, RoomType room, int numKeys, const char** keys, PEERBool add);

