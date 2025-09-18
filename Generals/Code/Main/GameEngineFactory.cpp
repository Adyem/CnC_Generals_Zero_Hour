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

#include "WinMain.h"
#include "Common/GameEngine.h"
#include "Common/GameMemory.h"
#include "Win32Device/Common/Win32GameEngine.h"

Bool gInitialEngineActiveState = true;

namespace {

GameEngineFactoryFunction g_gameEngineFactoryOverride = NULL;

}

void SetGameEngineFactoryOverride(GameEngineFactoryFunction factory)
{
        g_gameEngineFactoryOverride = factory;
}

GameEngineFactoryFunction GetGameEngineFactoryOverride()
{
        return g_gameEngineFactoryOverride;
}

GameEngine* CreateGameEngine(void)
{
        GameEngineFactoryFunction factory = GetGameEngineFactoryOverride();
        GameEngine* engine = NULL;

        if (factory != NULL)
        {
                engine = factory();
        }

        if (engine == NULL)
        {
                engine = NEW Win32GameEngine;
        }

        engine->setIsActive(gInitialEngineActiveState);
        return engine;
}
