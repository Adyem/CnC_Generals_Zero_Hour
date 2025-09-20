# WWLib Cross-Platform Window & Event Interface Design

## Goals
- Remove direct dependencies on `<windows.h>` constructs (`HWND`, `HACCEL`, `MSG`, `WPARAM`, `LPARAM`) from WWLib and higher layers.
- Provide a single abstraction that wraps an `sf::Window` and exposes the lifecycle hooks, message pumping, and accelerator behaviors required by the existing WWLib subsystems.
- Preserve existing responsibilities (keyboard/mouse input, dialog focus, accelerator routing) while enabling future platform ports.

## Architectural Overview
```
+------------------------+
|      WWApplication     |
+------------------------+
            |
            v
+------------------------+     +-------------------------+
| WindowSystem (new)     | --> | EventDispatch (new)     |
+------------------------+     +-------------------------+
            |                               |
            v                               v
    sf::Window owner               WWLib Consumers
 (creation, resize, etc.)   (Keyboard, Mouse, DialogMgr)
```

- **WindowSystem** owns the real `sf::Window` instance, handles creation/destruction, tracks state formerly provided via `ProgramInstance` / `MainWindow`, and exposes lightweight handles to the rest of WWLib.
- **EventDispatch** consumes `sf::Event` objects, normalizes them into WWLib-friendly `WindowEvent` objects, and routes them to registered listeners (keyboard, mouse, dialog hooks, accelerator map).

## Core Types

### `WindowToken`
- Replaces global `HWND` handle usage.
- Lightweight, copyable identifier (e.g., `struct WindowToken { uint32_t id; };`).
- Issued by `WindowSystem::primary_window()` and passed through to consumers that need to correlate events or request operations (confine cursor, query size).
- Encapsulates platform handle retrieval (`WindowSystem::native_handle(WindowToken)`), permitting platform-specific fallback when needed.

### `AcceleratorId`
- Replaces `HACCEL` usage.
- Enum/strong typedef representing logical accelerator tables (e.g., `AcceleratorId::MainMenu`, `AcceleratorId::DebugOverlay`).
- Backed by a map of accelerator definitions stored in JSON/INI and parsed at startup.
- Allows registration via `WindowSystem::register_accelerator(AcceleratorId, AcceleratorBindingSet)` and dispatch through `EventDispatch` when key combinations match.

### `DialogHook`
- Replaces dialog-focused callbacks that currently use Win32 dialog procedures.
- `using DialogHook = std::function<bool(const DialogEvent&)>;`
- `DialogEvent` is a variant-style struct that covers focus/activation, command buttons, text edits, etc.
- Managed via `WindowSystem::push_dialog_hook(DialogHook)` / `pop_dialog_hook()`; top-most hook receives events first, emulating modal dialog stacks.

### `WindowEvent`
A normalized event structure mirroring the subset of `sf::Event` consumed by WWLib:
```cpp
struct WindowEvent {
    WindowToken window;
    std::variant<Close, Resize, Key, MouseButton, MouseMove, Text, Focus> payload;
};
```
- `Key` includes accelerator metadata (modifiers, scancode, unicode) to replace `MSG` + `WPARAM/LPARAM` parsing.
- `MouseMove` and `MouseButton` carry both window-relative and screen-relative coordinates to cover current `ClientToScreen` usage.

## Modules & Responsibilities

### WindowSystem
- **Construction**: `WindowSystem::create_primary(const WindowConfig&)` builds the `sf::Window` and stores a `WindowToken` for the primary window.
- **Lifecycle**: `poll_events()` fetches all pending `sf::Event`s and forwards them to `EventDispatch`.
- **State Queries**: exposes window size, DPI, focus, cursor confinement, and platform handles without exposing `<windows.h>` to consumers.
- **Accelerators**: loads accelerator configuration, maintains active sets based on the focused window, and works with `EventDispatch` for activation.

### EventDispatch
- Maintains listener lists per event category (keyboard, mouse, system).
- Provides subscription APIs used by `WWKeyboard`, `WWMouse`, and any code previously hooking into `Windows_Message_Handler`.
- Handles accelerator detection by matching `Key` events against registered combinations and invoking `AcceleratorListener` callbacks.
- Invokes dialog hooks prior to default propagation, mirroring the Win32 message short-circuiting behavior.

### Input Modules
- `WWKeyboard` subscribes to `Key` and `Text` events, removing all knowledge of `MSG`/`WM_*` constants.
- `WWMouse` subscribes to `MouseMove` and `MouseButton`, using `WindowSystem` helpers for cursor confinement/capture.
- Other consumers (scroll, tooltip, debug overlays) receive events via dedicated listener interfaces rather than global state inspection.

## Dialog Lifecycle
1. Modal dialog opens → caller pushes a `DialogHook` capturing dialog state.
2. `EventDispatch` delivers relevant `WindowEvent` entries to the top hook.
3. Hook returns `true` to mark the event handled; otherwise, propagation continues to standard listeners.
4. When dialog closes, caller pops the hook; base listeners resume unfiltered event handling.

## Accelerator Processing
- Accelerator tables defined in configuration as `{"id": "MainMenu", "bindings": [{"combo": "Ctrl+S", "command": "Save"}, ...]}`.
- On `Key` events, `EventDispatch` asks `AcceleratorManager` to resolve combos.
- Resolved commands trigger callbacks registered with `WindowSystem::on_accelerator(AcceleratorId, AcceleratorCallback)`.
- Mirrors `TranslateAccelerator` + `DispatchMessage` flow without exposing Win32 types.

## Threading & Message Pump
- `WindowSystem::poll_events()` replaces `GetMessage/PeekMessage` loops.
- Main loop:
```cpp
while (app.running()) {
    window_system.poll_events();
    game.update();
    game.render();
}
```
- Idle processing hooks can be provided via `WindowSystem::on_idle(IdledCallback)` to mimic `WM_TIMER`/`WM_IDLE` handling where required.

## Configuration Integration
- Window creation parameters (`WindowConfig`) and accelerator bindings retrieved from the new filesystem-backed configuration store (JSON/INI).
- `WindowSystem` receives a `ConfigStore&` on construction to load/save window position, size, fullscreen flags, and accelerator definitions.

## Migration Strategy
1. Introduce `WindowSystem` & `EventDispatch` alongside existing Win32 wrappers.
2. Update `WWKeyboard`, `WWMouse`, and dialog-heavy modules to consume the new interfaces via dependency injection.
3. Transition higher-level systems to request `WindowToken` instead of `HWND`/`ProgramInstance`.
4. Once all consumers are migrated, delete `win.h/win.cpp`, registry-backed accelerator loading, and Win32 message pump.

This design isolates SFML-specific constructs to the `WindowSystem` layer, allowing WWLib to interact through portable tokens, events, and callbacks while preserving the behaviors expected by the existing game code.
