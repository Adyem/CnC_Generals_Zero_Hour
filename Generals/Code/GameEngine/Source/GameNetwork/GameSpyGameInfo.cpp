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

#include "PreRTS.h"

#include "GameNetwork/GameSpyGameInfo.h"
#include "GameNetwork/GameSpy/PeerDefs.h"

GameSpyGameInfo::GameSpyGameInfo() = default;

void GameSpyGameInfo::gotGOACall(void)
{
        // GameSpy GOA callbacks are not implemented on this platform.
}

void GameSpyGameInfo::init(void)
{
        GameSpyStagingRoom::init();
}

void GameSpyGameInfo::resetAccepted(void)
{
        GameSpyStagingRoom::resetAccepted();
}

Int GameSpyGameInfo::getLocalSlotNum(void) const
{
        return GameSpyStagingRoom::getLocalSlotNum();
}

void GameSpyGameInfo::startGame(Int /*gameID*/)
{
        // NAT negotiation and GameSpy start logic is unavailable on this platform.
}

AsciiString GameSpyGameInfo::generateGameResultsPacket(void)
{
        return AsciiString::TheEmptyString;
}

void WOLDisplayGameOptions(void)
{
        // Legacy Westwood Online UI is not available.
}

void WOLDisplaySlotList(void)
{
        // Legacy Westwood Online UI is not available.
}

void GameSpyStartGame(void)
{
        // Multiplayer session start is not supported in this build.
}

void GameSpyLaunchGame(void)
{
        // Multiplayer session launch is not supported in this build.
}

Bool GetLocalChatConnectionAddress(AsciiString serverName, UnsignedShort serverPort, UnsignedInt &localIP)
{
        (void)serverName;
        (void)serverPort;
        (void)localIP;
        return FALSE;
}
