#include "GameSpy/Peer/Peer.h"
#include "GameSpy/GP/GP.h"
#include "GameSpy/gstats/gpersist.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <cstdlib>
#include <limits>
#include <mutex>
#include <string>

namespace {
struct PeerContext {
    PEERCallbacks callbacks{};
    bool connected{false};
    int profileID{0};
    unsigned int localIP{0x7F000001u};
    std::string updatesChannel;
};

struct PersistentState {
    std::mutex mutex;
    bool connected{false};
    std::string locale;
    int wins{0};
    int losses{0};
};

PersistentState& persistentState()
{
    static PersistentState state;
    return state;
}

thread_local std::string& persistentBuffer()
{
    static thread_local std::string buffer;
    return buffer;
}

int safeParseInt(const std::string& value)
{
    if (value.empty()) {
        return 0;
    }

    char* end = nullptr;
    long converted = std::strtol(value.c_str(), &end, 10);
    if (end == value.c_str()) {
        return 0;
    }

    if (converted < 0) {
        return 0;
    }

    if (converted > std::numeric_limits<int>::max()) {
        return std::numeric_limits<int>::max();
    }

    return static_cast<int>(converted);
}

void invokeDisconnect(PeerContext* ctx, const char* reason)
{
    if (!ctx) {
        return;
    }
    ctx->connected = false;
    if (ctx->callbacks.disconnected) {
        ctx->callbacks.disconnected(ctx, reason, ctx->callbacks.param);
    }
}

void parseKeyValuePayload(const std::string& payload)
{
    auto& state = persistentState();
    std::lock_guard<std::mutex> lock(state.mutex);

    std::size_t pos = 0;
    while (pos < payload.size()) {
        if (payload[pos] == '\\') {
            ++pos;
        }
        std::size_t next = payload.find('\\', pos);
        if (next == std::string::npos) {
            break;
        }
        std::string key = payload.substr(pos, next - pos);
        pos = next + 1;
        next = payload.find('\\', pos);
        std::string value;
        if (next == std::string::npos) {
            value = payload.substr(pos);
            pos = payload.size();
        } else {
            value = payload.substr(pos, next - pos);
            pos = next;
        }

        if (key == "locale") {
            state.locale = value;
        } else if (key == "wins") {
            state.wins = std::max(0, safeParseInt(value));
        } else if (key == "losses") {
            state.losses = std::max(0, safeParseInt(value));
        }
    }
}

std::string buildPersistentPayload()
{
    auto& state = persistentState();
    std::lock_guard<std::mutex> lock(state.mutex);

    std::string payload;
    payload.reserve(64);
    payload.append("\\locale\\").append(state.locale);
    payload.append("\\wins\\").append(std::to_string(state.wins));
    payload.append("\\losses\\").append(std::to_string(state.losses));
    return payload;
}

} // namespace

// -----------------------------------------------------------------------------
// Peer SDK stubs
// -----------------------------------------------------------------------------

PEER peerInitialize(const PEERCallbacks* callbacks)
{
    PeerContext* context = new PeerContext();
    if (callbacks) {
        context->callbacks = *callbacks;
    }
    return context;
}

void peerShutdown(PEER peer)
{
    delete peer;
}

void peerDisconnect(PEER peer)
{
    invokeDisconnect(peer, "disconnected");
}

void peerThink(PEER)
{
    // No background work required in the stub implementation.
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
                      const PEERBool*,
                      const PEERBool*)
{
    return PEERTrue;
}

void peerConnect(PEER peer,
                 const char*,
                 int profileID,
                 void (*nickError)(PEER, int, const char*, void*),
                 void (*connectCallback)(PEER, PEERBool, void*),
                 void* param,
                 PEERBool)
{
    if (!peer) {
        return;
    }

    peer->connected = true;
    peer->profileID = profileID;

    if (connectCallback) {
        connectCallback(peer, PEERTrue, param);
    }

    if (nickError) {
        (void)nickError;
    }
}

void peerRetryWithNick(PEER peer, const char* nick)
{
    if (!peer) {
        return;
    }

    if (!nick) {
        invokeDisconnect(peer, "cancelled");
    }
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
                        void (*callback)(PEER, PEERBool, int, GServer, const char*,
                                         int, int, int, int, void*),
                        void* param,
                        PEERBool)
{
    if (callback) {
        callback(peer, PEERTrue, 0, nullptr, "Lobby", 0, 0, 0, 0, param);
    }
}

void peerJoinGroupRoom(PEER peer,
                       int groupID,
                       void (*callback)(PEER, PEERBool, PEERJoinResult, RoomType, void*),
                       void* param,
                       PEERBool)
{
    if (callback) {
        callback(peer, PEERTrue, PEERJoinSuccess, GroupRoom, reinterpret_cast<void*>(static_cast<intptr_t>(groupID)));
    }
}

void peerJoinStagingRoom(PEER peer,
                         GServer server,
                         const char*,
                         void (*callback)(PEER, PEERBool, PEERJoinResult, RoomType, void*),
                         void* param,
                         PEERBool)
{
    if (callback) {
        callback(peer, PEERTrue, PEERJoinSuccess, StagingRoom, server);
    }
}

void peerCreateStagingRoom(PEER peer,
                           const char*,
                           int,
                           const char*,
                           void (*callback)(PEER, PEERBool, PEERJoinResult, RoomType, void*),
                           void* param,
                           PEERBool)
{
    if (callback) {
        callback(peer, PEERTrue, PEERJoinSuccess, StagingRoom, param);
    }
}

void peerEnumPlayers(PEER peer,
                     RoomType room,
                     peerEnumPlayersCallback callback,
                     void* param)
{
    if (callback) {
        callback(peer, room, nullptr, 0, param);
    }
}

void peerMessagePlayer(PEER,
                       const char*,
                       const char*,
                       MessageType)
{
    // No networking backend is available in the stub.
}

void peerStartGame(PEER peer, const char*, int)
{
    if (peer && peer->callbacks.gameStarted) {
        peer->callbacks.gameStarted(peer, peer->localIP, "", peer->callbacks.param);
    }
}

void peerLeaveRoom(PEER peer, RoomType roomType, void*)
{
    if (peer && peer->callbacks.playerLeft) {
        peer->callbacks.playerLeft(peer, roomType, nullptr, nullptr, peer->callbacks.param);
    }
}

void peerSetReady(PEER peer, PEERBool ready)
{
    if (peer && peer->callbacks.readyChanged) {
        peer->callbacks.readyChanged(peer, nullptr, ready, peer->callbacks.param);
    }
}

void peerSetUpdatesRoomChannel(PEER peer, const char* channel)
{
    if (!peer) {
        return;
    }
    peer->updatesChannel = channel ? channel : "";
}

void peerStartListingGames(PEER peer, const char*, peerListingGamesCallback callback, void* param)
{
    if (callback) {
        callback(peer, PEERFalse, param);
    }
}

void peerStopListingGames(PEER)
{
    // Nothing to tear down in the stub implementation.
}

void peerUTMRoom(PEER peer, RoomType roomType, const char* key, const char* value, PEERBool authenticate)
{
    if (peer && peer->callbacks.roomUTM) {
        peer->callbacks.roomUTM(peer, roomType, nullptr, key, value, authenticate, peer->callbacks.param);
    }
}

void peerUTMPlayer(PEER peer, const char* nick, const char* key, const char* value, PEERBool authenticate)
{
    if (peer && peer->callbacks.playerUTM) {
        peer->callbacks.playerUTM(peer, nick, key, value, authenticate, peer->callbacks.param);
    }
}

PEERBool peerGetPlayerInfoNoWait(PEER, const char*, unsigned int* ip, int* profileID)
{
    if (ip) {
        *ip = 0x7F000001u;
    }
    if (profileID) {
        *profileID = 0;
    }
    return PEERTrue;
}

unsigned int peerGetLocalIP(PEER peer)
{
    return peer ? peer->localIP : 0;
}

void peerSetGlobalKeys(PEER,
                       int,
                       const char**,
                       const char**)
{
    // The stub implementation stores values only in the persistent data handlers.
}

void peerSetGlobalWatchKeys(PEER,
                            RoomType,
                            int,
                            const char**,
                            PEERBool)
{
    // No-op in the stub implementation.
}

// -----------------------------------------------------------------------------
// Presence & Messaging stubs
// -----------------------------------------------------------------------------

void gpConnect(GPConnection* connection,
               const char*,
               const char*,
               const char*,
               int,
               int,
               GPCallback callback,
               void* param)
{
    if (!connection) {
        return;
    }

    connection->connected = true;
    connection->profile = 1;

    if (callback) {
        GPConnectResponseArg response{};
        response.result = GP_NO_ERROR;
        response.profile = connection->profile;
        callback(connection, &response, param);
    }
}

void gpDisconnect(GPConnection* connection)
{
    if (!connection) {
        return;
    }
    connection->connected = false;
}

void gpDestroy(GPConnection* connection)
{
    if (!connection) {
        return;
    }
    connection->connected = false;
}

void gpProcess(GPConnection*)
{
    // No asynchronous processing is required in the stub implementation.
}

void gpSetStatus(GPConnection* connection, int status, const char*, const char*)
{
    if (!connection) {
        return;
    }
    connection->status = status;
}

// -----------------------------------------------------------------------------
// Persistent storage stubs
// -----------------------------------------------------------------------------

char gcd_gamename[64] = {};
char gcd_secret_key[64] = {};

int InitStatsConnection(int)
{
    auto& state = persistentState();
    std::lock_guard<std::mutex> lock(state.mutex);
    state.connected = true;
    return GE_NOERROR;
}

void CloseStatsConnection()
{
    auto& state = persistentState();
    std::lock_guard<std::mutex> lock(state.mutex);
    state.connected = false;
}

bool IsStatsConnected()
{
    auto& state = persistentState();
    std::lock_guard<std::mutex> lock(state.mutex);
    return state.connected;
}

void PersistThink()
{
    // The stub has no background events to process.
}

const char* GetChallenge(const char*)
{
    return "stub-challenge";
}

void GenerateAuth(const char*, const char*, char out[33])
{
    if (!out) {
        return;
    }
    std::fill_n(out, 32, '0');
    out[32] = '\0';
}

void PreAuthenticatePlayerPM(int localid, int profileid, const char*, persistAuthCallback callback, void* instance)
{
    if (callback) {
        callback(localid, profileid, 1, const_cast<char*>(""), instance);
    }
}

void GetPersistDataValues(int localid,
                          int profileid,
                          persisttype_t type,
                          int index,
                          const char*,
                          persistGetCallback callback,
                          void* instance)
{
    if (!callback) {
        return;
    }

    std::string payload = buildPersistentPayload();
    auto& buffer = persistentBuffer();
    buffer = payload;
    char* dataPtr = buffer.empty() ? const_cast<char*>("") : buffer.data();
    callback(localid, profileid, type, index, 1, dataPtr, static_cast<int>(buffer.size()), instance);
}

void SetPersistDataValues(int localid,
                          int profileid,
                          persisttype_t type,
                          int index,
                          char* values,
                          persistSetCallback callback,
                          void* instance)
{
    if (values) {
        parseKeyValuePayload(values);
    }

    if (callback) {
        callback(localid, profileid, type, index, 1, instance);
    }
}

