# WW3D bgfx Migration – Step 6 Validation Notes

## Automated Checks
- Searched the W3D device tree for lingering `<windows.h>` and Direct3D header inclusions; none remain after the earlier cleanup pass.
- Reviewed the W3D display bootstrap to ensure it unconditionally registers the bgfx backend before initializing WW3D.
- Walked through the shader compiler bootstrap to confirm the host platform macros select the matching `shaderc --platform` value on Windows (`windows`), Linux (`linux`), macOS (`osx`), and FreeBSD (`freebsd`).

## Outstanding Work
- The legacy Direct3D-only subsystems (e.g. terrain texture utilities and height map helpers) still hold references to `IDirect3D*` types and require a follow-up refactor before the renderer is fully backend-agnostic.
- End-to-end build and runtime smoke validation cannot be executed in the current container because the Windows-only toolchain and proprietary assets are unavailable.

## Next Actions
1. Schedule a Windows build with the bgfx toolchain to confirm there are no regressions in the final executable.
2. Continue migrating the remaining resource managers that expose raw Direct3D interfaces to the new backend abstraction.
3. Update QA documentation once a complete bgfx-powered binary has been produced and smoke-tested on target hardware.

## Shader Build Platform Switches
- **Windows**: `_WIN32` ➜ `shaderc --platform windows`
- **Linux**: `__linux__` ➜ `shaderc --platform linux`
- **macOS**: `__APPLE__` ➜ `shaderc --platform osx`
- **FreeBSD**: `__FreeBSD__` ➜ `shaderc --platform freebsd`
- **Other hosts**: fall back to the compiler default after logging a diagnostic so port bring-up can wire in the proper mapping.
