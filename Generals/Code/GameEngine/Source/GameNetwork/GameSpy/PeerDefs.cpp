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

#include "GameNetwork/GameSpy/PeerDefs.h"

namespace
{
        void SyncPalette()
        {
                for (Int i = 0; i < GSCOLOR_MAX; ++i)
                {
                        GameSpyColor[i] = OnlineServices::LobbyPalette[i];
                }
        }
}

GameSpyInfoInterface *TheGameSpyInfo = nullptr;
GameSpyStagingRoom *TheGameSpyGame = nullptr;
Color GameSpyColor[GSCOLOR_MAX];

void SetUpGameSpy(const char *motdBuffer, const char *configBuffer)
{
        OnlineServices::Initialize(motdBuffer, configBuffer);
        TheGameSpyInfo = OnlineServices::GetServices();
        TheGameSpyGame = OnlineServices::GetActiveGame();
        SyncPalette();
}

void TearDownGameSpy(void)
{
        OnlineServices::Shutdown();
        TheGameSpyInfo = OnlineServices::GetServices();
        TheGameSpyGame = OnlineServices::GetActiveGame();
        SyncPalette();
}
