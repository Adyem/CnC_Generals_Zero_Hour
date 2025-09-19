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

// FILE: SfmlGameEngine.cpp ////////////////////////////////////////////////////
// Author: OpenAI Assistant, 2025
// Description:
//   Implementation of the SFML-aware game engine device layer.
////////////////////////////////////////////////////////////////////////////////

#include "SfmlDevice/Common/SfmlGameEngine.h"

#include "Common/Debug.h"
#include "Common/GameAudio.h"
#include "Common/GameEngine.h"
#include "Common/GlobalData.h"
#include "Common/INIException.h"
#include "Common/Recorder.h"
#include "GameClient/Keyboard.h"
#include "GameLogic/GameLogic.h"
#include "GameNetwork/LANAPICallbacks.h"
#include "GameNetwork/NetworkInterface.h"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Window/Event.hpp"
#include "SFMLPlatform/SfmlAudioManager.h"
#include "SFMLPlatform/SfmlMouseBridge.h"
#include "SFMLPlatform/WindowSystem.h"
#include "SfmlDevice/Common/SfmlBIGFileSystem.h"
#include "SfmlDevice/Common/SfmlLocalFileSystem.h"
#include "W3DDevice/Common/W3DFunctionLexicon.h"
#include "W3DDevice/Common/W3DModuleFactory.h"
#include "W3DDevice/Common/W3DRadar.h"
#include "W3DDevice/Common/W3DThingFactory.h"
#include "W3DDevice/GameClient/W3DGameClient.h"
#include "W3DDevice/GameClient/W3DParticleSys.h"
#include "W3DDevice/GameLogic/W3DGameLogic.h"
#include "W3DDevice/GameClient/RenderBackend.h"
#if defined(_WIN32)
#include "W3DDevice/GameClient/W3DWebBrowser.h"
#include <atlbase.h>
#endif

#include <chrono>
#include <thread>

namespace
{

using Clock = std::chrono::steady_clock;

} // anonymous namespace

SfmlGameEngine::SfmlGameEngine() = default;
SfmlGameEngine::~SfmlGameEngine() = default;

void SfmlGameEngine::execute(void)
{
        sfml_platform::WindowSystem* windowSystem = sfml_platform::GetActiveWindowSystem();
        if (windowSystem == NULL)
        {
                GameEngine::execute();
                return;
        }

        const auto startTime = Clock::now();

        windowSystem->run(
                [this, startTime](sf::RenderWindow& window, float /*delta*/)
                {
                        if (getQuitting())
                        {
                                window.close();
                                return;
                        }

#if defined(_DEBUG) || defined(_INTERNAL)
                        if (TheGlobalData->m_benchmarkTimer > 0)
                        {
                                const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(Clock::now() - startTime).count();
                                if (TheGlobalData->m_benchmarkTimer < elapsed)
                                {
                                        if (TheGameLogic->isInGame())
                                        {
                                                if (TheRecorder->getMode() == RECORDERMODETYPE_RECORD)
                                                {
                                                        TheRecorder->stopRecording();
                                                }
                                                TheGameLogic->clearGameData();
                                        }
                                        setQuitting(TRUE);
                                }
                        }
#endif

                        try
                        {
                                GameEngine::update();
                        }
                        catch (INIException e)
                        {
                                if (e.mFailureMessage)
                                {
                                        RELEASE_CRASH((e.mFailureMessage));
                                }
                                else
                                {
                                        RELEASE_CRASH(("Uncaught Exception in GameEngine::update"));
                                }
                        }
                        catch (...)
                        {
                                try
                                {
                                        if (TheRecorder && TheRecorder->getMode() == RECORDERMODETYPE_RECORD && TheRecorder->isMultiplayer())
                                        {
                                                TheRecorder->cleanUpReplayFile();
                                        }
                                }
                                catch (...)
                                {
                                }
                                RELEASE_CRASH(("Uncaught Exception in GameEngine::update"));
                        }

                        if (getQuitting())
                        {
                                window.close();
                        }

                        if (auto* mouse = sfml_platform::GetActiveMouseBridge())
                        {
                                mouse->refreshCursor();
                        }
                },
                nullptr,
                [this](const sf::Event& event)
                {
                        handleEvent(event);
                });
}

void SfmlGameEngine::update(void)
{
        GameEngine::update();

        if (!isActive())
        {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));

                if (TheLAN != NULL)
                {
                        TheLAN->setIsActive(isActive());
                        TheLAN->update();
                }
        }
}

GameLogic* SfmlGameEngine::createGameLogic(void)
{
        return NEW W3DGameLogic;
}

GameClient* SfmlGameEngine::createGameClient(void)
{
        return NEW W3DGameClient;
}

ModuleFactory* SfmlGameEngine::createModuleFactory(void)
{
        return NEW W3DModuleFactory;
}

ThingFactory* SfmlGameEngine::createThingFactory(void)
{
        return NEW W3DThingFactory;
}

FunctionLexicon* SfmlGameEngine::createFunctionLexicon(void)
{
        return NEW W3DFunctionLexicon;
}

LocalFileSystem* SfmlGameEngine::createLocalFileSystem(void)
{
        return NEW SfmlLocalFileSystem;
}

ArchiveFileSystem* SfmlGameEngine::createArchiveFileSystem(void)
{
        return NEW SfmlBIGFileSystem;
}

NetworkInterface* SfmlGameEngine::createNetwork(void)
{
        return NetworkInterface::createNetwork();
}

Radar* SfmlGameEngine::createRadar(void)
{
        return NEW W3DRadar;
}

WebBrowser* SfmlGameEngine::createWebBrowser(void)
{
#if defined(_WIN32)
        return NEW CComObject<W3DWebBrowser>;
#else
        return NULL;
#endif
}

AudioManager* SfmlGameEngine::createAudioManager(void)
{
        AudioManagerFactoryFunction factory = GetAudioManagerFactoryOverride();
        if (factory != NULL)
        {
                return factory();
        }
        return NEW SfmlAudioManager;
}

ParticleSystemManager* SfmlGameEngine::createParticleSystemManager(void)
{
        return NEW W3DParticleSystemManager;
}

void SfmlGameEngine::handleEvent(const sf::Event& event)
{
        switch (event.type)
        {
                case sf::Event::Closed:
                        checkAbnormalQuitting();
                        reset();
                        setQuitting(TRUE);
                        break;

                case sf::Event::LostFocus:
                        if (TheKeyboard)
                        {
                                TheKeyboard->resetKeys();
                        }
                        if (auto* mouse = sfml_platform::GetActiveMouseBridge())
                        {
                                mouse->lostFocus(TRUE);
                        }
                        setIsActive(FALSE);
#if defined(_WIN32)
                        GetRenderBackend().HandleFocusChange(false);
#endif
                        if (TheAudio)
                        {
                                TheAudio->loseFocus();
                        }
                        break;

                case sf::Event::GainedFocus:
                        if (TheKeyboard)
                        {
                                TheKeyboard->resetKeys();
                        }
                        if (auto* mouse = sfml_platform::GetActiveMouseBridge())
                        {
                                mouse->lostFocus(FALSE);
                                mouse->refreshCursor();
                        }
                        setIsActive(TRUE);
#if defined(_WIN32)
                        GetRenderBackend().HandleFocusChange(true);
#endif
                        if (TheAudio)
                        {
                                TheAudio->regainFocus();
                        }
                        break;

                case sf::Event::MouseEntered:
                        if (auto* mouse = sfml_platform::GetActiveMouseBridge())
                        {
                                mouse->refreshCursor();
                        }
                        break;

                default:
                        break;
        }
}

GameEngine* CreateSfmlGameEngine()
{
        return NEW SfmlGameEngine;
}
