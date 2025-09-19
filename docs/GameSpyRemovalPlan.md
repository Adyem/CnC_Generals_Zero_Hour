# GameSpy Removal Plan

The long-term goal is to replace or retire the legacy GameSpy networking layer.
This document captures the incremental work that still needs to happen after the
first round of include cleanup.

## Completed in this step

* Removed direct `GameSpy/PeerDefs.h` dependencies from common persistence
  helpers (`UserPreferences.cpp` and `Recorder.cpp`) so they no longer pull in
  the GameSpy SDK for basic single-player features.
* Introduced a neutral `OnlineServices` facade that replaces the old GameSpy
  data models. All existing call sites now resolve through the compatibility
  wrapper in `PeerDefs.h`, letting the shell operate in an offline-only mode
  without including any GameSpy SDK headers.
* Greyed out the Multiplayer and Network entries on the main menu so players
  immediately see that online features are offline while the replacement
  service is under construction.
* Replaced the peer, buddy, persistent storage, and game-results background
  workers with neutral stubs so the shell no longer spins up GameSpy threads
  or calls into the legacy SDK during startup.
* Stubbed every WOL shell screen (lobby, staging, overlays, and patch
  downloads) to run through the `OnlineServices` facade, keeping the menus
  visible while all callbacks short-circuit to an offline notice.
* Simplified the GameSpy overlay helpers to show the same offline message and
  removed the last dependencies on buddy reconnect logic or patch downloaders.
* Stripped the GameSpy include paths and library references from the Visual
  Studio project files so Windows builds only link the neutral stubs.
* Updated the README to note that the legacy SDK is no longer required while
  the new backend is developed.

## Follow-up tasks

1. **Implement the replacement service.**
   * Finalise the new online backend API and connect the `OnlineServices`
     facade to it so menus can host and join sessions again.
   * Wire modder configuration (endpoints, authentication, etc.) into the
     facade once the service is ready.

2. **Remove unused GameSpy sources.**
   * Once the new backend is in place, prune the dormant GameSpy source files
     and headers from the tree to keep future maintenance simple.

Tracking these steps as separate change lists will let us continue trimming the
GameSpy surface without breaking unrelated single-player systems.
