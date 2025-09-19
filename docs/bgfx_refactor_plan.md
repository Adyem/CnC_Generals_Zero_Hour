# WW3D bgfx Refactor Plan

## Goals
- Replace legacy DirectX 8 wrapper infrastructure with bgfx-based abstractions.
- Provide a concrete `BgfxRenderBackend` implementation for the W3D device.
- Remove remaining Direct3D-only code paths and headers once bgfx is authoritative.

## Assumptions & Preparations
1. Confirm that the bgfx dependency is available to the build system (headers, libs, runtime shader tools).
2. Audit existing `WW3D_BGFX_AVAILABLE` usage and understand how the macro controls compilation.
3. Identify all call sites that currently depend on `DX8Wrapper`, `IDirect3D*` pointers, or other Direct3D-specific types.
4. Set up a development/test environment capable of running the game or targeted smoke tests after each major milestone.

## Step-by-Step Roadmap

### Step 1 – Make bgfx the default code path
- Update headers such as `texture.h`, `surfaceclass.h`, `dx8vertexbuffer.h`, and any others gated by `WW3D_BGFX_AVAILABLE` so that the macro is defined by default.
- Replace `#ifdef`/`#if` guards with `#if !defined(WW3D_BGFX_AVAILABLE)` patterns where necessary to preserve opt-out builds.
- Ensure the build configuration (project files, makefiles) define or respect the macro consistently.
- Compile to validate that enabling the macro does not immediately break unrelated platforms.

### Step 2 – Reimplement `DX8Wrapper` functionality using bgfx handles
- Inventory functions exposed by `DX8Wrapper` (`dx8wrapper.h/.cpp`) and categorize them (resource creation, state management, rendering commands, etc.).
- Design corresponding helper functions or thin wrappers that operate on bgfx handles (`bgfx::TextureHandle`, `bgfx::VertexBufferHandle`, etc.).
- Migrate shared utility logic (format conversions, usage flags) into reusable modules where possible.
- Implement and unit-test (where feasible) the new bgfx-based functions, ensuring parity with legacy behavior.
- Update clients of `DX8Wrapper` to include the new header(s) and use the bgfx-based API.

### Step 3 – Introduce `BgfxRenderBackend`
- Review the `IRenderBackend` interface in `Generals/Code/GameEngineDevice/Source/W3DDevice/GameClient` and document required entry points.
- Implement `BgfxRenderBackend`, handling:
  - Device/context initialization and shutdown through bgfx.
  - Resource creation/destruction (textures, vertex/index buffers, shaders) using the helpers from Step 2.
  - Frame submission, swap-chain management, and window-system integration.
  - Shader program loading, uniform setup, and state management.
- Provide conversion utilities between engine enums/structures and bgfx equivalents.
- Add dependency injection or factory code in the device initialization path to instantiate `BgfxRenderBackend`.

### Step 4 – Update engine subsystems to use the new backend
- Modify `W3DDisplay.cpp`, `W3DShaderManager.cpp`, `TextureClass`, `SurfaceClass`, and any other components currently hardwired to Direct3D objects to call into `IRenderBackend` (now powered by `BgfxRenderBackend`).
- Remove direct usage of `IDirect3DDevice8`, `IDirect3DTexture8`, etc., replacing them with backend-neutral abstractions or bgfx handles.
- Ensure lifetime management responsibilities (creation, reference counting, destruction) align with bgfx expectations.
- Adjust shader compilation/loading paths to output bgfx-compatible binaries or use bgfx shader tools.
- Incrementally build and test after each subsystem migration to catch regressions early.

### Step 5 – Remove or isolate Direct3D-specific code
- Search for remaining inclusions of `<windows.h>`, `d3d8.h`, `d3dx8.h`, or other Direct3D headers within the W3D device tree.
- Delete obsolete DX8-only files or wrap them with legacy build options if they must be retained temporarily.
- Update build scripts and project files to stop referencing Direct3D libraries.
- Clean up conditional compilation logic, leaving bgfx as the primary path.

### Step 6 – Validation and cleanup
- Run automated tests, rendering unit tests (if available), and in-game smoke tests to verify rendering correctness.
- Profile performance to ensure the bgfx path meets or exceeds the legacy implementation.
- Update documentation, developer onboarding guides, and any tooling scripts to reflect the new rendering backend.
- Perform a final codebase sweep for TODOs or transitional comments and resolve them.
- Prepare release notes summarizing the migration for the team.

## Incremental Delivery Strategy
- Treat each step as a separate development branch or PR (where feasible) to keep changes reviewable.
- Maintain backward compatibility checkpoints by keeping the build green after every major change.
- Coordinate with QA for targeted validation after Steps 3 and 4 when major rendering functionality changes.

## Open Questions / Follow-Up Items
- Determine whether platform-specific integrations (e.g., windowing, input capture) need additional abstraction for bgfx.
- Decide how to handle any Direct3D-specific shader code or preprocessor defines that have no bgfx equivalent.
- Evaluate tooling for shader conversion (bgfx shader compiler, material pipelines) and integrate into the build.
- Assess resource lifetime threading model differences between Direct3D and bgfx and update engine code accordingly.
