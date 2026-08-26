# Design: Restructuring Generals / Zero Hour Around FullLibft

Status: discovery proposal  
Date: 2026-08-25  
Scope inspected: this repository, with implementation detail sampled primarily from `GeneralsMD` (Zero Hour)  
Libft baseline inspected: `LibftLegends/Libft` `main` at `d7163331ccb3181153111f01f6582b134be2a162`

## 1. Executive decision

FullLibft should become the **replacement engine core** for Generals and Zero Hour. SAGE is the behavioral reference and migration source, not the permanent engine underneath the game.

The intended final system is a Libft-native RTS engine in which:

- Libft `Game` owns the top-level game state/world lifecycle and is extended with the deterministic entity, component/module, command, replay, and fixed-tick facilities required by Generals.
- Libft `GPGR` becomes the renderer/device foundation and is expanded from its current window-and-shader scope into a production RTS renderer.
- Libft supplies memory, types, containers, files, configuration, logging, time, jobs, networking, compression, scripting, persistence, math, and geometry.
- Generals-specific rules, units, factions, AI policy, UI layouts, assets, campaigns, and balance remain the game layer.
- SAGE classes survive only as temporary migration adapters or isolated file-format compatibility readers; they are removed from the runtime dependency graph when parity is reached.

The target dependency direction should be:

```text
Generals / Zero Hour rules, content, UI, campaigns
                         |
Libft-native RTS domain layer (entities, modules, commands, deterministic tick)
                         |
Libft Game + GPGR + Networking + Storage + runtime modules
                         |
Libft platform/graphics/audio backends
```

This is a staged engine replacement rather than an in-place library swap. The current Zero Hour engine/device/main tree contains about 1,332 C/C++ headers and sources and about 504,000 lines. Several SAGE contracts are pervasive: `AsciiString` appears in about 598 files, `Xfer` in 405, `UnicodeString` in 222, memory-pool symbols in 168, and random-value symbols in 142. They should be replaced, but in vertical slices with dual-run and compatibility tests rather than one repository-wide mechanical edit.

The recommended strategy is therefore:

1. Establish a modern, reproducible build and a behavior baseline.
2. Vendor and pin Libft and define which missing RTS/renderer capabilities are added upstream to Libft.
3. Introduce Libft-native identifiers, fixed-width values, errors, ownership, time, files, jobs, and configuration.
4. Rebuild the simulation around extensions to `game_state`, `game_world`, registries, events, hooks, replay, and scripting while comparing every tick against SAGE.
5. Expand GPGR into the full rendering and platform-device layer, initially preserving W3D/BIG/INI content through compatibility importers.
6. Replace SAGE serialization, networking, frontend, and tool backends, with explicit legacy import paths where desired.
7. Delete SAGE runtime subsystems only after their Libft replacements meet behavioral parity gates.

## 2. What was inspected

### 2.1 Current repository

The repository contains two related source trees:

- `Generals/`: base Command & Conquer: Generals.
- `GeneralsMD/`: Zero Hour (the later and more complete product tree).

Each is a legacy Visual C++ 6 workspace centered on `Code/RTS.dsw` and `.dsp` projects. Zero Hour is divided into:

- `Code/Main`: Win32 entry point and application resources.
- `Code/GameEngine`: platform-neutral-looking interfaces and most gameplay, client, common, and networking code.
- `Code/GameEngineDevice`: concrete Win32/DirectX/device implementations.
- `Code/Libraries`: STLport, Westwood libraries, compression, audio, profiling, and missing proprietary SDK surfaces.
- `Code/Tools`: WorldBuilder, asset tools, launchers, exporters, and other utilities.

The root README requires or names legacy/missing dependencies: DirectX 9, STLport 4.5.3, 3DS Max 4 SDK, NVASM, BYTEmark, Miles, Bink, SafeDisk, GameSpy, zlib 1.1.4, and LZH-Light. Libft can remove or reduce only part of this list. It does not itself solve the proprietary media SDKs, D3D renderer, editor SDK, or all legacy online-service behavior.

Architecturally important current contracts include:

- `GameEngine` and `SubsystemInterface`: explicit `init`, `reset`, `update`, ordered initialization, and global singleton access.
- `GameEngineDevice`: factories for device-specific filesystem, client, renderer/input/audio-related implementations.
- `AsciiString` / `UnicodeString`: copy-on-write engine strings used in public data structures and APIs.
- `MemoryPool`: allocation macros and pooled object behavior embedded throughout gameplay code.
- `Snapshot` / `Xfer`: serialization, save/load, CRC, and post-load processing.
- `RandomValue`: separate logic/client/audio randomness with deterministic implications.
- `FileSystem`, `LocalFileSystem`, `ArchiveFileSystem`, and `ArchiveFile`: loose and packed asset access.
- `GameNetwork`: lockstep frame transport, commands, LAN, file transfer, NAT, and obsolete GameSpy integrations.
- WW3D/WWMath/WWSaveLoad and device code: rendering, asset, and legacy format contracts.

The current design also relies heavily on process-wide globals (`TheGameEngine`, `TheGameLogic`, `TheGameClient`, `TheFileSystem`, and many others). `SubsystemInterfaceList` gives them lifecycle ordering but not true dependency ownership.

### 2.2 FullLibft

The Libft root README describes one canonical GNU Make graph that builds module archives and `Full_Libft.a`, with debug, tests, sanitizer checks, archive integrity tests, and performance benchmarks. It also supplies CMake, Meson, and Bazel quick-start templates. Its documented architecture contains 40+ modules and roughly 611 source files plus 251 headers in its generated dependency graph.

The most relevant documented capabilities are:

- `Basic`, `Advanced`, `CPP_class`, `Template`, `Buffer`: primitive functions, strings/ownership wrappers, containers, callables, and buffers.
- `CMA`, `SCMA`: tracked/limited custom allocation and shared allocation.
- `Errno`, `Debug`, `Sink`, `Logger`, `Observability`: common error reporting, diagnostics, stack traces, logs, metrics, and tracing.
- `Compatebility`, `System_utils`, `File`, `Filesystem`: Windows/POSIX shims, descriptors, paths, safe joins, atomic writes, directory operations, file watches, and system information.
- `Time`: wall/monotonic/high-resolution time, benchmarks, trace events, timers, countdowns, and FPS pacing.
- `PThread`, `Threading`: low-level primitives, RAII guards, cancellation, queues, thread pools, and scheduled tasks.
- `CLI`, `Config`, `Parser`, `JSon`, `XML`, `YAML`, `CSV`: typed configuration and structured-data parsing.
- `RNG`: random-number facilities.
- `Compression`: compression adapters and streaming helpers.
- `Networking`, `API`: sockets/protocol/service plumbing, encryption integration, async helpers, and observability.
- `Storage`: key/value backends, snapshots, TTL, batching, metrics, replication callbacks, and optional encryption.
- `Math`, `Geometry`: scalar/vector/matrix/quaternion math and intersection/culling queries.
- `Game`: general game-domain records, world/event scheduling, behavior trees, replay/persistence helpers, Lua hooks, pathfinding, and telemetry.
- `GPGR`: a small OpenGL demo/window/shader path.
- `Voxel`: voxel terrain and meshing.

Libft documents Windows compatibility helpers, but its canonical build and exact compiler/runtime support must still be proven against this project. Its module graph is dense: using one high-level module often pulls in much of the library.

## 3. Goals and non-goals

### Goals

- Replace the SAGE runtime with a Libft-native engine rather than permanently wrapping SAGE services.
- Make Libft `Game` the owner of world/state/event/replay/script lifecycle, extended for a deterministic RTS.
- Make an expanded Libft `GPGR` the renderer and window/input presentation foundation.
- Move game runtime code to Libft conventions: fixed-width integers, `ft_size_t`, `ft_bool`, explicit lifecycle, Libft ownership/containers where suitable, consistent thread-safety contracts, and `FT_ERR_*` return codes for functions that can fail.
- Retain shipped gameplay behavior and chosen content/save compatibility while migration is in progress.
- Produce a modern 32-bit Windows build first, then make portability and 64-bit work tractable.
- Separate simulation, presentation, device backends, tools, and third-party libraries into explicit targets.
- Remove STLport and obsolete middleware where a tested replacement exists.
- Enable headless deterministic tests and replay/desync validation.
- Allow Generals and Zero Hour to become games on one reusable Libft engine foundation instead of sustaining two manually duplicated SAGE cores.

### Non-goals and sequencing constraints

- The replacement must not reinterpret Generals as the RPG/voxel-shaped portions of Libft `Game`. Libft must gain reusable RTS primitives; Generals rules must not be distorted to fit unrelated character/inventory/crafting abstractions.
- GPGR's current window/shader API is a starting point, not evidence that rendering is already solved.
- Gameplay behavior is not intentionally changed while the backend is replaced. Simulation tick, ordering, RNG consumption, and numeric rules remain reference-compatible until an explicitly versioned gameplay change is approved.
- Legacy save, replay, packet, INI, BIG, W3D, audio, texture, and map formats may remain as import formats even after their SAGE runtime implementations disappear.
- Win64/cross-platform is a target of the type and backend redesign, but the first comparison build remains Win32/x86 so backend replacement can be separated from width-induced behavior changes.
- Missing Miles/Bink/3DS Max functionality requires explicit Libft backends, open replacements, conversion tools, or retirement decisions. SafeDisk and GameSpy are deliberately removed; they are not early blockers and receive no compatibility implementation.

## 4. Proposed target repository layout

Keep the original trees available as historical/reference inputs, but build a new shared layout from Zero Hour first:

```text
/
  CMakeLists.txt
  CMakePresets.json
  cmake/
  external/
    libft/                 # pinned git submodule or vendored source
    zlib/
  engine/
    runtime/               # interfaces and Libft adapters
      include/EngineRuntime/
      src/libft/
      src/win32/
    core/                  # subsystem lifecycle, names, common data
    simulation/            # deterministic GameLogic and modules
    client/                # GameClient, UI, particles, camera
    network/               # lockstep protocol and transports
    serialization/         # Snapshot/Xfer and version adapters
    assets/                # FileSystem/archive abstraction, INI/content
    device/
      d3d8_or_d3d9/
      win32/
      audio/
  games/
    generals/
    zero_hour/
  tools/
  tests/
    unit/
    integration/
    determinism/
    fixtures/
  docs/
    adr/
```

Do not physically move hundreds of files in the first change. Create the target graph with source lists pointing at existing paths, establish passing gates, and then move cohesive directories in mechanical follow-up changes. This preserves `git blame` and keeps build failures attributable.

## 5. Libft-native engine boundaries

The final engine is allowed and expected to use Libft public types directly. It still should not include `FullLibft.hpp` everywhere: each engine subsystem should include only the Libft modules it owns. This preserves compile-time boundaries and prevents the dense module graph from turning every change into a full rebuild.

Keep focused backend interfaces such as:

```cpp
namespace engine {
class Clock;
class Logger;
class Allocator;
class FileService;
class PathService;
class JobSystem;
class NetworkTransport;
class Metrics;
class Runtime;
}
```

Rules:

- New engine code uses fixed-width integer types, `ft_size_t`, and `ft_bool` for data; it separately uses Libft lifecycle conventions and returns documented `FT_ERR_*` values from fallible functions.
- Temporary adapters translate SAGE `Bool`/`Int`/strings/errors to Libft types at migration boundaries.
- Libft containers and strings may appear in new internal APIs, but serialized formats and packets always use explicit schemas rather than raw object memory.
- Legacy plugins/readers remain behind C/POD or opaque interfaces until replaced.
- Ownership is explicit: borrowed views, caller-provided buffers, `unique_ptr`-style ownership, or an allocator parameter.
- The runtime is created in `GameMain`, passed to the engine root, and temporarily published to legacy code through one compatibility accessor.
- Each service can temporarily have `Libft*`, `SageReference*`, and `Fake*` implementations to support gradual rollout, dual execution, and deterministic tests. `SageReference*` implementations are deleted after parity.

The boundary is not intended to hide Libft forever. It separates stable engine responsibilities—renderer, audio, files, transport—from their backend implementations while the engine's common vocabulary deliberately becomes Libft-native.

## 6. Libft-to-engine fit matrix

| Libft area | Current SAGE area | Decision | How it is useful / required safeguards |
|---|---|---|---|
| `Basic`, `Advanced` | base types, C runtime helpers, `AsciiString` utilities | Adopt as engine vocabulary | New APIs use `<cstdint>` widths as Libft does, `ft_size_t`, `ft_bool`, bounds-checked helpers, and Libft strings. Migrate old strings/types by subsystem with checked conversions and format-version adapters. |
| `CPP_class`, `Template`, `Buffer` | STLport and hand-written containers/buffers | Evaluate, then selective adopt | First remove STLport in favor of current standard library. Benchmark and review Libft containers before choosing them for hot paths. Never alter serialized iteration order implicitly. |
| `CMA` | `MemoryPool`, `new/delete`, pool macros | Adopt as backend, not API replacement | Route engine heap categories and diagnostic allocation through an adapter. Preserve fixed pools and allocation macros until object layout, alignment, lifetime, and failure behavior have parity tests. Avoid `SCMA` unless cross-process memory is a real requirement. |
| `Errno` | exceptions, `DEBUG_ASSERTCRASH`, return values | Adopt as the engine error convention | New APIs return documented `FT_ERR_*` codes and lifecycle objects expose local error accessors. Use the global stack for diagnostic context, not hidden simulation branching; define threading and clearing rules. |
| `Debug`, `Sink`, `Logger`, `Observability` | `Debug.h`, `MiniLog`, perf timers, ad-hoc logs | Strong early candidate | Supply structured categories, sinks, stack traces, counters, and trace sessions. Keep legacy macros as wrappers so call sites migrate without a flag day. Logging must not affect deterministic state. |
| `Compatebility`, `System_utils` | direct Win32 calls throughout Main/device/common | Strong foundation | Centralize environment, descriptors, CPU/memory, locale, stack traces, and portable OS shims. Device/window/message-pump calls remain in a Win32 backend. |
| `File`, `Filesystem` | `FileSystem`, `LocalFileSystem` | Strong candidate behind existing interfaces | Implement loose-file access, path normalization, safe joins, atomic preferences/saves, directory walking, and tools I/O. Keep BIG/archive lookup precedence and case behavior in engine asset code. |
| `Time` | Win32 timers, `PerfTimer`, frame limiter | Strong early candidate | Use monotonic/high-resolution time, benchmarks, tracing, and timers. Do not use generic `time_fps` to drive simulation until frame pacing and fixed-step behavior are verified separately. |
| `PThread`, `Threading` | GameSpy worker classes and manual Win32 synchronization | Strong candidate for non-simulation work | Use RAII locks, cancellation, queues, pools, and scheduler for loading, logging, decompression, patch/download work. Deterministic simulation remains single-threaded initially; task completion order must not affect game state. |
| `CLI` | `CommandLine.cpp` | Strong candidate | Parse typed flags into a new startup configuration, then populate legacy `GlobalData` during transition. Preserve legacy spellings and precedence. |
| `Config` | INI plus global data/user preferences | Complement, do not replace initially | Use for engine bootstrap, developer configuration, backend selection, and validation. SAGE INI parsing is content semantics and must remain until golden parse tests exist. |
| `Parser`, `JSon`, `XML`, `YAML`, `CSV` | INI, save metadata, tool formats | Additive | Use JSON/YAML for new manifests, tests, metrics, and tool pipelines. Do not mass-convert shipped INIs or binary saves. XML may help tooling, not core simulation. |
| `RNG` | logic/client/audio `RandomValue` | Only through compatibility adapter | The logic RNG's algorithm, seed, number of draws, range mapping, overflow, and state serialization must be byte-for-byte reproducible. Libft RNG can serve tools/client effects or a future protocol version. |
| `Compression` | zlib/LZH and archive/save/network compression | Candidate after corpus tests | Build adapters and compare decompression against real `.big`, save, replay, and packet fixtures. Algorithm/bitstream compatibility matters more than API similarity. |
| `Networking` | UDP transport, lockstep connection/frame data, GameSpy | Defer until offline parity, then replace | GameSpy is removed in Phase 0 and multiplayer remains disabled. Later, build a new Libft-native command/session protocol with observability, compression, and tests; use old lockstep code only as behavioral reference, not a shipped dependency. |
| `Storage` | preferences, caches, save metadata | Useful outside authoritative state | Good for launcher/tool caches, settings, metadata, and content indexes. Do not put live lockstep state in a generic KV store. |
| `Math`, `Geometry` | WWMath and gameplay geometry | Selective, late | Useful for new portable code and tests. Do not replace existing vectors/matrices/collision math until layouts, handedness, precision, epsilon, NaN, and compiler FP behavior are proven. Renderer-facing WWMath remains initially. |
| `Game` | `GameLogic`, objects/modules, AI, events, saves | Make it the simulation foundation and extend it | Use `game_state`, `game_world`, registry, event scheduler, hooks, replay, Lua bridge, data catalog, behavior trees, pathfinding, and telemetry. Add deterministic entity/module storage, fixed ticks, player/team/diplomacy, commands, selection/control groups, fog/shroud, production, combat, locomotion, RTS AI integration, and versioned snapshots. Do not force Generals units into RPG-only classes. |
| `GPGR` | WW3D/DirectX device layer | Expand into the production renderer | Retain `ft_gpu_window` and `ft_gpu_shader`, then add device/capability management, resources, render graph/passes, W3D mesh/animation/material support, terrain, particles, shadows, water, UI/text, picking, screenshots, and performance tooling. Use a backend-neutral GPGR API even if OpenGL is first. |
| `Voxel` | height-map terrain/W3D maps | Out of scope | Generals terrain is not a voxel world. This module offers no direct migration advantage. |
| `Lua` via `Game` | ScriptEngine | Future additive option | A sandboxed Lua 5.4 bridge may be useful for new mods, but changing existing script semantics would break content. Run side-by-side with an explicit new content version. |
| `Encryption`, `Encoding`, `URI`, `API` | login/patch/web/online services | Later service work | Useful for a replacement master server, launcher, secure update transport, and URL handling. They are not required in the deterministic engine core. |
| `Application`, `HTML`, `ReadLine`, `Regex`, `GetNextLine`, `DUMB` | tools/services | Optional | Pull these only for concrete tool or service requirements. `DUMB` is explicitly legacy/experimental and must not become an engine dependency. |

## 7. Libft `Game` expansion for a deterministic RTS

The existing module already offers useful orchestration: `game_state`, `game_world`, `game_world_registry`, `game_event_scheduler`, replay sessions, hooks, scripting, catalogs, behavior trees, pathfinding, server integration, persistence helpers, and telemetry. These should be the foundation, but the following reusable capabilities must be added to Libft rather than left as permanent Generals-only copies of SAGE.

### 7.0 Ownership boundary: Libft hosts; the game defines

Libft must not contain Generals or Zero Hour factions, sciences, generals, special powers, unit templates, command sets, asset names, balance numbers, or exact gameplay rules. Those belong to this repository's `games/generals` and `games/zero_hour` targets.

The correct relationship is:

```text
Libft Game
    ├── lifecycle and game/world ownership
    ├── generic typed catalog and extension registration
    ├── entity/component/module storage
    ├── deterministic tick, command, event, replay, and snapshot interfaces
    ├── generic player/team/relationship primitives
    ├── validation, lookup, serialization, hooks, and scripting interfaces
    └── no knowledge of Generals content
                    ▲
                    │ implements/registers through public interfaces
                    │
Generals / Zero Hour game module
    ├── owns INI files and imported game data
    ├── faction_definition
    ├── science_definition
    ├── general_definition
    ├── special_power_definition
    ├── unit/building/weapon/upgrade definitions
    ├── W3D/texture/audio/UI asset references
    └── exact gameplay systems and rules
```

Libft owns the shelves, IDs, lookup rules, lifecycle, and extension points. The game owns every book placed on those shelves and the code deciding what the book means.

### 7.0.1 Assessment of current Libft classes

| Current class | Useful now | Why it cannot directly own Generals data | Required Libft change |
|---|---|---|---|
| `game_data_catalog` | Lifecycle, locking, `ft_map`-backed registration/fetch pattern | It is compiled specifically around `game_item_definition`, recipes, and loadouts with `int32_t` IDs | Add a generic typed catalog/registry; keep the RPG convenience catalog as one consumer or compatibility layer. |
| `game_state` | Top-level lifecycle, worlds, variables, hooks | It directly owns `game_character` collections and dispatches item-crafted/character-damaged callbacks | Give it generic service/extension slots and entity/world ownership; move character conveniences into an optional RPG extension. |
| `game_world` | Event scheduler, replay, registry ownership, persistence orchestration | It directly embeds economy, crafting, dialogue, quest, vendor, and upgrade instances | Replace mandatory domain members with registered world services/extensions. Optional RPG services can be installed by games that want them. |
| `game_world_registry` | Register/fetch pattern and lifecycle | It only stores region definitions and world regions | Introduce a generic entity/definition/service registry; retain regions as a typed facility layered on it. |
| `game_hooks` | Priorities, named listeners, metadata, callable storage | `ft_game_hook_context` contains fixed `game_character`, `game_item`, and `game_event` pointers plus two ad-hoc payload integers | Add typed event channels or a type-erased payload descriptor with stable IDs, size/alignment, serializer, and validator. Keep legacy RPG hook adapters separately. |
| `game_script_bridge` | Sandboxed Lua, callback registration, limits, `void *user_data` escape hatch | Generic access currently depends mainly on untyped `user_data`; exposing raw game pointers would be unsafe and brittle | Add a registered script-service table with named, validated functions and opaque handles. Generals supplies bindings without Libft including Generals headers. |
| `game_upgrade` | Lifecycle and level concept | Four unnamed integer modifiers cannot describe Generals sciences/upgrades safely | Treat it as a simple RPG value type, not the base for sciences. Generals owns `science_definition`; a generic modifier/effect interface may be added separately. |
| `game_behavior_profile` | Behavior actions/profile pattern | Hard-coded aggression/caution doubles and action format may not match Generals AI | Allow games to register policy data and behavior nodes through generic interfaces; Generals owns its AI profiles and deterministic numeric representation. |

The game should not subclass these concrete RPG classes merely to smuggle in unrelated fields. Composition through generic interfaces is preferable: it avoids empty base members, unsafe downcasts, and Libft changes whenever a Generals definition gains a property.

### 7.0.2 Generic catalog interface required in Libft

Add a registry whose infrastructure is generic while definition storage remains game-owned and type-safe. One possible public shape is:

```cpp
struct game_definition_type
{
    uint64_t value; // stable type ID, collision-checked at registration
};

struct game_definition_id
{
    uint64_t value; // stable content ID within a definition type
};

struct game_definition_descriptor
{
    game_definition_type type;
    ft_string name;
    ft_size_t object_size;
    ft_size_t object_alignment;
    int32_t (*validate)(const void *definition, game_validation_report &report);
    int32_t (*serialize)(const void *definition, game_document_sink &sink);
    int32_t (*destroy)(void *definition);
};

class game_definition_registry
{
public:
    int32_t register_type(const game_definition_descriptor &descriptor);
    int32_t register_definition(game_definition_type type,
                                game_definition_id id,
                                void *owned_definition);
    int32_t find_definition(game_definition_type type,
                            game_definition_id id,
                            const void **definition_out) const;
};
```

A templated wrapper should provide type safety so normal game code does not manipulate `void *`:

```cpp
template <typename Definition>
class game_typed_catalog_view
{
public:
    int32_t register_definition(game_definition_id id,
                                ft_sharedptr<const Definition> definition);
    int32_t find(game_definition_id id,
                 ft_sharedptr<const Definition> &definition_out) const;
};
```

The type-erased layer exists only at the plugin/registry boundary. Inside the Zero Hour module, code remains strongly typed.

### 7.0.3 Game-owned definitions

Definitions live under the game target, for example:

```text
games/zero_hour/
  include/ZeroHour/Data/FactionDefinition.hpp
  include/ZeroHour/Data/ScienceDefinition.hpp
  include/ZeroHour/Data/GeneralDefinition.hpp
  include/ZeroHour/Data/SpecialPowerDefinition.hpp
  include/ZeroHour/Data/UnitDefinition.hpp
  include/ZeroHour/Data/WeaponDefinition.hpp
  include/ZeroHour/Rules/ScienceSystem.hpp
  include/ZeroHour/Rules/SpecialPowerSystem.hpp
  src/data/LegacyIniImporter.cpp
  src/data/ZeroHourCatalog.cpp
  src/rules/
```

Example game-owned data:

```cpp
namespace zero_hour
{
struct science_definition
{
    game_definition_id id;
    ft_string internal_name;
    uint32_t purchase_cost;
    uint32_t required_rank;
    ft_vector<game_definition_id> prerequisites;
    ft_vector<game_definition_id> granted_special_powers;
};

struct faction_definition
{
    game_definition_id id;
    ft_string internal_name;
    game_definition_id default_player_template;
    ft_vector<game_definition_id> available_sciences;
    ft_vector<game_definition_id> buildable_objects;
    asset_id command_bar_layout;
};
}
```

These structs are compiled into the Zero Hour game module, not Libft. Libft only sees their registered descriptors and opaque/type-safe catalog views.

### 7.0.4 Registration and use

At game initialization, Zero Hour installs its content types and rules into a Libft world:

```cpp
int32_t zero_hour_game_module::install(game_world &world,
                                       game_definition_registry &definitions,
                                       game_system_registry &systems)
{
    FT_TRY(register_zero_hour_definition_types(definitions));
    FT_TRY(this->_ini_importer.load_all(this->_files, definitions));
    FT_TRY(validate_zero_hour_catalog(definitions, this->_validation_report));

    FT_TRY(systems.add(this->_science_system));
    FT_TRY(systems.add(this->_special_power_system));
    FT_TRY(systems.add(this->_production_system));
    FT_TRY(systems.add(this->_victory_system));
    return FT_ERR_SUCCESS;
}
```

Libft drives the registered systems without knowing their concrete rules:

```cpp
int32_t game_world::run_phase(game_phase phase,
                              const game_tick_context &tick)
{
    return this->_systems.for_each_ordered(
        phase,
        [this, &tick](game_system &system) -> int32_t
        {
            return system.update(*this, tick);
        });
}
```

A Zero Hour rule obtains its own typed definition:

```cpp
int32_t science_system::purchase(game_entity_id player,
                                 game_definition_id science_id)
{
    ft_sharedptr<const science_definition> science;
    FT_TRY(this->_sciences.find(science_id, science));
    FT_TRY(this->verify_prerequisites(player, *science));
    FT_TRY(this->_economy.spend(player, science->purchase_cost));
    return this->grant(player, *science);
}
```

This is the central interface rule: Libft schedules and stores generic things; Zero Hour interprets `science_definition` and implements purchase behavior.

### 7.0.5 Asset ownership

Assets follow the same boundary:

- Libft File/Filesystem provides bytes, paths, hashes, asynchronous reads, and safe lifetime.
- GPGR provides texture/mesh/shader/font/video resource interfaces and opaque GPU handles.
- The game catalog owns which model, animation, texture, sound, UI layout, and effect belongs to a unit/power/faction.
- A Zero Hour W3D importer converts game-owned legacy asset references into GPGR resources.
- Libft must not contain paths such as a specific faction's command center model or a specific general's portrait.

Use stable game-owned asset IDs rather than paths throughout runtime rules:

```cpp
struct unit_definition
{
    game_definition_id id;
    asset_id model;
    asset_id selection_portrait;
    asset_id voice_set;
    game_definition_id weapon;
};
```

The asset catalog resolves `asset_id` to source/import/cache metadata; GPGR resolves the resulting render resource to a typed GPU handle.

### 7.1 Deterministic simulation kernel

Add a `game_simulation` or equivalent owned by `game_world` with:

- fixed-rate ticks expressed as `uint64_t` simulation frame numbers;
- explicit phases such as ingest commands, pre-update, movement, targeting/combat, effects, destruction, and commit;
- stable update ordering by persistent entity ID, never pointer address or hash-table bucket order;
- a deterministic command queue keyed by `(frame, player, sequence)`;
- deterministic event ordering with an insertion sequence as the final tie-breaker;
- seeded, named RNG streams with serializable state and draw counters;
- pause, single-step, fast-forward, replay, rollback checkpoint, and CRC hooks;
- a rule that background jobs may prepare data but cannot commit authoritative mutations except at deterministic phase barriers;
- per-phase profiling that cannot change ordering or state.

Libft's current event scheduler can be extended for this, but real-time timestamps must not order authoritative events. Simulation time is an integer tick domain distinct from wall/monotonic time.

### 7.2 Entity and module model

SAGE `Object` plus behavior/body/draw/update modules should become a reusable Libft entity/module system:

- `game_entity_id`: nonzero, stable, explicitly sized ID; use generation counters if IDs can be recycled.
- `game_entity_registry`: deterministic create/destroy/lookup and ordered iteration.
- component/module type IDs generated from canonical names, not compiler RTTI or address values.
- separately owned simulation components and presentation proxies so headless play has no renderer dependency.
- deferred mutation queues for creation/destruction/component changes during iteration.
- data-oriented storage where beneficial, but no mandatory archetype/ECS rewrite before parity.
- module lifecycle matching the Libft initialize/destroy/error contract.
- a registration/factory layer capable of constructing existing INI-named behavior modules.

The first port should map one SAGE object to one Libft entity and wrap each existing module category. Once behavior matches, individual modules can be rewritten natively and their wrappers removed.

### 7.3 Generic RTS interfaces to add

Libft `Game` needs reusable, data-agnostic interfaces for the following concepts. Libft provides their contracts, ordering, storage, and lifecycle; the game module provides definitions, assets, validation policy, and exact rule implementations:

- players, teams, alliances, diplomacy, victory/defeat, and observer state;
- ownership and transfer of entities;
- command issuing, validation, queuing, acknowledgement, and rejection reasons;
- selection sets and numbered control groups as client-side state;
- production queues, build placement, prerequisites, tech trees, upgrades, and resource economy;
- locomotion requests, formations, path requests, blocked-path recovery, and terrain cost layers;
- weapons, armor, damage types, projectiles, areas of effect, statuses, and death/destruction events;
- line of sight, shroud/fog, radar/detection, stealth, and reveal events;
- spatial queries, collision layers, broad-phase indexes, and deterministic query result ordering;
- AI goals, squads, tactics, threat maps, build planning, and behavior-tree adapters;
- map scripts, triggers, counters, named waypoints, teams, and cinematic commands;
- match rules, skirmish setup, campaign mission state, statistics, and replay metadata.

The Generals/Zero Hour module then composes those interfaces into command sets, sciences, special powers, factions, generals, and named INI modules. None of those concrete definitions becomes a Libft header or source file.

### 7.4 State, snapshot, and replay design

Replace `Snapshot`/`Xfer` with a Libft-native, schema-versioned state system:

- Every record has a stable numeric type/schema ID and schema version.
- Integer width and signedness are specified per field.
- Arrays/maps are length-prefixed with validated `ft_size_t`-to-wire conversions.
- References serialize entity IDs, never pointers.
- Unknown optional fields can be skipped; required-field absence is an error.
- Canonical serialization orders fields and keyed collections for deterministic hashes.
- Save snapshots, network checksums, replay checkpoints, and debugging dumps reuse the same canonical state description but may select different field groups.
- Readers enforce allocation, nesting, string, and collection limits.
- A legacy import module reads old Xfer saves/replays into the new schema; the main runtime never writes the legacy layout unless compatibility is explicitly required.

Libft `Storage`, `JSon`, `Buffer`, compression, encryption, and document source/sink APIs can support metadata and tooling, but authoritative snapshots should use a compact deterministic binary schema rather than JSON object iteration.

### 7.5 Data and scripting

- Add the generic definition/type registry described in Section 7.0. `game_data_catalog` can remain an RPG convenience catalog, but Generals must not add faction/science fields or methods to it.
- Initially parse original INI files into the new catalog through a compatibility importer; later introduce a canonical Libft config/schema representation without requiring content conversion on day one.
- Bind the sandboxed Lua bridge to command/event/query APIs, not raw entity pointers.
- Maintain deterministic script APIs: no wall clock, OS randomness, unordered iteration, filesystem access, or unrestricted floating-point-dependent host behavior.
- Preserve the original map-script engine through an adapter until a script corpus proves equivalent behavior.

### 7.6 What belongs upstream in Libft

Generic deterministic tick, entity and system registries, typed definition registration, command queue, canonical snapshot, replay/checkpoint, player/team relationships, spatial queries, and render abstractions belong in Libft. Generals definitions and assets, W3D interpretation, BIG override conventions, faction/science/general/special-power logic, balance, and exact legacy protocol/save import belong in this repository unless a genuinely game-independent interface is extracted later.

## 8. GPGR expansion into the renderer

At the inspected revision GPGR exposes only `ft_gpu_window`, `ft_gpu_shader`, a platform GL loader, input polling, and buffer swapping. Replacing WW3D therefore requires a real rendering architecture to be added.

### 8.1 Backend-neutral device API

Evolve GPGR so OpenGL is one backend behind interfaces for:

- adapters/devices, feature/capability discovery, swapchains, fullscreen/window modes, resize, vsync, and device-loss recovery;
- vertex/index/uniform/storage buffers with usage and mapping policies;
- 2D/array/cube textures, mipmaps, compressed formats, samplers, and render/depth targets;
- shader modules, programs/pipelines, vertex layouts, blend/depth/stencil/raster state;
- command lists, render passes, barriers, resource lifetime, and deferred deletion;
- timestamps, debug labels, validation messages, counters, and GPU capture integration.

Do not expose OpenGL object integers throughout the game. Use typed GPGR handles with generations so stale handles fail safely and a later Vulkan/Direct3D backend remains possible.

### 8.2 Required Generals render features

Implement in vertical slices:

1. Static W3D meshes, transforms, textures, material states, camera, and opaque pass.
2. Skeletal/hierarchical W3D animation, subobjects, damage states, and model condition switching.
3. Height-map terrain, blended terrain textures, roads, bridges, cliffs, scorch/decal layers, and culling/LOD.
4. Alpha-tested/transparent objects, sorting policy, additive effects, billboards, particles, beams, trails, and projectiles.
5. Dynamic lights, ambient/faction tinting, shadow decals, projected shadows, and—if desired later—shadow maps.
6. Water surface, reflections/refractions at an agreed parity level, weather, sky, and post-processing.
7. Shroud/fog-of-war texture generation and compositing, radar/minimap rendering, selection rings, health bars, and tactical overlays.
8. 2D UI batching, nine-slice panels, images, cursor, localized font atlas/text shaping, subtitles, and video surfaces.
9. Picking/selection rays, terrain intersection, screenshots, frame capture, resolution/UI scaling, and accessibility hooks.

### 8.3 Asset pipeline

Build readers first, converters second:

- Parse W3D and legacy texture/map resources into validated intermediate representations.
- Convert them at load time initially to prove parity.
- Add an offline `libft_assetc` tool that outputs versioned, endian-defined, bounds-checked GPGR-ready assets.
- Cache converted resources using source hashes, converter version, target backend capabilities, and settings.
- Keep original assets authoritative until converter output passes visual and animation corpus tests.

Use Libft File/Filesystem, Buffer, Compression, CLI, Logger, Threading, and Storage for the pipeline. Importers must be fuzzed because legacy binary asset data is untrusted input from a memory-safety perspective.

### 8.4 Renderer acceptance method

- Golden camera paths produce image captures at fixed frames.
- Compare images using exact masks for UI and perceptual thresholds for 3D scenes.
- Record object counts, draw calls, triangles, uploads, GPU/CPU time, and memory.
- Maintain representative maps for water, bridges, snow, night lighting, large armies, particle stress, shroud, and every major W3D animation/material feature.
- Visual parity exceptions require explicit documentation; they must not be hidden by broad image tolerances.

## 9. Libft type-system migration

Changing widths reduces accidental overflow only when each value's valid range and conversion rules are designed. Blindly replacing every `Int` with `int64_t` would enlarge hot structures, alter layouts and cache behavior, change overflow-dependent legacy behavior, and still leave unsafe narrowing at file/GPU/OS boundaries.

### 9.1 Canonical type policy

Use these defaults in new code:

| Meaning | Type | Notes |
|---|---|---|
| byte/octet | `uint8_t` | Serialized byte, not text character. |
| boolean | `ft_bool` | Use `FT_FALSE`/`FT_TRUE`; do not serialize raw C++ `bool`. |
| memory/container size | `ft_size_t` | Validate before conversion to file, network, OpenGL, or signed index types. |
| local array index | `ft_size_t` | Use a distinct invalid sentinel or optional result, not `-1`. |
| general signed arithmetic | `int32_t` or `int64_t` by declared range | Prefer the smallest type whose domain and intermediate calculations are safe. |
| entity/object ID | `uint64_t` | Reserve zero as invalid; serialize explicitly. If network size matters, use checked varints, not narrower in-memory IDs. |
| player/team/content/type ID | `uint32_t` | Stable hashed/assigned IDs with collision detection. |
| simulation frame/tick | `uint64_t` | Prevent long-session wrap; use checked differences. |
| durations | `int64_t` with unit in the type/name | Prefer strong wrappers such as ticks, milliseconds, microseconds. |
| counts/capacities on wire/disk | explicitly `uint32_t`/`uint64_t` | Enforce maxima before allocating; never serialize `ft_size_t` directly. |
| money/resources/score | usually `int64_t` | Define whether negatives are valid and use checked/saturating policy. |
| fixed-point simulation scalar | named signed integer backing | Recommended for newly stabilized authoritative calculations when exact cross-platform determinism is required. |
| render scalar | `float` | Non-authoritative presentation math; use Libft Math/Geometry conventions. |

Create strong wrappers where mixing values is dangerous: `EntityId`, `PlayerId`, `ContentId`, `SimulationTick`, `Milliseconds`, `ResourceAmount`, `WorldCoord`, and `Angle`. The wrapper's backing field remains a Libft-standard fixed-width integer.

### 9.2 Arithmetic policy

- Use checked add/subtract/multiply/cast helpers at asset, packet, allocation, economy, damage, coordinate, and timer boundaries.
- Widen intermediate expressions before multiplication or summation. A 64-bit destination does not prevent overflow if two `int32_t` operands multiply first.
- Reject malformed external values rather than clamping silently.
- Use saturation only where the gameplay rule explicitly calls for a cap; record this in the field contract.
- Unsigned types are for nonnegative domains, not a universal overflow fix. Avoid unsigned subtraction unless checked.
- Division must define zero handling and rounding direction.
- Enum storage and valid ranges are explicit; unknown serialized enum values return an error or map to a documented fallback.
- Compile with conversion/sign/overflow warnings elevated for migrated targets and use UBSan in the portable CI build.

If Libft lacks checked integer operations and strong unit wrappers, add a low-level generic facility to `Basic` or `Math` and test it across compilers.

### 9.3 Migration table for legacy aliases

Inventory every definition and field use of `Bool`, `Int`, `UnsignedInt`, `Short`, `UnsignedShort`, `Byte`, `Real`, `ObjectID`, frame/time aliases, and pointer-sized casts. Classify each occurrence as:

1. transient internal value;
2. authoritative simulation value;
3. serialized save/replay value;
4. network field;
5. asset-format field;
6. GPU/OS API value;
7. bitmask or enum.

Then migrate one subsystem at a time. At temporary boundaries use named conversion functions such as `sage_int_to_i32_checked`, `ft_size_to_u32_checked`, and `legacy_object_id_to_entity_id`. Ban raw C-style casts in migrated targets.

### 9.4 Layout and format separation

New runtime types do not dictate legacy format widths. For example, a runtime `EntityId` may be 64-bit while a legacy save has a 32-bit `ObjectID`. The legacy reader checks and converts it; the new serializer writes the new schema. Similarly, GPGR resource sizes use `ft_size_t`, while OpenGL calls receive checked `GLsizeiptr`/`GLsizei` conversions.

### 9.5 Type migration gates

- `static_assert` every legacy packed structure's size and offsets while its reader exists.
- Golden boundary tests cover minimum, maximum, just-out-of-range, negative-to-unsigned, large allocation, tick wrap, and multiplication overflow.
- Determinism tests compare before/after values at each frame, not only final CRC.
- Memory and cache benchmarks guard against making every object unnecessarily 64-bit.
- A repository check prevents reintroduction of legacy aliases in migrated directories.

## 10. Required legacy dependency removal

Some dependencies must be removed before a useful modern build is possible. Removal means deleting the runtime behavior and build dependency, not adding empty SDK files until the linker succeeds.

### 10.1 SafeDisk and copy protection — remove in Phase 0

SafeDisk is the unsupported copy-protection dependency described in the repository README. The released source already contains compile-time copy-protection branches, launcher handshakes, validation during gameplay, and campaign-score gating. This code has no place in the Libft engine.

Zero Hour removal points:

| File/area | Required change |
|---|---|
| `GeneralsMD/Code/GameEngine/Include/Common/CopyProtection.h` | Delete the `DO_COPY_PROTECTION` switch and `CopyProtect` API after callers are removed. |
| `GeneralsMD/Code/GameEngine/Source/Common/System/CopyProtection.cpp` | Remove from the build, then delete. Do not replace it with an always-successful security facade. |
| `GeneralsMD/Code/Main/WinMain.cpp` | Remove copy-protection message dispatch, launcher-running check, launcher notification, and shutdown calls. Startup proceeds directly to normal initialization. |
| `GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogic.cpp` | Remove frame-1024 validation and the failure/self-destruct behavior. |
| `GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/ScoreScreen.cpp` | Remove `copyProtectOK`; campaign victory/result flow depends only on actual match state. |
| `GeneralsMD/Code/GameEngine/GameEngine.dsp` and new CMake manifests | Remove `CopyProtection.cpp/.h`. |
| `GeneralsMD/Code/Tools/Launcher/` | Remove `COPY_PROTECT`, `Protect.cpp/.h`, SafeDisk includes, SafeDisk launcher chaining, and SafeDisk project groups. Prefer launching the game executable directly; keep only useful argument/update/bootstrap behavior in a later Libft launcher. |
| `GeneralsMD/Code/Tools/Launcher/DatGen/` | Remove `SafeDisk/CdaPfn.h` and retire the SafeDisk data-generation path. |

Apply the equivalent removal to `Generals/` when that game target is brought over. Do not remove `MSG_SELF_DESTRUCT` merely because its comment mentions copy protection: verify whether the message is also used for the legitimate “quit to observer” behavior, rename/document it if retained, and remove only protection-triggered sends.

Acceptance checks:

- No `SafeDisk`, `CopyProtect`, `DO_COPY_PROTECTION`, or `COPY_PROTECT` reference appears in a shipped target.
- The game starts directly, a campaign victory advances normally, and a long match passing frame 1024 is unaffected.
- No missing SafeDisk headers/libraries are part of configure or link checks.

### 10.2 GameSpy — remove in Phase 0; multiplayer intentionally unavailable

Remove GameSpy rather than attempting to preserve dead service behavior. Early builds are explicitly offline-only: campaign, skirmish, replay, options, content loading, and tools are the focus. Multiplayer menus must be disabled with a clear “multiplayer is unavailable in this development build” state; they must not crash, hang, start background threads, or try network connections.

Removal scope:

- Remove the complete `GeneralsMD/Code/GameEngine/Source/GameNetwork/GameSpy/` and corresponding `Include/GameNetwork/GameSpy/` groups from new build manifests.
- Remove top-level GameSpy implementations and headers: `GameSpy.cpp`, `GameSpyChat`, `GameSpyGameInfo`, `GameSpyGP`, `GameSpyOverlay`, `GameSpyPersistentStorage`, and `GameSpyThread`.
- Remove `Common/GameSpyMiscPreferences.h` or migrate genuinely generic preferences to offline settings.
- Remove `../Libraries/Source/GameSpy` include paths, `Libraries/Source/GameSpy` SDK expectations, and GameSpy libraries such as HTTP, Presence, Stats, Peer, and Patching from link configuration.
- Remove GameSpy menu assets from active menu registration or make them unreachable during the transition.
- Remove GameSpy initialization and shutdown includes from `GameEngine.cpp`, especially `GameResultsThread` and related global queues.
- Audit generic network files before deletion. `NetworkInterface`, `Network`, `Connection`, `Transport`, frame data, UDP, LAN, file transfer, and message parsing are not all GameSpy; retain them only as reference/fixtures until the later Libft Networking phase.

Introduce one explicit feature state instead of scattered null checks:

```cpp
enum class MultiplayerAvailability : uint8_t
{
    unavailable_during_engine_migration = 0,
    available = 1
};

static MultiplayerAvailability multiplayer_availability()
{
    return MultiplayerAvailability::unavailable_during_engine_migration;
}
```

Frontend pseudocode:

```cpp
int32_t open_multiplayer_menu(frontend_context &frontend)
{
    if (multiplayer_availability() != MultiplayerAvailability::available)
    {
        frontend.show_notice("Multiplayer is unavailable while the Libft engine port is in progress.");
        return FT_ERR_INVALID_STATE;
    }
    return frontend.open_screen(frontend_screen::multiplayer);
}
```

Here `int32_t` is the function's return storage type and `FT_ERR_INVALID_STATE` is a named error value returned in that type. `FT_ERR_*` is **not** an integer-size family and must never be described or used as one.

### 10.3 Other dependency disposition

| Dependency | Early decision | Final direction |
|---|---|---|
| STLport 4.5.3 | Keep only if needed to compile the SAGE reference target | Remove; new Libft code uses Libft/modern compiler facilities. |
| DirectX 8/9-era renderer libraries | Keep for reference renderer if it can build | Remove after GPGR parity. |
| WW3D/WWMath/WWSaveLoad | Keep as reference/import implementation | Replace runtime with GPGR/Libft; retain narrow asset importers as needed. |
| Miles Sound System / Asimp3 | Stub or disable audio only if needed for first headless target | Replace with a supported Libft audio backend/open codec stack. |
| Bink | Disable video playback initially with a clean skip/fallback | Replace with a supported video/codec backend. |
| GameSpy SDK | Remove immediately | Later multiplayer uses Libft Networking/API; no GameSpy compatibility goal. |
| SafeDisk | Remove immediately | No replacement. Ownership is established by obtaining the game as stated in the repository README, not runtime DRM. |
| 3DS Max 4 SDK | Do not build exporter initially | Replace with standalone conversion/import tools or a plugin for a supported DCC version. |
| NVASM | Disable legacy shader assembly tooling where not required | Replace with GPGR shader pipeline and modern offline compilation. |
| BYTEmark | Remove from required game build | Use Libft and project-specific benchmarks. |
| zlib 1.1.4 / LZH-Light | Retain only for legacy format compatibility | Replace runtime compression with Libft adapters after byte-level corpus tests; keep isolated legacy decoders if assets require them. |

The first supported build profile should therefore be `offline-headless`, followed by `offline-gpgr`. Neither target links GameSpy, SafeDisk, Miles, or Bink. Audio/video can initially report `FT_ERR_UNSUPPORTED` and the application must continue when the content is nonessential.

## 11. Staged architecture and code examples

The examples below are pseudocode: names may change to match actual Libft headers, but the dependency direction, ownership, error handling, and migration sequence are normative.

### 11.1 Phase-to-file map

| Phase | Add first | Modify during phase | Remove at exit |
|---|---|---|---|
| 0: build/offline baseline | `CMakeLists.txt`, `CMakePresets.json`, `cmake/LegacySources.cmake`, `tests/determinism/` | `GeneralsMD/Code/Main/WinMain.cpp`, legacy project manifests, frontend multiplayer actions | SafeDisk/copy-protection sources; GameSpy sources/SDK/link inputs |
| 1–2: Libft runtime/types | `engine/runtime/include/EngineRuntime/Runtime.hpp`, `Result.hpp`, `Types.hpp`, `src/libft/`; Libft checked-integer additions if missing | `GameMain`, logging/time/CLI call sites, `Lib/BaseType.h` boundaries | migrated debug/timer/CLI implementations |
| 3: files/content | `engine/assets/FileService`, `LegacyBigReader`, `LegacyIniImporter`, corpus tests | SAGE `FileSystem` callers and content bootstrap | SAGE loose-file implementation after importer parity |
| 4–5: memory/jobs | `engine/runtime/MemoryDomains`, `JobSystem`, deterministic commit queue | memory-pool families and background workers one family at a time | replaced pool/thread implementations |
| 6: simulation kernel | Libft `Game` additions for ticks/entities/commands/snapshots; `engine/simulation/` | `GameEngine.cpp` main loop and adapters around `GameLogic`, `Object`, `MessageStream`, `RandomValue` | nothing broad until the first vertical slice passes |
| 7: gameplay slices | Libft RTS facilities plus `games/zero_hour/rules/` | related SAGE GameLogic module families and INI registration | each SAGE slice immediately after parity |
| 8: GPGR | GPGR resource/device/render-pass APIs; `engine/render/`; asset importers | presentation event production and frontend/window bootstrap | WW3D/device renderer after visual parity |
| 9: multiplayer | Libft command protocol, session/transport/lobby services | re-enable multiplayer frontend and add new settings | retained legacy generic networking reference code |
| 10–11: peripherals/final removal | audio/video backends, tools, compatibility libraries | remaining UI/tools and game target composition | remaining SAGE runtime, STLport, proprietary SDK assumptions |

### 11.2 Bootstrap evolution

Phase 1 keeps SAGE simulation but moves process ownership to Libft services:

```cpp
int32_t game_main(int32_t argument_count, char **argument_values)
{
    engine_runtime runtime;
    int32_t error_code = runtime.initialize(argument_count, argument_values);
    if (error_code != FT_ERR_SUCCESS)
        return error_code;

    sage_reference_game game;
    error_code = game.initialize(runtime);
    if (error_code == FT_ERR_SUCCESS)
        error_code = game.run_offline();

    game.destroy();
    runtime.destroy();
    return error_code;
}
```

At the Phase 7 exit the same composition root owns no SAGE game engine:

```cpp
int32_t game_main(int32_t argument_count, char **argument_values)
{
    engine_runtime runtime;
    zero_hour_game game;

    FT_TRY(runtime.initialize(argument_count, argument_values));
    FT_TRY(game.initialize(runtime));
    FT_TRY(game.load_content("Data/INI"));

    const int32_t run_error = game.run_offline();
    game.destroy();
    runtime.destroy();
    return run_error;
}
```

`FT_TRY` here means “return the non-success error from the called function”; it does not define the width of any gameplay value.

### 11.3 Deterministic simulation loop

The Libft world owns authoritative ticking; render time and wall time do not:

```cpp
int32_t zero_hour_game::advance_one_tick()
{
    game_tick_context tick;
    tick.number = this->_next_tick;                 // uint64_t / SimulationTick
    tick.random_streams = &this->_random_streams;

    FT_TRY(this->_commands.consume_for_tick(tick.number, this->_world));
    FT_TRY(this->_world.begin_tick(tick));
    FT_TRY(this->_world.run_phase(game_phase::locomotion, tick));
    FT_TRY(this->_world.run_phase(game_phase::combat, tick));
    FT_TRY(this->_world.run_phase(game_phase::effects, tick));
    FT_TRY(this->_world.commit_deferred_mutations(tick));
    FT_TRY(this->_world.end_tick(tick));

    this->_last_hash = this->_world.canonical_state_hash();
    ++this->_next_tick;
    return FT_ERR_SUCCESS;
}
```

During migration, a comparator runs the same recorded command in SAGE and Libft:

```cpp
int32_t parity_runner::advance_and_compare()
{
    FT_TRY(this->_sage.advance_one_tick());
    FT_TRY(this->_libft.advance_one_tick());

    comparison difference;
    FT_TRY(compare_tick_state(this->_sage.export_probe_state(),
                              this->_libft.export_probe_state(),
                              difference));
    if (!difference.equal)
        return this->report_first_difference(difference);
    return FT_ERR_SUCCESS;
}
```

### 11.4 Entity migration example

First wrap a SAGE object without changing its behavior:

```cpp
struct legacy_object_component
{
    Object *reference; // temporary, never serialized
};

int32_t import_sage_object(game_entity_registry &registry, Object &object,
                           game_entity_id *entity_id_out)
{
    if (entity_id_out == ft_nullptr)
        return FT_ERR_INVALID_ARGUMENT;

    FT_TRY(registry.create_with_stable_id(convert_object_id(object.getID()),
                                          entity_id_out));
    return registry.add_component(*entity_id_out,
                                  legacy_object_component{&object});
}
```

Then replace a vertical slice with native components:

```cpp
struct transform_component { world_position position; facing_angle facing; };
struct health_component { int64_t current; int64_t maximum; };
struct weapon_component { content_id weapon; simulation_tick next_fire_tick; };

int32_t combat_system::update(game_world &world, const game_tick_context &tick)
{
    return world.for_each_ordered<transform_component, health_component,
                                  weapon_component>(
        [&world, &tick](game_entity_id entity, auto &transform,
                        auto &health, auto &weapon) -> int32_t
        {
            return update_weapon_deterministically(world, tick, entity,
                                                   transform, health, weapon);
        });
}
```

`for_each_ordered` must guarantee entity-ID ordering. A normal unordered container is not acceptable for authoritative updates.

### 11.5 Checked type conversion example

```cpp
int32_t legacy_object_id_to_entity_id(UnsignedInt legacy_id,
                                      game_entity_id *entity_id_out)
{
    if (entity_id_out == ft_nullptr || legacy_id == INVALID_ID)
        return FT_ERR_INVALID_ARGUMENT;

    entity_id_out->value = static_cast<uint64_t>(legacy_id);
    return FT_ERR_SUCCESS;
}

int32_t resource_add_checked(int64_t current, int64_t amount,
                             int64_t *result_out)
{
    if (result_out == ft_nullptr)
        return FT_ERR_INVALID_ARGUMENT;
    if (amount > 0 && current > FT_INT64_MAX - amount)
        return FT_ERR_OUT_OF_RANGE;
    if (amount < 0 && current < FT_INT64_MIN - amount)
        return FT_ERR_OUT_OF_RANGE;
    *result_out = current + amount;
    return FT_ERR_SUCCESS;
}
```

The return is an error code. The `int64_t` fields define the resource domain. These are separate decisions.

### 11.6 Simulation-to-GPGR boundary

The renderer consumes immutable presentation data and cannot mutate the game world:

```cpp
int32_t build_render_snapshot(const game_world &world,
                              render_snapshot &snapshot_out)
{
    snapshot_out.tick = world.current_tick();
    return world.for_each_visible_entity_ordered(
        [&snapshot_out](game_entity_id id, const transform_component &transform,
                        const renderable_component &renderable) -> int32_t
        {
            return snapshot_out.instances.push_back(
                render_instance{id, transform.position, transform.facing,
                                renderable.model, renderable.condition_flags});
        });
}

int32_t gpgr_renderer::render(const render_snapshot &snapshot)
{
    FT_TRY(this->_device.begin_frame());
    FT_TRY(this->_terrain_pass.draw(snapshot));
    FT_TRY(this->_opaque_pass.draw(snapshot));
    FT_TRY(this->_effects_pass.draw(snapshot));
    FT_TRY(this->_ui_pass.draw(snapshot));
    return this->_device.end_frame();
}
```

Early Phase 6 headless targets omit the renderer entirely. Phase 8 attaches GPGR to the same presentation snapshot contract.

### 11.7 Error and shutdown policy

Every fallible initialize/update/load call returns an `FT_ERR_*` value stored in `int32_t`. Destruction remains idempotent and attempts complete cleanup even if initialization stopped halfway:

```cpp
int32_t zero_hour_game::initialize(engine_runtime &runtime)
{
    int32_t error_code = this->_world.initialize();
    if (error_code != FT_ERR_SUCCESS)
        return error_code;

    error_code = this->_catalog.initialize(runtime.files());
    if (error_code != FT_ERR_SUCCESS)
    {
        this->_world.destroy();
        return error_code;
    }
    this->_initialized = FT_TRUE;
    return FT_ERR_SUCCESS;
}
```

Do not use error codes as object IDs, counts, sizes, flags, or booleans. Do not infer success from zero unless comparing to `FT_ERR_SUCCESS`; this keeps the public contract readable.

## 12. Compatibility invariants

These invariants are release gates, not aspirations:

### Determinism and networking

- Fixed simulation step and frame numbering do not change.
- Given the same seed and command stream, object IDs, updates, CRCs, and final state match.
- Logic RNG state and draw order match exactly.
- Associative-container iteration affecting simulation is explicitly ordered.
- Floating-point compiler flags are controlled; no fast-math migration.
- Packet structures use fixed-width fields and explicit endian encoding, never raw new class layouts.
- Legacy clients either remain protocol-compatible or a deliberate protocol version rejects them clearly.

### Saves, replays, and assets

- `Snapshot`/`Xfer` order and version bytes remain unchanged until a versioned serializer is introduced.
- Golden save/replay fixtures load and re-save with documented equivalence.
- Archive search order, path normalization, case folding, and duplicate-name behavior remain identical.
- W3D, BIG, TGA/DDS, audio, map, and INI corpora have read/parse tests.
- Old content is not silently rewritten.

### ABI, memory, and text

- The initial target remains Win32/x86 to isolate modernization from pointer-size changes.
- Public packed structs and file/network layouts get `static_assert(sizeof/offsetof)` checks.
- Allocator ownership never crosses a module boundary unless both allocation and free use the same service.
- UTF-16/wide-character assumptions are documented. `UnicodeString` conversion to Libft/UTF-8 must be explicit and loss-tested.
- No exception may cross C callbacks, device libraries, or module ABI boundaries.

## 13. Build and dependency plan

### 13.1 What the obsolete build actually does

`GeneralsMD/Code/RTS.dsw` is a Visual C++ 6 workspace containing 34 projects. The primary graph is approximately:

```text
RTS.exe
  ├── GameEngine.lib
  │     ├── Compression.lib
  │     ├── Benchmark.lib
  │     └── EABrowserDispatch.lib
  ├── GameEngineDevice.lib
  │     ├── Compression.lib
  │     ├── Benchmark.lib
  │     └── EABrowserDispatch.lib
  ├── WW3D2 / WWDebug / WWLib / WWUtil / WWMath / WWSaveLoad / wwshade
  ├── GameSpy HTTP / Presence / Stats / Peer / Patching
  ├── WWDownload, profile, debug
  ├── Bink, Miles, Direct3D 8, D3DX8, DirectInput 8, DirectSound
  ├── buildVersionUpdate + versionUpdate custom steps
  └── launcher
```

The workspace also builds tools such as WorldBuilder, GUIEdit, ImagePacker, MapCacheBuilder, ParticleEditor, CRCDiff, patchgrabber, and the obsolete launcher/update programs.

Important findings from the project files:

- `RTS.dsp` has 218 source/resource/data entries, `GameEngine.dsp` has 1,159, and `GameEngineDevice.dsp` has 212. Many entries in `RTS.dsp` are runtime INI/window/string assets rather than compilable sources.
- Configurations are `Release`, `Debug`, `Internal`, `Profile`, plus a `DebugW3D` variant. They achieve variants by changing library filenames and preprocessor definitions rather than using one target graph with configuration properties.
- The executable is forced to `/machine:I386` and links a long handwritten list of Windows system and proprietary libraries.
- Include paths assume missing source SDK directories and mix public/private headers globally.
- `PreRTS.h` injects ATL, Windows, DirectInput, MAPI, SNMP, VFW, WinInet, registry, shell, multimedia, STLport, engine globals, and game headers into nearly every GameEngine translation unit. This is a major reason apparently “common” code is not portable.
- The version custom build runs executables from a hard-coded `\projects\generalsmd\production\Run` directory and modifies `generatedVersion.h`/`buildVersion.h` during builds. This is non-reproducible and creates dirty source trees.
- GameEngineDevice invokes `../tools/nvasm/nvasm` repeatedly to convert `.nvp/.nvv` files into `.pso/.vso`, with configuration-dependent output paths.
- Several tools copy outputs directly to `Run`, a 3DS Max installation, or other assumed developer-machine paths.
- Debug/release libraries use different manually encoded names such as `GameEngineDebug.lib`; the CRT is also selected manually with `/MD` or `/MDd` and old `/nodefaultlib` workarounds.
- The `.dsw` claims dependencies on GameSpy projects whose SDK tree is absent except for placeholders, so the declared graph cannot be reproduced from this checkout.

The `.dsp` files remain useful as an inventory and ordering reference, but should never be mechanically converted into the final CMake architecture. Doing so would preserve the same monolith and missing dependencies in a newer file format.

### 13.2 Build-system objectives

The CMake build must:

- configure on Windows, Linux, and macOS without pretending every target is available everywhere;
- always produce at least dependency reports, portable libraries, and tests supported by the current migration stage;
- keep platform code in platform-specific targets selected by CMake conditions;
- support Ninja, Unix Makefiles, and Visual Studio/Xcode generators without generator-specific commands;
- use target-scoped include directories, definitions, features, warnings, and link dependencies;
- keep all generated files and binaries under the build directory;
- never write into a game installation or source `Run` directory during compilation;
- separate compilation from asset staging and packaging;
- fail at configure time with a precise message when an explicitly requested capability is unavailable;
- allow the default early build to succeed without SafeDisk, GameSpy, Miles, Bink, NVASM, or the 3DS Max SDK;
- make offline/headless the first portable product and multiplayer opt-in only after its Libft replacement exists.

CMake makes the build description portable; it does not make Windows-dependent source code portable automatically. Portability comes from selecting `platform/windows`, `platform/linux`, or `platform/macos` implementations behind the same Libft/engine interfaces.

### 13.3 Proposed CMake file layout

```text
CMakeLists.txt
CMakePresets.json
cmake/
  ProjectOptions.cmake
  CompilerWarnings.cmake
  Sanitizers.cmake
  Dependencies.cmake
  InstallLayout.cmake
  LegacySourceInventory.cmake
  ValidateConfiguration.cmake
  GenerateBuildInfo.cmake
external/
  libft/                         # pinned submodule
engine/
  CMakeLists.txt
  runtime/CMakeLists.txt
  assets/CMakeLists.txt
  serialization/CMakeLists.txt
  simulation/CMakeLists.txt
  presentation/CMakeLists.txt
  render/CMakeLists.txt
  platform/windows/CMakeLists.txt
  platform/linux/CMakeLists.txt
  platform/macos/CMakeLists.txt
games/
  generals/CMakeLists.txt
  zero_hour/CMakeLists.txt
tools/CMakeLists.txt
tests/CMakeLists.txt
packaging/CMakeLists.txt
```

The old `GeneralsMD/Code` paths can be referenced from checked-in manifests during migration. Files should not be physically moved merely to make the initial CMake configure work.

### 13.4 Target graph

Use namespaced CMake targets and link only declared responsibilities:

```text
libft::basic, libft::game, libft::gpgr, ...

cnc::runtime
cnc::assets
cnc::legacy_big_importer
cnc::legacy_ini_importer
cnc::serialization
cnc::simulation
cnc::presentation
cnc::renderer_gpgr
cnc::platform

cnc::generals_rules
cnc::zero_hour_rules

cnc_generals_headless
cnc_zero_hour_headless
cnc_generals
cnc_zero_hour
```

During parity work, optional reference targets may exist:

```text
sage_reference_common
sage_reference_simulation
sage_reference_renderer_win32
sage_parity_runner
```

They are not dependencies of final Libft-native targets. CMake should be able to prove this using the link graph.

Target rules:

- Use `STATIC` libraries for owned implementation boundaries and `INTERFACE` libraries only for policies/header-only APIs.
- Use `OBJECT` libraries sparingly for the temporary case where the same legacy sources must feed a test and reference executable without inventing a false public library API.
- Every target declares `PUBLIC`, `PRIVATE`, and `INTERFACE` dependencies deliberately.
- No directory-wide `include_directories`, `link_directories`, `add_definitions`, or global compiler flags.
- No library filename construction based on Debug/Release. Link target names and let CMake choose configuration artifacts.
- Do not make tools dependencies of the game unless they generate a declared build output. Asset conversion intended for distribution belongs in a separate explicit target.

### 13.5 Options and feature states

Initial cache options should be small and capability-oriented:

```cmake
option(CNC_BUILD_ZERO_HOUR       "Build Zero Hour targets" ON)
option(CNC_BUILD_GENERALS        "Build Generals targets" OFF)
option(CNC_BUILD_HEADLESS        "Build renderer-free game/test targets" ON)
option(CNC_BUILD_GPGR            "Build the Libft GPGR renderer" OFF)
option(CNC_BUILD_SAGE_REFERENCE  "Build temporary Windows SAGE reference targets" OFF)
option(CNC_BUILD_TOOLS           "Build supported portable tools" OFF)
option(CNC_BUILD_TESTS           "Build tests" ON)
option(CNC_ENABLE_MULTIPLAYER    "Build Libft multiplayer after it exists" OFF)
option(CNC_ENABLE_AUDIO          "Build supported audio backend" OFF)
option(CNC_ENABLE_VIDEO          "Build supported video backend" OFF)
option(CNC_ENABLE_SANITIZERS     "Enable supported sanitizers" OFF)
```

There should be no options for SafeDisk or GameSpy: those systems are removed. `CNC_ENABLE_MULTIPLAYER=ON` must initially fail configuration with a clear “not implemented during engine migration” error rather than silently compiling dead GameSpy code.

`CNC_BUILD_SAGE_REFERENCE` is Windows/x86-only and temporary. It can accept explicit SDK paths if a developer has legal copies, but it must never be required by the default build or CI portability matrix.

### 13.6 Presets and supported build profiles

Use checked-in configure/build/test presets rather than a long list of command-line flags:

| Preset | Purpose | Expected platforms |
|---|---|---|
| `dev-headless` | Libft runtime, asset import, deterministic simulation, tests; no renderer/audio/video/network | Windows, Linux, macOS |
| `dev-gpgr` | Offline game with GPGR | platforms supported by the expanded GPGR backend |
| `ci-debug` | Warnings, assertions, unit/integration tests | all CI platforms |
| `ci-asan` | Address/undefined behavior diagnostics where supported | Clang/GCC platforms first |
| `release-offline` | Optimized offline product and install/package rules | supported desktop platforms |
| `sage-reference-win32` | Temporary original behavior runner | Windows x86 only |

Recommended initial user workflow once implementation begins:

```sh
cmake --preset dev-headless
cmake --build --preset dev-headless
ctest --preset dev-headless
```

Presets select a build directory such as `out/build/dev-headless`; they do not encode a user-specific compiler path. Developers select a supported compiler through their normal environment or a local, uncommitted user preset.

### 13.7 Libft integration

The desired integration is:

```cmake
add_subdirectory(external/libft EXCLUDE_FROM_ALL)
target_link_libraries(cnc_runtime PRIVATE libft::basic libft::time libft::logger)
target_link_libraries(cnc_simulation PRIVATE libft::game libft::rng)
target_link_libraries(cnc_renderer_gpgr PRIVATE libft::gpgr)
```

Libft currently documents GNU Make as canonical and its CMake quick-start gathers translation units for a consumer. For a durable cross-platform build, Libft should gain or expose maintained CMake targets per module with:

- stable namespaced target names;
- transitive module dependencies matching `Docs/module_dependency_graph.md`;
- target-scoped public include paths and compile features;
- Windows/Linux/macOS platform source selection;
- consistent static/shared and debug/release behavior;
- install/export metadata if it will also be consumed outside this repository.

Until upstream Libft CMake support exists, this project may keep a temporary `cmake/LibftAdapter.cmake` with explicit checked-in source lists. It must be pinned to one Libft commit and verified against Libft's generated dependency graph. Do not use `ExternalProject` to invoke GNU Make as the normal solution: it complicates multi-configuration generators, target dependency propagation, cross-compilation, IDE integration, and sanitizer flags.

Before vendoring, resolve licensing in writing. GitHub metadata and the inspected tree did not expose a recognized license file at the reviewed revision. A combined distribution requires an explicit compatible license.

### 13.8 Source manifests and migration from `.dsp`

Use the `.dsp` `SOURCE=` records to create the first inventory, but check the resulting CMake lists into the repository and review them by ownership:

```cmake
set(SAGE_REFERENCE_COMMON_SOURCES
    "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Source/Common/AsciiString.cpp"
    "${PROJECT_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Source/Common/RandomValue.cpp"
    # Explicit list continues.
)
```

Do not recursively glob production sources. Globs hide ownership changes, pick up obsolete/GameSpy/copy-protection files accidentally, and make configure behavior dependent on whatever happens to be present in the checkout.

Each legacy file is classified in `LegacySourceInventory.cmake` as one of:

- `reference_only`;
- `portable_candidate`;
- `windows_backend`;
- `legacy_importer`;
- `remove_safedisk`;
- `remove_gamespy`;
- `blocked_proprietary_sdk`;
- `retired_tool`.

The inventory is temporary and shrinks as native targets replace it. A validation function should reject GameSpy and SafeDisk paths if they appear in an active source list.

### 13.9 Platform separation

Replace the global `PreRTS.h` assumption with narrow precompiled headers per temporary target. A PCH is a compile optimization, not a dependency declaration.

```cmake
if(WIN32)
    target_sources(cnc_platform PRIVATE platform/windows/Platform.cpp)
elseif(APPLE)
    target_sources(cnc_platform PRIVATE platform/macos/Platform.mm)
elseif(UNIX)
    target_sources(cnc_platform PRIVATE platform/linux/Platform.cpp)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()
```

Portable targets may not include `windows.h`, ATL, DirectX, WinInet, registry, shell, or multimedia headers. CI should run a source/include audit for this rule. The temporary SAGE reference targets can retain `PreRTS.h` and old platform dependencies while they exist.

Avoid architecture checks such as `if(CMAKE_SIZEOF_VOID_P EQUAL 4)` as a substitute for type correctness. The reference runner starts x86 for parity, while new Libft targets should build on native 64-bit platforms from the beginning. Legacy readers explicitly decode legacy 32-bit layouts.

### 13.10 Generated build information

Replace `versionUpdate.exe` and `buildVersionUpdate.exe` with configure-time generation into the binary directory:

```cmake
configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/BuildInfo.hpp.in"
    "${PROJECT_BINARY_DIR}/generated/BuildInfo.hpp"
    @ONLY
)
```

Build information should use an explicit project version plus an optional Git commit obtained without modifying sources. Reproducible builds must permit a caller-provided timestamp such as `SOURCE_DATE_EPOCH`; do not embed the current time by default. Generated headers are normal target dependencies and must not be produced by executing a freshly built GUI tool from a hard-coded directory.

### 13.11 Assets, runtime tree, install, and packaging

The old project treats many `Run/Data`, `Run/Window`, and string files as IDE source entries and writes executables directly beside them. CMake should separate four concerns:

1. **Compile:** create libraries/executables under the build tree.
2. **Test fixtures:** copy only declared small fixtures with `configure_file(COPYONLY)` or target-specific custom commands.
3. **Developer staging:** an explicit `stage_zero_hour` target assembles a runnable directory under `out/stage/<preset>` from user-provided legally owned game data plus project overrides.
4. **Install/package:** `install(TARGETS ...)`, `install(FILES/DIRECTORY ...)`, and CPack assemble redistributable project-owned artifacts without copying proprietary game data automatically.

Use a cache path such as `CNC_GAME_DATA_ROOT` only for staging/testing locally. Validate it at stage time, never bake it into binaries, and never require it merely to compile unit tests.

Legacy shader `.nvp/.nvv` custom commands are not ported. GPGR shaders receive a new source format/compiler pipeline. Until GPGR reaches that feature, those shaders belong only to the optional reference renderer and its NVASM dependency report.

### 13.12 Dependency discovery policy

Use this order:

1. pinned source dependency via `add_subdirectory` for Libft and small audited dependencies;
2. CMake config package through `find_package(... CONFIG REQUIRED)` for system/developer-provided libraries;
3. `FetchContent` only for an explicitly pinned public dependency when repository policy permits network fetching;
4. never silently search arbitrary old SDK directories or `Libraries/Lib` for filenames.

The default configure must not access the network. CI and developers initialize pinned submodules explicitly. Every optional dependency exposes a CMake imported target such as `OpenAL::OpenAL`, never raw include/library variables propagated globally.

### 13.13 Compiler and warning policy

Set a deliberate C++ standard per target through `target_compile_features`; start with the minimum supported by current Libft and raise it through an ADR. Do not depend on compiler-default language modes.

Policies:

- warnings are strict for new Libft-native code;
- legacy reference code gets a documented, target-local suppression list that must shrink;
- warnings-as-errors is enabled in CI for native targets, not blindly for untouched legacy sources;
- exceptions and RTTI settings are explicit and consistent at ABI boundaries;
- MSVC runtime selection uses `CMAKE_MSVC_RUNTIME_LIBRARY`;
- visibility is hidden by default where shared libraries are introduced;
- sanitizer, coverage, LTO, and static-analysis switches are orthogonal options/presets, not new source configurations;
- floating-point flags for deterministic simulation are explicit and tested across compilers.

### 13.14 Tests as build targets

Use `include(CTest)` and register independently runnable tests with labels:

```text
unit
asset-corpus
determinism
parity                 # requires sage-reference-win32
render-golden          # requires GPGR/display or offscreen context
integration-offline
multiplayer            # absent/disabled until Phase 9
```

Tests consume normal targets rather than recompiling private `.cpp` lists with different flags. Fixtures are declared inputs. `ctest --preset ...` must be the same local and CI entry point.

### 13.15 CMake implementation sequence

When implementation is authorized, add CMake in buildable increments:

1. **Configure-only skeleton:** root project, options, presets, dependency validation, `libft` adapter, CTest, and a build report. Default configuration succeeds without legacy SDKs.
2. **Portable smoke target:** compile a tiny executable against selected Libft targets on all three desktop platforms.
3. **Offline headless libraries:** add asset importers and the new game/simulation kernel with explicit manifests and tests.
4. **Windows SAGE reference target:** optional x86 target only, with SafeDisk/GameSpy removed and remaining proprietary dependencies reported. This is not the portability baseline.
5. **Parity runner:** link/launch isolated reference and Libft implementations and compare recorded state.
6. **GPGR product target:** add only after the expanded GPGR target/resource API builds independently.
7. **Game executables and staging:** assemble offline Zero Hour first, then Generals.
8. **Tools and multiplayer:** enable later as supported native targets rather than blocking the core build.

Each step must configure, build, and test before adding the next. A target that cannot yet build should not be added with dozens of placeholder source files; represent it as a documented disabled capability.

### 13.16 Initial CI matrix

```text
Windows: MSVC x64     dev-headless, tests
Windows: clang-cl x64 ci-debug, tests
Linux:   Clang x64    dev-headless, ASan/UBSan
Linux:   GCC x64      release-offline/headless
macOS:   AppleClang   dev-headless, tests
Windows: MSVC x86     sage-reference-win32 parity job (optional/non-blocking initially)
```

The portable Libft-native matrix is blocking. The reference job may initially be non-blocking because missing legacy SDKs are expected, but its status and missing dependency report remain visible.

### 13.17 Definition of build-system success

The CMake migration is complete when:

- a clean checkout plus pinned dependencies configures without VS6, `.dsw`, `.dsp`, STLport, SafeDisk, GameSpy, or proprietary SDKs for the default headless target;
- Windows, Linux, and macOS build the same portable target graph with platform backends selected explicitly;
- offline Zero Hour can be staged from user-provided game data through a documented command;
- no build command modifies tracked source files or writes to hard-coded external paths;
- every binary/library has one owning CMake target and all dependencies are target-scoped;
- disabled features fail clearly when requested;
- configure/build/test/install/package workflows are reproducible through presets;
- the `.dsw/.dsp` files are no longer consulted by active builds and can move to an archival directory.

### 13.18 First implemented slices on `libft-engine-migration`

The implementation branch now contains the configure/build skeleton described above:

```text
CMakeLists.txt                         # project options, CTest, profile checks
CMakePresets.json                      # dev-headless and release-offline presets
cmake/LibftBasic.cmake                 # explicit pinned Basic source manifest
tests/smoke/libft_smoke.cpp            # type/error/checked-arithmetic smoke test
```

This deliberately builds only Libft `Basic` (excluding `basic_locale_compare.cpp`, which links `System_utils`) and the smoke executable. It does not claim to build SAGE, the renderer, or the game yet. The smoke slice was compiled and executed with Clang in this environment; the installed environment did not contain the `cmake` executable, so CMake configure/build itself remains to be run on a machine with CMake 3.20+ and Ninja or another supported generator.

The branch now also contains the first Libft-native engine layers:

```text
engine/runtime/include/CncRuntime/Types.hpp
engine/runtime/include/CncRuntime/Runtime.hpp
engine/runtime/src/Runtime.cpp
engine/simulation/include/CncSimulation/World.hpp
engine/simulation/src/World.cpp
engine/simulation/include/CncSimulation/SystemRegistry.hpp
engine/simulation/src/SystemRegistry.cpp
engine/simulation/include/CncSimulation/DefinitionRegistry.hpp
engine/simulation/src/DefinitionRegistry.cpp
```

`cnc::Runtime` owns the first composition-root lifecycle and exposes Libft type/error conventions. `cnc::DeterministicWorld` is intentionally a generic scaffold rather than a Generals rules module: it provides stable 64-bit entity IDs, insertion-sequenced commands, fixed tick advancement, checked signed arithmetic, lifecycle error returns, and a canonical state hash. It is the seam where Libft `Game` facilities will be integrated next; it does not yet model factions, units, or SAGE behavior.

`cnc::SystemRegistry` is the next generic seam. It registers callbacks by phase, explicit signed order, and registration sequence, then propagates the first non-success `FT_ERR_*` result. It deliberately has no knowledge of Generals systems; the future Zero Hour module will register its own science, production, combat, and victory systems through this interface.

`cnc::DefinitionRegistry` now provides the corresponding data-hosting seam. Libft owns type/content-ID uniqueness, opaque lifetime through an explicit destroy callback, lookup, and validation dispatch; a game target owns the concrete definition structs and typed wrappers. The current smoke test registers a synthetic definition, looks it up, and verifies cleanup. This is intentionally not a Generals catalog yet.

The first concrete game-owned adapter is now present under `games/zero_hour`:

```text
games/zero_hour/include/ZeroHourData/Catalog.hpp
games/zero_hour/src/Catalog.cpp
```

`zero_hour::Catalog` registers faction, science, general, and special-power type
descriptors, allocates the concrete records, installs a minimal default dataset,
and exposes typed lookup functions. The registry destroys the records, so
ownership remains explicit and the Libft-facing layer never needs to know what a
faction, general, or power means. The smoke test exercises initialization,
registration, typed lookup, and shutdown. This pattern is the template for
adding units and rules data; it should be extended with real asset IDs and
loaders rather than moving those records into Libft.
The catalog also exposes a validation pass that dispatches each concrete record
through its game-owned validator and returns a `ValidationReport`; malformed
asset data can therefore be rejected before a match starts.

`Catalog::load_manifest` provides the first asset-facing path. It accepts a
deterministic CSV grammar (`SCIENCE`, `FACTION`, `GENERAL`, `POWER`), parses
fixed-width numeric fields with range checks, rejects malformed or duplicate
records, and validates the complete catalog before returning. The fixture at
`tests/fixtures/zero_hour_manifest.csv` is copied by CMake and loaded by the
smoke test. This is an interim game-owned loader; once Libft File is portable it
should replace the stream I/O while preserving this grammar and ownership
boundary.

The parser is now exposed separately as `Catalog::load_manifest_text`. File
acquisition delegates to that function, so a future Libft File adapter can feed
the exact same parser without changing ownership, validation, or error
semantics. The smoke test covers both the staged-file and in-memory paths.

`GameSession::load_data_manifest` is the composition-root entry point for a
real match. It loads the game-owned catalog from a path and wires the science,
special-power, and general ledgers to that catalog. Call it only on a fresh
session (a populated catalog returns `FT_ERR_ALREADY_EXISTS`) so live rules
cannot change under a running simulation. A future Libft File adapter can feed
the same API without moving ownership into Libft.
The catalog now separates acquisition from parsing with
`Catalog::load_manifest_with_reader`. A reader callback supplies file contents
and can be implemented with Libft File/Filesystem later; validation and game
ownership remain entirely in Zero Hour. The current `load_manifest` is only the
standard-library reader used until that Libft adapter is enabled.

Command sequencing is also bounded: the session refuses a new command when
its 64-bit insertion sequence reaches `UINT64_MAX`, returning
`FT_ERR_OUT_OF_RANGE` instead of wrapping and silently changing deterministic
ordering.

Catalog validation also follows references between records: factions must point
to an existing science, and generals must point to an existing faction and
special power. The invalid fixture
`tests/fixtures/zero_hour_invalid_manifest.csv` proves that a syntactically
valid but semantically broken manifest returns `FT_ERR_CONFIGURATION` instead
of entering a match.

`zero_hour::ScienceLedger` is the first concrete game-rule system. It consumes
the catalog through typed lookups, enforces available-point and duplicate
purchase rules, tracks purchased science IDs, and reports failures with Libft
`FT_ERR_*` results. Its state is game-owned and deterministic; Libft remains
responsible only for the generic registry/data lifetime beneath it.

`GameSession` now owns the ledger and initializes it after the catalog is
installed; shutdown destroys ledger state before catalog records. Game systems
can therefore access `session.science_ledger()` through the same composition
root used for world commands, renderer state, and the offline network
capability.

`zero_hour::SpecialPowerLedger` now provides the matching game-owned cooldown
rule seam. It validates power IDs through the catalog, computes fixed-tick
ready times with overflow checks, rejects activation while cooling down, and is
lifecycle-managed by `GameSession` alongside `ScienceLedger`.

`zero_hour::GeneralRoster` binds generic `EntityId` values to validated
game-owned general definitions. Duplicate or unknown assignments return
`FT_ERR_*` errors, while the world itself remains ignorant of factions and
generals. The roster is also owned and reset by `GameSession`, establishing the
pattern for unit, faction, and player-component bindings.

The presentation seam now has a portable `cnc::HeadlessRenderer` in
`engine/render`. It models initialize/begin/submit/end/shutdown and validates
resource IDs and dimensions while retaining frame/command counters for tests.
The target is deliberately backend-free: a future GPGR renderer can implement
the same command contract, and an offscreen backend can verify parity without a
display. The smoke test exercises one complete frame. No DirectX, X11, or GPGR
source is linked until the renderer/resource API is expanded in its own target.
`Renderer` is the backend-neutral interface exposed by `GameSession`; the
headless implementation is the current adapter and GPGR can later implement
the same lifecycle and command contract.

The first end-to-end product target is `cnc_zero_hour_headless` under
`apps/zero_hour_headless`. It initializes `GameSession`, installs the game-owned
catalog, advances one typed command, submits one validated render command, and
prints the resulting tick/hash/frame counters. CMake registers it as
`integration.offline`; this is the initial runnable Zero Hour path and remains
independent of SAGE, DirectX, SafeDisk, GameSpy, and multiplayer.
With `--manifest <path>` it exercises the same game-owned manifest loader used
by staged assets; without arguments it uses built-in defaults.
`--help` reports the active backend profile so launchers can distinguish this
offline/headless migration build from a future networked or GPGR-enabled one.

The root CMake file now installs only declared native targets and their public
headers under standard `GNUInstallDirs` locations; it never copies the legacy
`Run/` tree. The offline executable is installable as a normal runtime target,
and CPack emits ZIP/TGZ packages containing project-owned artifacts. Proprietary
game data remains an explicit staging input and is not bundled automatically.

The new `stage_zero_hour` target implements the explicit staging boundary. Set
`CNC_GAME_DATA_ROOT` to a legally owned data directory, then run
`cmake --build <build-dir> --target stage_zero_hour`; CMake validates the path
at stage time and copies the executable, declared project manifest, and data
into `build/stage/zero_hour`. The root is empty by default, so compilation and
unit tests never require proprietary game files.

The install graph now exports native targets under the relocatable
`CnCGeneralsZeroHour` package configuration. Consumers can use
`find_package(CnCGeneralsZeroHour CONFIG REQUIRED)` and link targets such as
`cnc::runtime`, `cnc::simulation`, or `cnc::game_session`; the package defines a
compatibility `libft::basic` alias for the exported Libft Basic target. Exported
interfaces do not contain source-tree paths.

`tests/package_consumer` is an out-of-tree-style package smoke project. CI
configures and builds it against the installed prefix with
`find_package(CnCGeneralsZeroHour CONFIG REQUIRED)`, proving exported targets,
headers, and transitive links work after installation rather than only inside
the repository build.
The consumer explicitly uses Ninja, matching the single-configuration package
producer and avoiding configuration-mapping differences on Windows IDE
generators.
Install rules declare archive, shared-library, and runtime destinations for
each native target so the same exported package layout works with static or
shared toolchains on all supported platforms.

Configure-time build metadata now replaces the legacy `versionUpdate.exe` and
`buildVersionUpdate.exe` mutation flow. CMake generates
`CncBuild/BuildInfo.hpp` in the binary tree with the project version and a short
Git commit (or `unknown` outside a Git checkout); no tracked source is modified
and no timestamp is embedded by default, preserving reproducible builds.
The generated header is installed with the runtime public headers, so packaged
offline builds expose the same version/commit contract to tools and launchers.

`.github/workflows/headless.yml` now makes the portable matrix executable: each
push/PR configures, builds, and tests `dev-headless` on Windows, Ubuntu, and
macOS. A separate Ubuntu Clang job enables conversion warnings-as-errors plus
ASan/UBSan. The Libft submodule URL is HTTPS so hosted runners do not require a
developer SSH key; the pinned branch/commit remains controlled by the
submodule metadata.

The sanitizer job uses the checked-in `ci-sanitizers` configure/build/test
presets rather than inline cache overrides. Its binary directory is isolated
from `dev-headless`, so local and CI runs cannot mix instrumented and normal
objects.

The offline-first multiplayer boundary is now explicit in
`engine/network/CncNetwork/OfflineNetworkSession`. Initialization and shutdown
are harmless, but connect/send return `FT_ERR_INVALID_OPERATION` and
`is_online()` is always false. The stub is a real CMake target and smoke-tested,
so future Libft networking can replace it behind a known capability contract
without re-enabling GameSpy or silently creating a partially working lobby.
`NetworkSession` is the abstract contract exposed by `GameSession`; the
offline implementation is only one backend, keeping game code independent of
transport selection.

`cnc::GameSession` now acts as the headless composition root. Its startup order
is Runtime -> DeterministicWorld -> Zero Hour Catalog; shutdown reverses that
order and clears scheduled systems. A tick runs ingest systems, advances the
world, then runs simulation and presentation systems. This gives future unit,
production, combat, renderer, and networking modules one explicit lifecycle
without embedding Generals rules in Libft. `engine/game` is exposed as the
`cnc::game_session` CMake target and is covered by the smoke executable.

The composition root now owns the renderer and network capabilities as well:
startup initializes the offline network capability and renderer after world/data
initialization, and shutdown tears them down before the catalog/world/runtime.
Callers therefore receive one lifecycle-managed backend graph; the offline app
uses `session.renderer()` and cannot accidentally bypass the configured session.

`GameSession` now captures a replay record after every successful tick: the
simulation tick and canonical state hash are stored in a session-owned history
that can be cleared or consumed by future Libft replay/file adapters. Capture
happens after presentation systems complete, so failed ticks are never reported
as successful replay frames.
`verify_replay` compares an expected sequence against the captured tick/hash
records and returns `FT_ERR_CONFIGURATION` on the first divergence. This gives
the future Libft replay/file adapter a deterministic acceptance gate before
playback or network synchronization is attempted.

The session also owns the first input boundary: `submit_world_delta` accepts a
validated entity command, assigns a monotonically increasing sequence number,
and stores it until the next tick. Commands are stably sorted and dispatched to
the deterministic world before ingest systems run; a failed world validation
leaves the tick unsuccessful. This is deliberately transport-neutral, so the
future Libft networking adapter and local UI can produce the same command stream
without becoming simulation dependencies. Multiplayer remains disabled; this
is only the offline command path.

The first determinism gate is now implemented in
`tests/determinism/replay.cpp`. It runs the same typed command stream through two
fresh sessions and compares the canonical state hash after every tick. The test
is exposed as the `cnc_determinism_replay` CMake target and the
`determinism.replay` CTest case. This should remain a blocking test as concrete
faction, economy, combat, and map systems replace the scaffold.

The active-source exclusion audit is implemented in
`cmake/ValidateMigrationSources.cmake`, exposed as the
`cnc_validate_migration_sources` target and `migration.exclusions` CTest case.
It scans only `engine/`, `games/`, and `tests/` (not the archival `GeneralsMD/`
tree) and fails on SafeDisk, copy-protection, or GameSpy references. This keeps
retired middleware out of new code while the legacy inventory tracks what still
must be removed when the optional reference target is dismantled.
The same audit rejects native-width spellings (`unsigned int`, `unsigned long`,
and `long long`) in active sources; fixed-width `<cstdint>` types or Libft
aliases are required for new systems.

Native migration targets now share `cmake/CompilerWarnings.cmake`. On MSVC it
enables `/W4` and `/permissive-`; on GCC/Clang it enables `-Wall`, `-Wextra`,
`-Wpedantic`, `-Wconversion`, and `-Wsign-conversion`. `CNC_WARNINGS_AS_ERRORS`
can promote those diagnostics to errors without imposing new flags on the
third-party Libft archive or the obsolete SAGE reference. The conversion checks
are intentional: they expose accidental narrowing while the codebase moves to
Libft fixed-width aliases and explicit `FT_ERR_*` result handling.

`CNC_ENABLE_SANITIZERS` is available for native targets. With a Clang/GCC
generator it adds AddressSanitizer and UndefinedBehaviorSanitizer plus frame
pointers to the runtime, simulation, game, renderer, app, and test targets.
MSVC fails clearly when this option is requested rather than silently ignoring
it; the CI matrix can select a Clang job for sanitizer coverage.

The next implementation step is to add maintained Libft CMake targets (or temporary explicit adapters) for the transitive `Errno`, `System_utils`, `Time`, `File`, and `Game` modules, then replace this scaffold's storage with Libft `Game` registries and typed systems. Do not expand the manifest to every Libft `.cpp` file until each module's platform and third-party dependencies are represented as CMake targets.

The first dependency probe is represented by `cmake/LibftTime.cmake`. It has an
explicit source manifest and a `libft::time` target, but `CNC_BUILD_LIBFT_TIME`
defaults to `OFF`: the current Libft Time headers transitively include
`PThread/recursive_mutex.hpp` (`pthread.h`) and `Errno` internal headers. That
would make the default Windows graph fail before the platform abstraction is
ready. This is an intentional capability gate, not a silent fallback. The
portable runtime continues using `std::chrono` until Libft PThread and
Compatibility are added as platform-selected targets; only then should the
runtime link `libft::time`.

`cmake/LibftGame.cmake` now records the complete Libft Game source manifest as
an opt-in `libft::game` target. `CNC_BUILD_LIBFT_GAME` defaults to `OFF` because
Game includes Template containers, PThread mutexes, internal Errno headers,
Lua, and platform Compatibility code; linking it into the default target would
reintroduce the exact portability failure the migration is avoiding. Once those
dependencies have platform-selected CMake targets, this manifest is the single
place to enable Game and the game-owned adapters can consume only the specific
Libft services they need.

## 14. Engine replacement phases

### Phase 0 — Baseline and archaeology

Deliverables:

- A CMake inventory of all current targets and missing SDK stubs.
- A runnable headless or minimally rendered executable.
- SafeDisk/copy-protection and GameSpy removed from the active source/link graph.
- An explicit offline-only feature state with multiplayer UI disabled cleanly.
- Golden fixtures: skirmish command recordings, RNG vectors, CRC checkpoints, saves, replays, representative BIG archives and INIs.
- Characterization tests for string, path, archive, INI, `Xfer`, memory-pool, and network-frame behavior.
- CI for x86 Debug/Release and tests.

Exit gate: the baseline is reproducible and failures are distinguishable from migration changes.

### Phase 1 — Runtime shell

Add `EngineRuntime`, error translation, and dependency ownership. Instantiate it before legacy global subsystem initialization. Provide fake clock/files/logging services for tests. Keep compatibility accessors so current call sites do not all change.

Exit gate: runtime creation/destruction is leak-checked and legacy startup behavior is unchanged.

### Phase 2 — Diagnostics, time, and CLI

- Map `DEBUG_LOG`, assertions, `MiniLog`, and performance markers to Libft logging/observability sinks.
- Replace scattered timing calls behind `Clock`; preserve tick and frame pacing math.
- Parse command line using Libft CLI and populate a typed `StartupOptions`; translate to `TheWritableGlobalData` during transition.
- Add checked integer/cast helpers, strong IDs/units, and the legacy-type inventory described in Section 9.

Exit gate: logs contain equivalent diagnostics, command-line golden tests pass, and deterministic runs produce the same CRC series.

### Phase 3 — Files, paths, and configuration shell

- Implement a Libft-backed `LocalFileSystem` or a new adapter beneath it.
- Keep `ArchiveFileSystem` and BIG parsing intact; make it consume the file service.
- Move preferences, new engine config, temp files, and atomic writes to runtime services.
- Keep original INI parser for content and build a corpus test before any replacement.

Exit gate: loose/packed asset resolution, mod override order, non-ASCII user paths, read-only installs, and save/preferences tests pass.

### Phase 4 — Allocator and ownership replacement

- Define allocation domains: simulation, renderer, audio, network, assets, tools.
- Back general domains with a Libft `CMA` adapter and expose statistics/limits.
- Keep legacy pools for classes that depend on pool macros, address stability, or fixed layout.
- Migrate pool families individually only after allocation/failure/performance tests.

Exit gate: no cross-heap frees, no new lifetime bugs, stable frame-time and memory budgets, and fault-injection tests pass.

### Phase 5 — Concurrency and background services

- Replace bespoke worker lifecycle with Libft cancellation, queues, guards, and thread pools.
- Start with logging, file hashing, downloads, decompression, cache building, and tools.
- Marshal results to the main thread through a deterministic completion queue.
- Do not parallelize authoritative object updates in this phase.

Exit gate: clean shutdown under cancellation, ThreadSanitizer/diagnostic runs where available, and no nondeterministic simulation output.

### Phase 6 — Libft Game deterministic kernel

- Add fixed ticks, stable entity IDs, deterministic command/event ordering, named RNG streams, canonical snapshots, and per-frame hashes to Libft `Game`.
- Make a headless Libft `game_world` run the match lifecycle.
- Dual-run a small scenario by mirroring SAGE object creation, commands, movement, damage, and destruction into the Libft world and comparing every phase.
- Port player/team/diplomacy, entity registry, and the minimum locomotion/combat components needed for one unit-versus-unit fixture.

Exit gate: a headless Libft world reproduces selected SAGE fixtures tick-for-tick without SAGE owning the migrated systems.

### Phase 7 — Simulation vertical slices

- Port complete slices in dependency order: map/spatial queries; locomotion/pathing; targeting/weapons/damage; ownership/economy; production/build placement; upgrades/special powers; fog/radar; AI; scripts; victory/campaign state.
- For each slice, add generic facilities to Libft `Game`, implement Generals rules in the game layer, dual-run fixtures, and then delete the corresponding SAGE runtime implementation.
- Migrate the slice's types, containers, errors, allocation, and serialization fully to Libft conventions.
- Emit renderer-independent presentation snapshots/events.

Exit gate: complete headless skirmish and representative campaign missions run on Libft `Game`; SAGE `GameLogic`, `Object`, module factories, and authoritative `Snapshot/Xfer` no longer execute.

### Phase 8 — GPGR renderer replacement

- Expand GPGR's device, resource, command, and pipeline APIs.
- Port visual slices in the order listed in Section 8, starting with static W3D models and terrain.
- Feed GPGR presentation snapshots/events rather than SAGE `Drawable` objects.
- Add an offline asset converter after runtime importers establish correctness.
- Replace input/window services through GPGR/platform modules and port UI, text, minimap, and shroud.

Exit gate: representative matches and frontend screens render through GPGR with accepted visual and performance parity; WW3D and the SAGE renderer are not linked.

### Phase 9 — Libft networking and online stack

- Define a Libft-native versioned command protocol using fixed-width schemas and canonical simulation commands.
- Use Libft Networking, Compression, Encryption, and Observability for LAN discovery, reliable command delivery, file transfer, matchmaking/lobby integration, NAT strategy, and desync reporting.
- Keep the original packet reader only as an optional compatibility gateway if required.
- Replace obsolete GameSpy services with a documented Libft server/API deployment.
- Add fuzzing, latency/loss simulation, soak tests, and multi-platform determinism sessions.

Exit gate: multiplayer matches run entirely through Libft networking and simulation, including graceful failure and desync diagnostics.

### Phase 10 — Audio, video, UI, and tools

- Define or expand Libft audio/video device abstractions for required game behavior.
- Replace Miles/Bink or define supported open codec pipelines.
- Port WorldBuilder and maintained asset tools to Libft runtime, GPGR, file, command, and serialization APIs.
- Use Libft `Command` for editor undo/redo where its caller-owned model fits; extend it for transactions/history persistence if needed.
- Remove SafeDisk, obsolete patching, and unsupported online/tool projects deliberately.

Exit gate: the supported game and tool distribution has no required proprietary legacy SDK.

### Phase 11 — Final type migration and SAGE removal

- Remove STLport and remaining SAGE strings, containers, errors, allocators, globals, base types, and subsystem lifecycle.
- Preserve only named compatibility libraries for promised legacy files, assets, saves, or protocols.
- Make Zero Hour and Generals game/content targets over the same Libft engine.
- Add dependency checks proving runtime targets do not link SAGE, WW3D, or `GameEngineDevice` code.

Exit gate: SAGE is no longer the engine, only a historical reference and optional data-import implementation.

### Phase 12 — Post-parity improvements

After parity, evaluate explicitly versioned improvements such as higher tick rates, rollback/rejoin, multithreaded deterministic phases, larger maps/player counts, modern rendering, native Lua modding, new asset formats, and replacement AI behavior. These changes should not be mixed into parity work.

## 15. Required test architecture

### Unit and characterization tests

- Primitive/string conversions, including empty, embedded NUL, invalid UTF, and long strings.
- Path normalization, case rules, traversal prevention, and archive precedence.
- Every legacy command-line flag and conflicting-option precedence.
- RNG golden sequences for all seeds/ranges and serialized states.
- Xfer primitives, object graphs, versions, CRC, and post-load processing.
- Allocator alignment, exhaustion, destructor, and cross-module ownership behavior.

### Determinism harness

Build a headless runner that accepts map, players, seed, and recorded commands, then emits:

- per-frame light CRC;
- periodic deep snapshot CRC;
- RNG stream positions;
- object creation/destruction IDs;
- final canonical snapshot.

Run old/reference and migrated implementations on identical fixtures and compare the output. A final-state-only test is insufficient because compensating divergences can hide earlier errors.

### Integration and corpus tests

- Boot to shell, load map, start/skirmish/end match, save/load, replay, and mod override flows.
- Parse all shipped INIs and enumerate all shipped archives.
- Load representative W3D, texture, audio, and map assets.
- Two- and multi-client network tests with packet loss, jitter, duplication, and disconnect/rejoin conditions supported by the protocol.
- Tool smoke tests for WorldBuilder and content utilities that remain supported.

### Performance gates

Capture baseline and budgets for startup, map load, simulation update, render submission, network frame processing, peak/steady allocations, archive reads, and shutdown. Libft's benchmark/trace facilities can record these, but acceptance thresholds belong to this project.

## 16. Risks and mitigations

| Risk | Impact | Mitigation |
|---|---|---|
| Libft license is absent/unclear | Combined distribution may be impermissible | Resolve and commit an explicit compatible license before vendoring/distribution. |
| Dense Libft module graph | Binary size, compile time, unexpected coupling | Per-subsystem module includes, selected-module build, dependency rules, include audits, and link-map checks. |
| Two allocator/error/lifecycle models | leaks, double frees, confusing failure state | One boundary policy, ownership annotations, error translation, fault injection. |
| RNG/math/container changes | multiplayer desyncs | Golden sequences, deterministic harness, fixed ordering, defer replacements. |
| Save/packet/layout changes | corrupt saves or incompatible clients | Fixed-width serialization, version adapters, fixture corpus, layout assertions. |
| Legacy compiler assumptions | long porting tail | Modern x86 baseline first, warnings inventory, staged conformance fixes. |
| Missing proprietary middleware | executable still cannot link/run | Track renderer/audio/video/online replacements as separate workstreams with stubs. |
| Global singleton coupling | difficult testing and shutdown order | Runtime composition root plus compatibility accessors; migrate constructor dependencies gradually. |
| Engine replacement scope | years of unstable work before anything runs | Vertical slices that end in playable/headless results, dual execution, per-subsystem deletion gates, and no mass type replacement. |
| GPGR is currently minimal | renderer work is substantially underestimated | Treat renderer as a first-class workstream with capability milestones, asset corpus, image tests, and measured parity levels. |
| Wider types change behavior/layout | desyncs, memory growth, broken formats | Semantic type inventory, checked conversions, explicit schemas, strong IDs/units, layout assertions, and cache benchmarks. |

## 17. Decisions that must be made before implementation

1. **Supported outcome:** preservation-compatible Win32 build, modern Windows remaster, or cross-platform engine. The plan supports all three, but device-backend scope and compatibility gates differ.
2. **First playable target:** Zero Hour is recommended because it is the later codebase; Generals should follow through shared-engine extraction.
3. **Compatibility promise:** decide whether original saves, replays, mods, and network clients must remain compatible. Default recommendation: preserve saves/content; version network compatibility explicitly.
4. **Renderer backend order:** GPGR replacement is decided; choose its first production backend (continued OpenGL is the shortest path) and the required parity level for shadows, water, video, and UI text.
5. **Libft licensing and release policy:** settle before integration.
6. **Libft consumption model:** selected module archives versus full static archive, after the compiler spike measures both.
7. **C++ baseline:** choose a standard supported by both codebases and the intended compilers; do not let an IDE conversion decide implicitly.

## 18. First implementation milestone

The first milestone should be small and evidence-producing:

1. Add top-level CMake presets for an x86 headless test target.
2. Compile a narrow slice containing base types, strings, RNG, Snapshot/Xfer, GameState/logic essentials, and fixture support.
3. Add the deterministic runner and two or three recorded scenarios.
4. Pin Libft after its licensing is resolved.
5. Add `engine_runtime` with only `Clock`, `Logger`, and `PathService` adapters.
6. Switch those three services behind build/runtime flags and compare behavior/performance.
7. Add a Libft `Game` prototype containing fixed ticks, stable entities, deterministic commands, RNG state, and canonical hashes; reproduce one recorded combat fixture.
8. Expand GPGR just enough to render one imported static W3D model and terrain patch in a golden camera test.
9. Produce the legacy integer/type inventory and implement checked conversion helpers before migrating gameplay fields.

This milestone validates the toolchain, Libft extension workflow, deterministic simulation direction, GPGR asset/render direction, type policy, Windows support, and parity methodology before scaling the replacement across the game.

## 19. Definition of “Libft has replaced SAGE”

The restructuring can truthfully claim this when:

- `game_state`/`game_world` plus Libft RTS extensions own authoritative state, entity/module lifecycle, ticks, events, commands, replay, and scripting.
- GPGR owns the live renderer, window/input presentation path, GPU resources, and render passes.
- Libft modules own allocation, errors, diagnostics, time, files, configuration, jobs, compression, networking, persistence, math, and geometry used by runtime targets.
- New runtime APIs use Libft typing and lifecycle conventions; SAGE base types, strings, global subsystem model, allocators, and containers are absent.
- Direct OS/GPU/audio calls are confined to Libft platform/backend implementations.
- The original SAGE simulation, WW3D renderer, `GameEngineDevice`, networking implementation, and `Snapshot/Xfer` are not linked into the shipped runtime.
- Generals and Zero Hour are game/content targets over the same Libft-native engine.
- Determinism, content, visual, save/import, networking, and performance gates pass.

Legacy SAGE code may remain in the repository as a behavioral reference and in narrowly named import/compatibility libraries. It is not part of the running engine.

## 20. Source references

Current repository evidence:

- `README.md`
- `GeneralsMD/Code/RTS.dsw` and `RTS.dsp`
- `GeneralsMD/Code/GameEngine/Include/Common/GameEngine.h`
- `GeneralsMD/Code/GameEngine/Include/Common/SubsystemInterface.h`
- `GeneralsMD/Code/GameEngine/Include/Common/AsciiString.h`
- `GeneralsMD/Code/GameEngine/Include/Common/UnicodeString.h`
- `GeneralsMD/Code/GameEngine/Include/Common/MemoryPool.h`
- `GeneralsMD/Code/GameEngine/Include/Common/Snapshot.h`
- `GeneralsMD/Code/GameEngine/Include/Common/Xfer.h`
- `GeneralsMD/Code/GameEngine/Include/Common/FileSystem.h`
- `GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp`

Libft evidence reviewed from GitHub at the pinned revision:

- root `README.md`
- `Docs/module_layering.md`
- `Docs/module_dependency_graph.md`
- `Docs/quick_start_templates/cmake/README.md`
- module READMEs for `Basic`, `Buffer`, `CLI`, `CMA`, `CPP_class`, `Compatebility`, `Compression`, `Config`, `Debug`, `File`, `Filesystem`, `Game`, `Geometry`, `JSon`, `Logger`, `Math`, `Networking`, `Observability`, `Parser`, `PThread`, `RNG`, `Storage`, `System_utils`, `Threading`, `Time`, and `XML`

Because Libft is actively changing, re-run the module/API and license audit whenever the pinned revision changes.
