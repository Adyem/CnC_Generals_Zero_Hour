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

#include "PreRTS.h"

#include "GameNetwork/Online/OnlineServices.h"

#include "Common/IgnorePreferences.h"

#include <cstdlib>

extern void GetAdditionalDisconnectsFromUserFile(PSPlayerStats *stats);
extern Int GetAdditionalDisconnectsFromUserFile(Int playerID);

namespace OnlineServices
{
        namespace
        {
                class NullOnlineServices : public Interface
                {
                public:
                        NullOnlineServices();
                        virtual ~NullOnlineServices();

                        virtual void reset(void) override;
                        virtual void clearGroupRoomList(void) override;
                        virtual GroupRoomMap *getGroupRoomList(void) override;
                        virtual void addGroupRoom(const GroupRoom &room) override;
                        virtual Bool gotGroupRoomList(void) const override { return m_gotGroupRoomList; }
                        virtual void joinGroupRoom(Int groupID) override;
                        virtual void leaveGroupRoom(void) override;
                        virtual void joinBestGroupRoom(void) override;
                        virtual void setCurrentGroupRoom(Int groupID) override;
                        virtual Int getCurrentGroupRoom(void) const override { return m_currentGroupRoomID; }
                        virtual void updatePlayerInfo(const PlayerInfo &info, AsciiString oldNick) override;
                        virtual void playerLeftGroupRoom(AsciiString nick) override;
                        virtual PlayerInfoMap *getPlayerInfoMap(void) override { return &m_playerInfoMap; }

                        virtual BuddyInfoMap *getBuddyMap(void) override { return &m_buddyMap; }
                        virtual BuddyInfoMap *getBuddyRequestMap(void) override { return &m_buddyRequestMap; }
                        virtual BuddyMessageList *getBuddyMessages(void) override { return &m_buddyMessages; }
                        virtual Bool isBuddy(Int id) const override;

                        virtual void setLocalName(AsciiString name) override { m_localName = name; }
                        virtual AsciiString getLocalName(void) const override { return m_localName; }
                        virtual void setLocalProfileID(Int profileID) override { m_localProfileID = profileID; }
                        virtual Int getLocalProfileID(void) const override { return m_localProfileID; }
                        virtual AsciiString getLocalEmail(void) const override { return m_localEmail; }
                        virtual void setLocalEmail(AsciiString email) override { m_localEmail = email; }
                        virtual AsciiString getLocalPassword(void) const override { return m_localPassword; }
                        virtual void setLocalPassword(AsciiString passwd) override { m_localPassword = passwd; }
                        virtual void setLocalBaseName(AsciiString name) override { m_localBaseName = name; }
                        virtual AsciiString getLocalBaseName(void) const override { return m_localBaseName; }

                        virtual void setCachedLocalPlayerStats(PSPlayerStats stats) override { m_cachedLocalPlayerStats = stats; }
                        virtual PSPlayerStats getCachedLocalPlayerStats(void) const override { return m_cachedLocalPlayerStats; }

                        virtual void clearStagingRoomList(void) override;
                        virtual StagingRoomMap *getStagingRoomList(void) override { return &m_stagingRooms; }
                        virtual GameSpyStagingRoom *findStagingRoomByID(Int id) override;
                        virtual void addStagingRoom(const GameSpyStagingRoom &room) override;
                        virtual void updateStagingRoom(const GameSpyStagingRoom &room) override;
                        virtual void removeStagingRoom(const GameSpyStagingRoom &room) override;
                        virtual Bool hasStagingRoomListChanged(void) const override { return m_stagingRoomsDirty; }
                        virtual void leaveStagingRoom(void) override;
                        virtual void markAsStagingRoomHost(void) override;
                        virtual void markAsStagingRoomJoiner(Int gameID) override;
                        virtual void sawFullGameList(void) override { m_sawFullGameList = TRUE; }

                        virtual Bool amIHost(void) const override { return m_isHosting; }
                        virtual GameSpyStagingRoom *getCurrentStagingRoom(void) override;
                        virtual void setGameOptions(void) override {}
                        virtual Int getCurrentStagingRoomID(void) const override { return m_localStagingRoomID; }

                        virtual void setDisallowAsianText(Bool val) override { m_disallowAsianText = val; }
                        virtual void setDisallowNonAsianText(Bool val) override { m_disallowNonAsianText = val; }
                        virtual Bool getDisallowAsianText(void) const override { return m_disallowAsianText; }
                        virtual Bool getDisallowNonAsianText(void) const override { return m_disallowNonAsianText; }

                        virtual void registerTextWindow(GameWindow *win) override;
                        virtual void unregisterTextWindow(GameWindow *win) override;
                        virtual Int addText(UnicodeString message, Color color, GameWindow *win) override;
                        virtual void addChat(const PlayerInfo &player, UnicodeString message, Bool isPublic, Bool isAction, GameWindow *win) override;
                        virtual void addChat(AsciiString nick, Int profileID, UnicodeString message, Bool isPublic, Bool isAction, GameWindow *win) override;
                        virtual Bool sendChat(UnicodeString message, Bool isAction, GameWindow *playerListbox) override;

                        virtual void setMOTD(const AsciiString &motd) override { m_rawMotd = motd; }
                        virtual const AsciiString &getMOTD(void) const override { return m_rawMotd; }

                        virtual void setConfig(const AsciiString &config) override { m_rawConfig = config; }
                        virtual const AsciiString &getConfig(void) const override { return m_rawConfig; }

                        virtual void setPingString(const AsciiString &ping) override { m_pingString = ping; }
                        virtual const AsciiString &getPingString(void) const override { return m_pingString; }
                        virtual Int getPingValue(const AsciiString &otherPing) const override;

                        virtual void addToSavedIgnoreList(Int profileID, AsciiString nick) override;
                        virtual void removeFromSavedIgnoreList(Int profileID) override;
                        virtual Bool isSavedIgnored(Int profileID) const override;
                        virtual SavedIgnoreMap returnSavedIgnoreList(void) const override { return m_savedIgnoreMap; }
                        virtual void loadSavedIgnoreList(void) override;

                        virtual IgnoreList returnIgnoreList(void) const override { return m_ignoreList; }
                        virtual void addToIgnoreList(AsciiString nick) override;
                        virtual void removeFromIgnoreList(AsciiString nick) override;
                        virtual Bool isIgnored(AsciiString nick) const override;

                        virtual void setLocalIPs(UnsignedInt internalIP, UnsignedInt externalIP) override;
                        virtual UnsignedInt getInternalIP(void) const override { return m_internalIP; }
                        virtual UnsignedInt getExternalIP(void) const override { return m_externalIP; }

                        virtual Bool isDisconnectedAfterGameStart(Int *reason) const override;
                        virtual void markAsDisconnectedAfterGameStart(Int reason) override;

                        virtual Bool didPlayerPreorder(Int profileID) const override;
                        virtual void markPlayerAsPreorder(Int profileID) override;

                        virtual void setMaxMessagesPerUpdate(Int num) override { m_maxMessagesPerUpdate = num; }
                        virtual Int getMaxMessagesPerUpdate(void) const override { return m_maxMessagesPerUpdate; }

                        virtual Int getAdditionalDisconnects(void) override { return m_additionalDisconnects; }
                        virtual void clearAdditionalDisconnects(void) override { m_additionalDisconnects = 0; }
                        virtual void readAdditionalDisconnects(void) override;
                        virtual void updateAdditionalGameSpyDisconnections(Int count) override;

                private:
                        void clearTextWindows();
                        void destroyStagingRooms();

                        GroupRoomMap m_groupRooms;
                        Bool m_gotGroupRoomList;
                        PlayerInfoMap m_playerInfoMap;
                        BuddyInfoMap m_buddyMap;
                        BuddyInfoMap m_buddyRequestMap;
                        BuddyMessageList m_buddyMessages;
                        IgnoreList m_ignoreList;
                        SavedIgnoreMap m_savedIgnoreMap;

                        StagingRoomMap m_stagingRooms;
                        Bool m_stagingRoomsDirty;
                        Bool m_sawFullGameList;
                        Bool m_isHosting;
                        Int m_currentGroupRoomID;
                        Int m_localProfileID;
                        Int m_localStagingRoomID;
                        Int m_joinedStagingRoom;
                        GameSpyStagingRoom m_localStagingRoom;

                        AsciiString m_localName;
                        AsciiString m_localBaseName;
                        AsciiString m_localEmail;
                        AsciiString m_localPassword;
                        PSPlayerStats m_cachedLocalPlayerStats;

                        Bool m_disallowAsianText;
                        Bool m_disallowNonAsianText;
                        std::set<GameWindow *> m_textWindows;

                        AsciiString m_rawMotd;
                        AsciiString m_rawConfig;
                        AsciiString m_pingString;

                        UnsignedInt m_internalIP;
                        UnsignedInt m_externalIP;

                        Bool m_disconnectedAfterStart;
                        Int m_disconnectReason;

                        std::set<Int> m_preorderPlayers;

                        Int m_maxMessagesPerUpdate;
                        Int m_additionalDisconnects;
                };

                NullOnlineServices::NullOnlineServices()
                        : m_gotGroupRoomList(FALSE),
                          m_stagingRoomsDirty(FALSE),
                          m_sawFullGameList(FALSE),
                          m_isHosting(FALSE),
                          m_currentGroupRoomID(0),
                          m_localProfileID(0),
                          m_localStagingRoomID(0),
                          m_joinedStagingRoom(0),
                          m_disallowAsianText(FALSE),
                          m_disallowNonAsianText(FALSE),
                          m_internalIP(0),
                          m_externalIP(0),
                          m_disconnectedAfterStart(FALSE),
                          m_disconnectReason(0),
                          m_maxMessagesPerUpdate(100),
                          m_additionalDisconnects(-1)
                {
                }

                NullOnlineServices::~NullOnlineServices()
                {
                        destroyStagingRooms();
                }

                void NullOnlineServices::destroyStagingRooms()
                {
                        for (StagingRoomMap::iterator it = m_stagingRooms.begin(); it != m_stagingRooms.end(); ++it)
                        {
                                delete it->second;
                        }
                        m_stagingRooms.clear();
                }

                void NullOnlineServices::reset(void)
                {
                        clearGroupRoomList();
                        clearStagingRoomList();
                        m_playerInfoMap.clear();
                        m_buddyMap.clear();
                        m_buddyRequestMap.clear();
                        m_buddyMessages.clear();
                        m_ignoreList.clear();
                        m_savedIgnoreMap.clear();
                        m_preorderPlayers.clear();
                        m_localName.clear();
                        m_localBaseName.clear();
                        m_localEmail.clear();
                        m_localPassword.clear();
                        m_pingString.clear();
                        m_rawConfig.clear();
                        m_rawMotd.clear();
                        m_currentGroupRoomID = 0;
                        m_localProfileID = 0;
                        m_localStagingRoomID = 0;
                        m_joinedStagingRoom = 0;
                        m_isHosting = FALSE;
                        m_sawFullGameList = FALSE;
                        m_gotGroupRoomList = FALSE;
                        m_stagingRoomsDirty = FALSE;
                        m_disallowAsianText = FALSE;
                        m_disallowNonAsianText = FALSE;
                        m_internalIP = 0;
                        m_externalIP = 0;
                        m_disconnectedAfterStart = FALSE;
                        m_disconnectReason = 0;
                        m_maxMessagesPerUpdate = 100;
                        m_additionalDisconnects = -1;
                        clearTextWindows();
                        m_cachedLocalPlayerStats.reset();
                        m_localStagingRoom.reset();
                }

                void NullOnlineServices::clearGroupRoomList(void)
                {
                        m_groupRooms.clear();
                        m_gotGroupRoomList = FALSE;
                }

                GroupRoomMap *NullOnlineServices::getGroupRoomList(void)
                {
                        return &m_groupRooms;
                }

                void NullOnlineServices::addGroupRoom(const GroupRoom &room)
                {
                        m_groupRooms[room.m_groupID] = room;
                        m_gotGroupRoomList = TRUE;
                }

                void NullOnlineServices::joinGroupRoom(Int groupID)
                {
                        m_currentGroupRoomID = groupID;
                        m_joinedStagingRoom = 0;
                }

                void NullOnlineServices::leaveGroupRoom(void)
                {
                        m_currentGroupRoomID = 0;
                        m_joinedStagingRoom = 0;
                }

                void NullOnlineServices::joinBestGroupRoom(void)
                {
                        if (!m_groupRooms.empty())
                        {
                                m_currentGroupRoomID = m_groupRooms.begin()->first;
                        }
                }

                void NullOnlineServices::setCurrentGroupRoom(Int groupID)
                {
                        m_currentGroupRoomID = groupID;
                        m_playerInfoMap.clear();
                }

                void NullOnlineServices::updatePlayerInfo(const PlayerInfo &info, AsciiString oldNick)
                {
                        if (oldNick != AsciiString::TheEmptyString)
                        {
                                m_playerInfoMap.erase(oldNick);
                        }
                        m_playerInfoMap[info.m_name] = info;
                }

                void NullOnlineServices::playerLeftGroupRoom(AsciiString nick)
                {
                        m_playerInfoMap.erase(nick);
                }

                Bool NullOnlineServices::isBuddy(Int id) const
                {
                        return (m_buddyMap.find(id) != m_buddyMap.end());
                }

                void NullOnlineServices::clearStagingRoomList(void)
                {
                        destroyStagingRooms();
                        m_stagingRoomsDirty = TRUE;
                }

                GameSpyStagingRoom *NullOnlineServices::findStagingRoomByID(Int id)
                {
                        StagingRoomMap::iterator it = m_stagingRooms.find(id);
                        if (it != m_stagingRooms.end())
                        {
                                return it->second;
                        }
                        return NULL;
                }

                void NullOnlineServices::addStagingRoom(const GameSpyStagingRoom &room)
                {
                        GameSpyStagingRoom *copy = new GameSpyStagingRoom(room);
                        m_stagingRooms[room.getID()] = copy;
                        m_stagingRoomsDirty = TRUE;
                }

                void NullOnlineServices::updateStagingRoom(const GameSpyStagingRoom &room)
                {
                        StagingRoomMap::iterator it = m_stagingRooms.find(room.getID());
                        if (it != m_stagingRooms.end())
                        {
                                *(it->second) = room;
                                m_stagingRoomsDirty = TRUE;
                        }
                }

                void NullOnlineServices::removeStagingRoom(const GameSpyStagingRoom &room)
                {
                        StagingRoomMap::iterator it = m_stagingRooms.find(room.getID());
                        if (it != m_stagingRooms.end())
                        {
                                delete it->second;
                                m_stagingRooms.erase(it);
                                m_stagingRoomsDirty = TRUE;
                        }
                }

                void NullOnlineServices::leaveStagingRoom(void)
                {
                        m_joinedStagingRoom = 0;
                        m_isHosting = FALSE;
                        m_localStagingRoom.reset();
                }

                void NullOnlineServices::markAsStagingRoomHost(void)
                {
                        m_isHosting = TRUE;
                        m_localStagingRoomID = m_localStagingRoom.getID();
                }

                void NullOnlineServices::markAsStagingRoomJoiner(Int gameID)
                {
                        m_isHosting = FALSE;
                        m_joinedStagingRoom = gameID;
                }

                GameSpyStagingRoom *NullOnlineServices::getCurrentStagingRoom(void)
                {
                        if (m_isHosting)
                        {
                                return &m_localStagingRoom;
                        }
                        if (m_joinedStagingRoom != 0)
                        {
                                GameSpyStagingRoom *joined = findStagingRoomByID(m_joinedStagingRoom);
                                if (joined)
                                {
                                        return joined;
                                }
                        }
                        return &m_localStagingRoom;
                }

                void NullOnlineServices::registerTextWindow(GameWindow *win)
                {
                        if (win)
                        {
                                m_textWindows.insert(win);
                        }
                }

                void NullOnlineServices::unregisterTextWindow(GameWindow *win)
                {
                        if (win)
                        {
                                m_textWindows.erase(win);
                        }
                }

                void NullOnlineServices::clearTextWindows()
                {
                        m_textWindows.clear();
                }

                Int NullOnlineServices::addText(UnicodeString, Color, GameWindow *)
                {
                        return 0;
                }

                void NullOnlineServices::addChat(const PlayerInfo &, UnicodeString, Bool, Bool, GameWindow *)
                {
                }

                void NullOnlineServices::addChat(AsciiString, Int, UnicodeString, Bool, Bool, GameWindow *)
                {
                }

                Bool NullOnlineServices::sendChat(UnicodeString, Bool, GameWindow *)
                {
                        return FALSE;
                }

                Int NullOnlineServices::getPingValue(const AsciiString &otherPing) const
                {
                        return atoi(otherPing.str());
                }

                void NullOnlineServices::addToSavedIgnoreList(Int profileID, AsciiString nick)
                {
                        m_savedIgnoreMap[profileID] = nick;
                }

                void NullOnlineServices::removeFromSavedIgnoreList(Int profileID)
                {
                        m_savedIgnoreMap.erase(profileID);
                }

                Bool NullOnlineServices::isSavedIgnored(Int profileID) const
                {
                        return (m_savedIgnoreMap.find(profileID) != m_savedIgnoreMap.end());
                }

                void NullOnlineServices::loadSavedIgnoreList(void)
                {
                        m_savedIgnoreMap.clear();
                        IgnorePreferences prefs;
                        prefs.load();
                        for (IgnorePreferences::Iterator it = prefs.begin(); it != prefs.end(); ++it)
                        {
                                m_savedIgnoreMap[it->first] = it->second;
                        }
                }

                void NullOnlineServices::addToIgnoreList(AsciiString nick)
                {
                        m_ignoreList.insert(nick);
                }

                void NullOnlineServices::removeFromIgnoreList(AsciiString nick)
                {
                        m_ignoreList.erase(nick);
                }

                Bool NullOnlineServices::isIgnored(AsciiString nick) const
                {
                        return (m_ignoreList.find(nick) != m_ignoreList.end());
                }

                void NullOnlineServices::setLocalIPs(UnsignedInt internalIP, UnsignedInt externalIP)
                {
                        m_internalIP = internalIP;
                        m_externalIP = externalIP;
                }

                Bool NullOnlineServices::isDisconnectedAfterGameStart(Int *reason) const
                {
                        if (reason)
                        {
                                *reason = m_disconnectReason;
                        }
                        return m_disconnectedAfterStart;
                }

                void NullOnlineServices::markAsDisconnectedAfterGameStart(Int reason)
                {
                        m_disconnectedAfterStart = TRUE;
                        m_disconnectReason = reason;
                }

                Bool NullOnlineServices::didPlayerPreorder(Int profileID) const
                {
                        return (m_preorderPlayers.find(profileID) != m_preorderPlayers.end());
                }

                void NullOnlineServices::markPlayerAsPreorder(Int profileID)
                {
                        m_preorderPlayers.insert(profileID);
                }

                void NullOnlineServices::readAdditionalDisconnects(void)
                {
                        m_additionalDisconnects = GetAdditionalDisconnectsFromUserFile(m_localProfileID);
                }

                void NullOnlineServices::updateAdditionalGameSpyDisconnections(Int count)
                {
                        m_additionalDisconnects = count;
                }

                // end anonymous namespace
        }

        static NullOnlineServices g_nullServices;
        static Interface *g_services = &g_nullServices;

        Color LobbyPalette[LOBBY_COLOR_MAX] =
        {
                GameMakeColor(255, 255, 255, 255),
                GameMakeColor(255, 255, 128, 255),
                GameMakeColor(200, 200, 200, 255),
                GameMakeColor(128, 255, 128, 255),
                GameMakeColor(255, 128, 128, 255),
                GameMakeColor(255, 64, 64, 255),
                GameMakeColor(255, 255, 255, 255),
                GameMakeColor(255, 255, 0, 255),
                GameMakeColor(128, 255, 255, 255),
                GameMakeColor(128, 255, 128, 255),
                GameMakeColor(160, 160, 160, 255),
                GameMakeColor(255, 255, 255, 255),
                GameMakeColor(255, 200, 200, 255),
                GameMakeColor(255, 200, 255, 255),
                GameMakeColor(255, 128, 255, 255),
                GameMakeColor(200, 200, 255, 255),
                GameMakeColor(200, 200, 255, 255),
                GameMakeColor(200, 200, 255, 255),
                GameMakeColor(200, 200, 255, 255),
                GameMakeColor(255, 255, 255, 255),
                GameMakeColor(128, 255, 128, 255),
                GameMakeColor(255, 128, 128, 255),
                GameMakeColor(255, 255, 128, 255),
                GameMakeColor(200, 200, 200, 255),
                GameMakeColor(255, 255, 0, 255),
                GameMakeColor(255, 255, 255, 255),
                GameMakeColor(255, 200, 0, 255)
        };

        Interface *GetServices()
        {
                        return g_services;
        }

        GameSpyStagingRoom *GetActiveGame()
        {
                        return g_services ? g_services->getCurrentStagingRoom() : NULL;
        }

        void Initialize(const char *motd, const char *config)
        {
                        g_services = &g_nullServices;
                        g_services->reset();
                        if (motd)
                        {
                                g_services->setMOTD(motd);
                        }
                        if (config)
                        {
                                g_services->setConfig(config);
                        }
        }

        void Shutdown()
        {
                        if (g_services)
                        {
                                g_services->reset();
                        }
                        g_services = &g_nullServices;
        }

        void ResetServices(Interface *replacement)
        {
                        g_services = replacement ? replacement : &g_nullServices;
        }

        bool AsciiComparator::operator()(AsciiString s1, AsciiString s2) const
        {
                        return stricmp(s1.str(), s2.str()) < 0;
        }

        Bool PlayerInfo::isIgnored(void) const
        {
                        return FALSE;
        }
}
