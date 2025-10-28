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
//                                                                            //
//  (c) 2001-2003 Electronic Arts Inc.                                        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// FILE: SfmlGameEngine.h /////////////////////////////////////////////////////////////////////////
// Author: OpenAI Assistant, 2025
// Description:
//   SFML-aware variant of the legacy Win32 game engine.  The implementation
//   relies on the message pump override installed by the SFML bootstrap rather
//   than querying Win32 window state directly.
///////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef __SFMLGAMEENGINE_H_
#define __SFMLGAMEENGINE_H_

#include "Common/GameEngine.h"

namespace sf
{
        class Event;
}

class SfmlGameEngine : public GameEngine
{
public:
        SfmlGameEngine();
        virtual ~SfmlGameEngine();

        virtual void execute(void);
        virtual void update(void);

protected:
        virtual GameLogic* createGameLogic(void);
        virtual GameClient* createGameClient(void);
        virtual ModuleFactory* createModuleFactory(void);
        virtual ThingFactory* createThingFactory(void);
        virtual FunctionLexicon* createFunctionLexicon(void);
        virtual LocalFileSystem* createLocalFileSystem(void);
        virtual ArchiveFileSystem* createArchiveFileSystem(void);
        virtual NetworkInterface* createNetwork(void);
        virtual Radar* createRadar(void);
        virtual WebBrowser* createWebBrowser(void);
        virtual AudioManager* createAudioManager(void);
        virtual ParticleSystemManager* createParticleSystemManager(void);

private:
        void handleEvent(const sf::Event& event);
};

GameEngine* CreateSfmlGameEngine();

#endif  // __SFMLGAMEENGINE_H_
