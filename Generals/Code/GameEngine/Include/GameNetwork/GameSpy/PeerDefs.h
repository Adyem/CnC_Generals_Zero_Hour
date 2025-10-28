/*
 * Command & Conquer Generals(tm)
 * Copyright 2025 Electronic Arts Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Legacy compatibility wrapper that exposes the neutral online services
// façade through the historic GameSpy include path.


#ifndef __PEERDEFS_H__
#define __PEERDEFS_H__

#include "GameNetwork/Online/OnlineServices.h"

using IgnoreList = OnlineServices::IgnoreList;
using SavedIgnoreMap = OnlineServices::SavedIgnoreMap;
using GameSpyRCMenuData = OnlineServices::RosterMenuEntry;
using BuddyInfo = OnlineServices::BuddyInfo;
using BuddyInfoMap = OnlineServices::BuddyInfoMap;
using BuddyMessage = OnlineServices::BuddyMessage;
using BuddyMessageList = OnlineServices::BuddyMessageList;
using GameSpyGroupRoom = OnlineServices::GroupRoom;
using GroupRoomMap = OnlineServices::GroupRoomMap;
using StagingRoomMap = OnlineServices::StagingRoomMap;
using PlayerInfo = OnlineServices::PlayerInfo;
using PlayerInfoMap = OnlineServices::PlayerInfoMap;

enum GameSpyColors
{
        GSCOLOR_DEFAULT = OnlineServices::LOBBY_COLOR_DEFAULT,
        GSCOLOR_CURRENTROOM = OnlineServices::LOBBY_COLOR_CURRENTROOM,
        GSCOLOR_ROOM = OnlineServices::LOBBY_COLOR_ROOM,
        GSCOLOR_GAME = OnlineServices::LOBBY_COLOR_GAME,
        GSCOLOR_GAME_FULL = OnlineServices::LOBBY_COLOR_GAME_FULL,
        GSCOLOR_GAME_CRCMISMATCH = OnlineServices::LOBBY_COLOR_GAME_CRCMISMATCH,
        GSCOLOR_PLAYER_NORMAL = OnlineServices::LOBBY_COLOR_PLAYER_NORMAL,
        GSCOLOR_PLAYER_OWNER = OnlineServices::LOBBY_COLOR_PLAYER_OWNER,
        GSCOLOR_PLAYER_BUDDY = OnlineServices::LOBBY_COLOR_PLAYER_BUDDY,
        GSCOLOR_PLAYER_SELF = OnlineServices::LOBBY_COLOR_PLAYER_SELF,
        GSCOLOR_PLAYER_IGNORED = OnlineServices::LOBBY_COLOR_PLAYER_IGNORED,
        GSCOLOR_CHAT_NORMAL = OnlineServices::LOBBY_COLOR_CHAT_NORMAL,
        GSCOLOR_CHAT_EMOTE = OnlineServices::LOBBY_COLOR_CHAT_EMOTE,
        GSCOLOR_CHAT_OWNER = OnlineServices::LOBBY_COLOR_CHAT_OWNER,
        GSCOLOR_CHAT_OWNER_EMOTE = OnlineServices::LOBBY_COLOR_CHAT_OWNER_EMOTE,
        GSCOLOR_CHAT_PRIVATE = OnlineServices::LOBBY_COLOR_CHAT_PRIVATE,
        GSCOLOR_CHAT_PRIVATE_EMOTE = OnlineServices::LOBBY_COLOR_CHAT_PRIVATE_EMOTE,
        GSCOLOR_CHAT_PRIVATE_OWNER = OnlineServices::LOBBY_COLOR_CHAT_PRIVATE_OWNER,
        GSCOLOR_CHAT_PRIVATE_OWNER_EMOTE = OnlineServices::LOBBY_COLOR_CHAT_PRIVATE_OWNER_EMOTE,
        GSCOLOR_CHAT_BUDDY = OnlineServices::LOBBY_COLOR_CHAT_BUDDY,
        GSCOLOR_CHAT_SELF = OnlineServices::LOBBY_COLOR_CHAT_SELF,
        GSCOLOR_ACCEPT_TRUE = OnlineServices::LOBBY_COLOR_ACCEPT_TRUE,
        GSCOLOR_ACCEPT_FALSE = OnlineServices::LOBBY_COLOR_ACCEPT_FALSE,
        GSCOLOR_MAP_SELECTED = OnlineServices::LOBBY_COLOR_MAP_SELECTED,
        GSCOLOR_MAP_UNSELECTED = OnlineServices::LOBBY_COLOR_MAP_UNSELECTED,
        GSCOLOR_MOTD = OnlineServices::LOBBY_COLOR_MOTD,
        GSCOLOR_MOTD_HEADING = OnlineServices::LOBBY_COLOR_MOTD_HEADING,
        GSCOLOR_MAX = OnlineServices::LOBBY_COLOR_MAX
};

extern Color GameSpyColor[GSCOLOR_MAX];

enum GameSpyBuddyStatus
{
        BUDDY_OFFLINE = static_cast<int>(OnlineServices::BuddyStatus::Offline),
        BUDDY_ONLINE = static_cast<int>(OnlineServices::BuddyStatus::Online),
        BUDDY_LOBBY = static_cast<int>(OnlineServices::BuddyStatus::Lobby),
        BUDDY_STAGING = static_cast<int>(OnlineServices::BuddyStatus::Staging),
        BUDDY_LOADING = static_cast<int>(OnlineServices::BuddyStatus::Loading),
        BUDDY_PLAYING = static_cast<int>(OnlineServices::BuddyStatus::Playing),
        BUDDY_MATCHING = static_cast<int>(OnlineServices::BuddyStatus::Matching),
        BUDDY_MAX = static_cast<int>(OnlineServices::BuddyStatus::Max)
};

using GameSpyInfoInterface = OnlineServices::Interface;

extern GameSpyInfoInterface *TheGameSpyInfo;
extern GameSpyStagingRoom *TheGameSpyGame;

inline GameSpyInfoInterface *GetOnlineServices()
{
        return OnlineServices::GetServices();
}

void WOLDisplayGameOptions(void);
void WOLDisplaySlotList(void);
Bool GetLocalChatConnectionAddress(AsciiString serverName, UnsignedShort serverPort, UnsignedInt &localIP);
void SetLobbyAttemptHostJoin(Bool start);
class GameInfo;
void SendStatsToOtherPlayers(const GameInfo *game);

class PSPlayerStats;
void GetAdditionalDisconnectsFromUserFile(PSPlayerStats *stats);
extern Int GetAdditionalDisconnectsFromUserFile(Int playerID);

void SetUpGameSpy(const char *motdBuffer, const char *configBuffer);
void TearDownGameSpy(void);

#endif // __PEERDEFS_H__
