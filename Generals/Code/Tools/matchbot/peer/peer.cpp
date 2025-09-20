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

#include "peer.h"

#include <cstring>

struct PeerContext {
    PEERCallbacks callbacks{};
    bool connected{false};
};

PEER peerInitialize(const PEERCallbacks* callbacks)
{
    PeerContext* context = new PeerContext();
    if (callbacks) {
        context->callbacks = *callbacks;
    }
    context->connected = false;
    return context;
}

void peerShutdown(PEER peer)
{
    delete peer;
}

void peerDisconnect(PEER peer)
{
    if (!peer) {
        return;
    }
    peer->connected = false;
    if (peer->callbacks.disconnected) {
        peer->callbacks.disconnected(peer, "disconnected", peer->callbacks.param);
    }
}

void peerThink(PEER)
{
    // No background processing required for the stub implementation.
}

PEERBool peerIsConnected(PEER peer)
{
    return (peer && peer->connected) ? PEERTrue : PEERFalse;
}

PEERBool peerSetTitle(PEER,
                      const char*,
                      const char*,
                      const char*,
                      const char*,
                      int,
                      int,
                      PEERBool,
                      const PEERBool*,
                      const PEERBool*)
{
    return PEERTrue;
}

void peerConnect(PEER peer,
                 const char*,
                 int,
                 void (*nickError)(PEER, int, const char*, void*),
                 void (*connectCallback)(PEER, PEERBool, void*),
                 void* param,
                 PEERBool)
{
    if (!peer) {
        return;
    }

    peer->connected = true;
    if (connectCallback) {
        connectCallback(peer, PEERTrue, param);
    }
    // Notify the registered callback structure as well, if provided.
    if (peer->callbacks.disconnected == nullptr) {
        (void)nickError;
    }
}

void peerRetryWithNick(PEER, const char*)
{
    // Nothing to do for the stub implementation.
}

void peerAuthenticateCDKey(PEER peer,
                           const char*,
                           void (*callback)(PEER, int, const char*, void*),
                           void* param,
                           PEERBool)
{
    if (callback) {
        callback(peer, 1, "", param);
    }
}

void peerListGroupRooms(PEER peer,
                        const char*,
                        void (*callback)(PEER, PEERBool, int, SBServer, const char*,
                                         int, int, int, int, void*),
                        void* param,
                        PEERBool)
{
    if (callback) {
        callback(peer, PEERTrue, 1, nullptr, "QuickMatch", 0, 0, 0, 0, param);
    }
}

void peerJoinGroupRoom(PEER peer,
                       int,
                       void (*callback)(PEER, PEERBool, PEERJoinResult, RoomType, void*),
                       void* param,
                       PEERBool)
{
    if (callback) {
        callback(peer, PEERTrue, PEERJoinResult_Success, GroupRoom, param);
    }
}

void peerEnumPlayers(PEER peer,
                     RoomType room,
                     void (*callback)(PEER, PEERBool, RoomType, int, const char*, int, void*),
                     void* param)
{
    if (callback) {
        callback(peer, PEERFalse, room, -1, nullptr, 0, param);
    }
}

void peerMessagePlayer(PEER peer,
                       const char* nick,
                       const char* message,
                       MessageType messageType)
{
    (void)nick;
    (void)message;
    (void)messageType;

    (void)peer;

    // In the original GameSpy SDK this would deliver a private message to
    // another player.  The stub has no networking backend, so the call is a
    // no-op beyond parameter validation.
}
