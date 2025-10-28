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

// Lightweight compatibility wrapper for the historic GameSpy game info API.


#ifndef __GAMESPYGAMEINFO_H__
#define __GAMESPYGAMEINFO_H__

#include "GameNetwork/GameSpy/StagingRoomGameInfo.h"

class GameSpyGameInfo : public GameSpyStagingRoom
{
public:
        GameSpyGameInfo();

        void gotGOACall(void);

        void init(void) override;
        void resetAccepted(void) override;
        Int getLocalSlotNum(void) const override;
        void startGame(Int gameID) override;

        AsciiString generateGameResultsPacket(void);
};

void WOLDisplayGameOptions(void);
void WOLDisplaySlotList(void);
void GameSpyStartGame(void);
void GameSpyLaunchGame(void);
Bool GetLocalChatConnectionAddress(AsciiString serverName, UnsignedShort serverPort, UnsignedInt &localIP);

#endif // __GAMESPYGAMEINFO_H__
