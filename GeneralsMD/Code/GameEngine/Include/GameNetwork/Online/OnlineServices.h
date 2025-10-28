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

////////////////////////////////////////////////////////////////////////////////
//                                                                                                                             /
/
//  (c) 2001-2003 Electronic Arts Inc.                                                                                         /
/
//                                                                                                                             /
/
////////////////////////////////////////////////////////////////////////////////

// FILE: OnlineServices.h //////////////////////////////////////////////////////
// Description: Neutral facades for the legacy GameSpy driven menus so that
//              the shell can degrade to an offline mode while we replace the
//              networking stack.


#ifndef __ONLINE_SERVICES_H__
#define __ONLINE_SERVICES_H__

#include "Common/STLTypedefs.h"
#include "GameClient/Color.h"
#include "GameNetwork/GameSpy/StagingRoomGameInfo.h"

class GameWindow;
class PSPlayerStats;

namespace OnlineServices
{
        using ProfileID = Int;
        using StatusCode = Int;

        enum class RosterItemType
        {
                Buddy,
                Request,
                NonBuddy,
                None,
        };

        class RosterMenuEntry
        {
        public:
                AsciiString m_nick;
                ProfileID m_profileID;
                RosterItemType m_itemType;
        };

        class BuddyInfo
        {
        public:
                ProfileID m_profileID;
                AsciiString m_name;
                AsciiString m_email;
                AsciiString m_countryCode;
                StatusCode m_status;
                UnicodeString m_statusString;
                UnicodeString m_locationString;
        };
        typedef std::map<ProfileID, BuddyInfo> BuddyInfoMap;

        class BuddyMessage
        {
        public:
                UnsignedInt m_timestamp;
                ProfileID m_senderID;
                AsciiString m_senderNick;
                ProfileID m_recipientID;
                AsciiString m_recipientNick;
                UnicodeString m_message;
        };
        typedef std::list<BuddyMessage> BuddyMessageList;

        class GroupRoom
        {
        public:
                GroupRoom()
                        : m_groupID(0),
                          m_numWaiting(0),
                          m_maxWaiting(0),
                          m_numGames(0),
                          m_numPlaying(0)
                {
                }

                AsciiString m_name;
                UnicodeString m_translatedName;
                Int m_groupID;
                Int m_numWaiting;
                Int m_maxWaiting;
                Int m_numGames;
                Int m_numPlaying;
        };
        typedef std::map<Int, GroupRoom> GroupRoomMap;

        typedef std::map<Int, GameSpyStagingRoom *> StagingRoomMap;
        typedef std::set<AsciiString> IgnoreList;
        typedef std::map<Int, AsciiString> SavedIgnoreMap;

        class PlayerInfo
        {
        public:
                PlayerInfo()
                        : m_wins(0),
                          m_losses(0),
                          m_profileID(0),
                          m_flags(0),
                          m_rankPoints(0),
                          m_side(0),
                          m_preorder(0)
                {
                }

                AsciiString m_name;
                AsciiString m_locale;
                Int m_wins;
                Int m_losses;
                Int m_profileID;
                Int m_flags;
                Int m_rankPoints;
                Int m_side;
                Int m_preorder;
                Bool isIgnored(void) const;
        };

        struct AsciiComparator
        {
                bool operator()(AsciiString s1, AsciiString s2) const;
        };
        typedef std::map<AsciiString, PlayerInfo, AsciiComparator> PlayerInfoMap;

        enum LobbyColor
        {
                LOBBY_COLOR_DEFAULT = 0,
                LOBBY_COLOR_CURRENTROOM,
                LOBBY_COLOR_ROOM,
                LOBBY_COLOR_GAME,
                LOBBY_COLOR_GAME_FULL,
                LOBBY_COLOR_GAME_CRCMISMATCH,
                LOBBY_COLOR_PLAYER_NORMAL,
                LOBBY_COLOR_PLAYER_OWNER,
                LOBBY_COLOR_PLAYER_BUDDY,
                LOBBY_COLOR_PLAYER_SELF,
                LOBBY_COLOR_PLAYER_IGNORED,
                LOBBY_COLOR_CHAT_NORMAL,
                LOBBY_COLOR_CHAT_EMOTE,
                LOBBY_COLOR_CHAT_OWNER,
                LOBBY_COLOR_CHAT_OWNER_EMOTE,
                LOBBY_COLOR_CHAT_PRIVATE,
                LOBBY_COLOR_CHAT_PRIVATE_EMOTE,
                LOBBY_COLOR_CHAT_PRIVATE_OWNER,
                LOBBY_COLOR_CHAT_PRIVATE_OWNER_EMOTE,
                LOBBY_COLOR_CHAT_BUDDY,
                LOBBY_COLOR_CHAT_SELF,
                LOBBY_COLOR_ACCEPT_TRUE,
                LOBBY_COLOR_ACCEPT_FALSE,
                LOBBY_COLOR_MAP_SELECTED,
                LOBBY_COLOR_MAP_UNSELECTED,
                LOBBY_COLOR_MOTD,
                LOBBY_COLOR_MOTD_HEADING,
                LOBBY_COLOR_MAX
        };

        extern Color LobbyPalette[LOBBY_COLOR_MAX];

        enum class BuddyStatus
        {
                Offline,
                Online,
                Lobby,
                Staging,
                Loading,
                Playing,
                Matching,
                Max
        };

        class Interface
        {
        public:
                virtual ~Interface() {}

                virtual void reset(void) {}
                virtual void clearGroupRoomList(void) = 0;
                virtual GroupRoomMap *getGroupRoomList(void) = 0;
                virtual void addGroupRoom(const GroupRoom &room) = 0;
                virtual Bool gotGroupRoomList(void) const = 0;
                virtual void joinGroupRoom(Int groupID) = 0;
                virtual void leaveGroupRoom(void) = 0;
                virtual void joinBestGroupRoom(void) = 0;
                virtual void setCurrentGroupRoom(Int groupID) = 0;
                virtual Int getCurrentGroupRoom(void) const = 0;
                virtual void updatePlayerInfo(const PlayerInfo &info, AsciiString oldNick = AsciiString::TheEmptyString) = 0;
                virtual void playerLeftGroupRoom(AsciiString nick) = 0;
                virtual PlayerInfoMap *getPlayerInfoMap(void) = 0;

                virtual BuddyInfoMap *getBuddyMap(void) = 0;
                virtual BuddyInfoMap *getBuddyRequestMap(void) = 0;
                virtual BuddyMessageList *getBuddyMessages(void) = 0;
                virtual Bool isBuddy(Int id) const = 0;

                virtual void setLocalName(AsciiString name) = 0;
                virtual AsciiString getLocalName(void) const = 0;
                virtual void setLocalProfileID(Int profileID) = 0;
                virtual Int getLocalProfileID(void) const = 0;
                virtual AsciiString getLocalEmail(void) const = 0;
                virtual void setLocalEmail(AsciiString email) = 0;
                virtual AsciiString getLocalPassword(void) const = 0;
                virtual void setLocalPassword(AsciiString passwd) = 0;
                virtual void setLocalBaseName(AsciiString name) = 0;
                virtual AsciiString getLocalBaseName(void) const = 0;

                virtual void setCachedLocalPlayerStats(PSPlayerStats stats) = 0;
                virtual PSPlayerStats getCachedLocalPlayerStats(void) const = 0;

                virtual void clearStagingRoomList(void) = 0;
                virtual StagingRoomMap *getStagingRoomList(void) = 0;
                virtual GameSpyStagingRoom *findStagingRoomByID(Int id) = 0;
                virtual void addStagingRoom(const GameSpyStagingRoom &room) = 0;
                virtual void updateStagingRoom(const GameSpyStagingRoom &room) = 0;
                virtual void removeStagingRoom(const GameSpyStagingRoom &room) = 0;
                virtual Bool hasStagingRoomListChanged(void) const = 0;
                virtual void leaveStagingRoom(void) = 0;
                virtual void markAsStagingRoomHost(void) = 0;
                virtual void markAsStagingRoomJoiner(Int gameID) = 0;
                virtual void sawFullGameList(void) = 0;

                virtual Bool amIHost(void) const = 0;
                virtual GameSpyStagingRoom *getCurrentStagingRoom(void) = 0;
                virtual void setGameOptions(void) = 0;
                virtual Int getCurrentStagingRoomID(void) const = 0;

                virtual void setDisallowAsianText(Bool val) = 0;
                virtual void setDisallowNonAsianText(Bool val) = 0;
                virtual Bool getDisallowAsianText(void) const = 0;
                virtual Bool getDisallowNonAsianText(void) const = 0;

                virtual void registerTextWindow(GameWindow *win) = 0;
                virtual void unregisterTextWindow(GameWindow *win) = 0;
                virtual Int addText(UnicodeString message, Color color, GameWindow *win) = 0;
                virtual void addChat(const PlayerInfo &player, UnicodeString message, Bool isPublic, Bool isAction, GameWindow *win) = 0;
                virtual void addChat(AsciiString nick, Int profileID, UnicodeString message, Bool isPublic, Bool isAction, GameWindow *win) = 0;
                virtual Bool sendChat(UnicodeString message, Bool isAction, GameWindow *playerListbox) = 0;

                virtual void setMOTD(const AsciiString &motd) = 0;
                virtual const AsciiString &getMOTD(void) const = 0;

                virtual void setConfig(const AsciiString &config) = 0;
                virtual const AsciiString &getConfig(void) const = 0;

                virtual void setPingString(const AsciiString &ping) = 0;
                virtual const AsciiString &getPingString(void) const = 0;
                virtual Int getPingValue(const AsciiString &otherPing) const = 0;

                virtual void addToSavedIgnoreList(Int profileID, AsciiString nick) = 0;
                virtual void removeFromSavedIgnoreList(Int profileID) = 0;
                virtual Bool isSavedIgnored(Int profileID) const = 0;
                virtual SavedIgnoreMap returnSavedIgnoreList(void) const = 0;
                virtual void loadSavedIgnoreList(void) = 0;

                virtual IgnoreList returnIgnoreList(void) const = 0;
                virtual void addToIgnoreList(AsciiString nick) = 0;
                virtual void removeFromIgnoreList(AsciiString nick) = 0;
                virtual Bool isIgnored(AsciiString nick) const = 0;

                virtual void setLocalIPs(UnsignedInt internalIP, UnsignedInt externalIP) = 0;
                virtual UnsignedInt getInternalIP(void) const = 0;
                virtual UnsignedInt getExternalIP(void) const = 0;

                virtual Bool isDisconnectedAfterGameStart(Int *reason) const = 0;
                virtual void markAsDisconnectedAfterGameStart(Int reason) = 0;

                virtual Bool didPlayerPreorder(Int profileID) const = 0;
                virtual void markPlayerAsPreorder(Int profileID) = 0;

                virtual void setMaxMessagesPerUpdate(Int num) = 0;
                virtual Int getMaxMessagesPerUpdate(void) const = 0;

                virtual Int getAdditionalDisconnects(void) = 0;
                virtual void clearAdditionalDisconnects(void) = 0;
                virtual void readAdditionalDisconnects(void) = 0;
                virtual void updateAdditionalGameSpyDisconnections(Int count) = 0;
        };

        Interface *GetServices();
        GameSpyStagingRoom *GetActiveGame();
        void Initialize(const char *motd, const char *config);
        void Shutdown();
        void ResetServices(Interface *replacement);
}

#endif // __ONLINE_SERVICES_H__
