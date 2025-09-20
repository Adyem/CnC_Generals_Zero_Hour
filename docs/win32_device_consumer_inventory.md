# Win32 Device Consumer Inventory

This document captures the remaining places in the open-source Command & Conquer Generals code base that reference the legacy Win32 device layer. The inventory was assembled by searching for the primary classes implemented under `Generals/Code/GameEngineDevice/Source/Win32Device` and `Win32Device/Common` and then recording every consumer that still includes or instantiates them.

## Win32GameEngine

* **Current status** – After switching the bootstrap to SFML, no production code paths outside the Win32 device subtree include the Win32 game engine implementation. Zero Hour’s launcher now constructs `SfmlGameEngine` directly.【F:GeneralsMD/Code/Main/WinMain.cpp†L60-L78】【F:GeneralsMD/Code/Main/WinMain.cpp†L1419-L1430】
* **Implementation** – The original device implementation remains within the Win32 device source tree for reference.【F:Generals/Code/GameEngineDevice/Source/Win32Device/Common/Win32GameEngine.cpp†L33-L144】【F:Generals/Code/GameEngineDevice/Include/Win32Device/Common/Win32GameEngine.h†L25-L116】

## Win32OSDisplay

* **Legacy implementation** – The modal message-box wrapper continues to live in the Win32 device layer.【F:Generals/Code/GameEngineDevice/Source/Win32Device/Common/Win32OSDisplay.cpp†L25-L126】
* **Runtime usage** – Game audio initialisation prompts still call `OSDisplayWarningBox`, which has now been reimplemented for the SFML platform layer but historically routed through the Win32 code.【F:Generals/Code/GameEngine/Source/Common/Audio/GameAudio.cpp†L247-L299】

## Win32Mouse

* **Application globals** – The top-level application state tracks the active mouse instance through the Win32-derived type.【F:Generals/Code/Main/ApplicationGlobals.cpp†L19-L34】
* **Engine bootstrap** – The Windows entry point includes the Win32 mouse header so that the message pump can feed raw events into the mouse subsystem.【F:Generals/Code/Main/WinMain.cpp†L90-L101】
* **W3D client** – The default mouse factory and the W3D-specific cursor implementation both derive from `Win32Mouse`, keeping the legacy behaviour in place for the renderer.【F:Generals/Code/GameEngineDevice/Include/W3DDevice/GameClient/W3DGameClient.h†L53-L136】【F:Generals/Code/GameEngineDevice/Include/W3DDevice/GameClient/W3DMouse.h†L54-L108】
* **Tools** – Utilities such as WorldBuilder depend on the Win32 mouse implementation for their editor-specific input handling.【F:Generals/Code/Tools/WorldBuilder/src/WorldBuilder.cpp†L77-L108】

## Win32DIMouse

* **Inclusion** – The DirectInput flavoured mouse header is still included by the W3D game client, although the runtime now overrides mouse creation with the SFML-backed adapter.【F:Generals/Code/GameEngineDevice/Include/W3DDevice/GameClient/W3DGameClient.h†L53-L135】
* **Implementation** – The DirectInput wrapper itself continues to live in the Win32 device subtree.【F:Generals/Code/GameEngineDevice/Source/Win32Device/GameClient/Win32DIMouse.cpp†L19-L194】

These references document the full set of touch points that need to be considered when removing or refactoring the remaining Win32-specific code paths.

