/*
**      Command & Conquer Generals(tm)
**      Copyright 2025 Electronic Arts Inc.
**
**      This program is free software: you can redistribute it and/or modify
**      it under the terms of the GNU General Public License as published by
**      the Free Software Foundation, either version 3 of the License, or
**      (at your option) any later version.
**
**      This program is distributed in the hope that it will be useful,
**      but WITHOUT ANY WARRANTY; without even the implied warranty of
**      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**      GNU General Public License for more details.
**
**      You should have received a copy of the GNU General Public License
**      along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MATCHBOT_PEER_PEER_H
#define MATCHBOT_PEER_PEER_H

#include <cstddef>

enum RoomType {
    TitleRoom,
    GroupRoom,
    StagingRoom
};

constexpr std::size_t NumRooms = 3;

enum MessageType {
    NormalMessage = 0,
    ActionMessage = 1,
    NoticeMessage = 2,

    // Legacy aliases retained for compatibility with existing code.
    MessageType_Normal = NormalMessage,
    MessageType_Action = ActionMessage,
    MessageType_Notice = NoticeMessage
};

enum PEERJoinResult {
    PEERJoinResult_Success
};

constexpr int PEER_IN_USE = 1;

using SBServer = void*;

using PEERBool = int;
constexpr PEERBool PEERTrue = 1;
constexpr PEERBool PEERFalse = 0;

using CHAT = void*;
using CHATBool = int;
constexpr CHATBool CHATTrue = 1;
constexpr CHATBool CHATFalse = 0;

struct PeerContext;
using PEER = PeerContext*;

struct PEERCallbacks {
    void (*disconnected)(PEER, const char*, void*);
    void (*playerChangedNick)(PEER, RoomType, const char*, const char*, void*);
    void (*playerJoined)(PEER, RoomType, const char*, void*);
    void (*playerLeft)(PEER, RoomType, const char*, const char*, void*);
    void (*roomMessage)(PEER, RoomType, const char*, const char*, MessageType, void*);
    void (*playerMessage)(PEER, const char*, const char*, MessageType, void*);
    void* param;
};

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
                      int availableRooms,
                      int maxPlayers,
                      PEERBool persist,
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
                        void (*callback)(PEER, PEERBool, int, SBServer, const char*,
                                         int, int, int, int, void*),
                        void* param,
                        PEERBool blocking);

void peerJoinGroupRoom(PEER peer,
                       int groupID,
                       void (*callback)(PEER, PEERBool, PEERJoinResult, RoomType, void*),
                       void* param,
                       PEERBool blocking);

void peerEnumPlayers(PEER peer,
                     RoomType room,
                     void (*callback)(PEER, PEERBool, RoomType, int, const char*, int, void*),
                     void* param);

void peerMessagePlayer(PEER peer,
                       const char* nick,
                       const char* message,
                       MessageType messageType);

#endif // MATCHBOT_PEER_PEER_H
