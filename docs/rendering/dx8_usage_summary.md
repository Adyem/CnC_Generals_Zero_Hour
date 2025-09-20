# Direct3D 8 Usage Inventory

This note captures the Direct3D 8 features that the legacy renderer touches so we can design equivalent bgfx plumbing behind `GraphicsBackend`. The focus is on state buckets, resource lifetimes, and device entry points exercised in the `WW3D2` library.

## Render-state coverage

* **Tracked render-state cache.** `BgfxStateData` mirrors every render-state we mutate—depth/stencil flags, alpha test configuration, fog, lighting material sources, and rasterization modes—so translating the cache requires handling the full struct when submitting to bgfx.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h†L163-L227】【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h†L1067-L1298】
* **Default initialization.** The DX8 bootstrap writes a baseline for fog, lighting, fill, stencil, and bump-mapping stage parameters. Any bgfx path must reproduce these defaults (or intentionally diverge) before the first frame.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp†L526-L638】
* **Runtime mutations.** Mesh submission toggles alpha blending, normal normalization, and Z bias on demand, so the bgfx backend needs equivalent toggles when we map `Set_DX8_Render_State` to bgfx state changes.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8renderer.cpp†L1660-L1882】【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8renderer.cpp†L2120-L2132】

## Texture stages and samplers

* **Per-stage bookkeeping.** Texture bindings, UV sources, addressing modes, filters, transform matrices, and enable flags are cached per stage; converting to bgfx requires mirroring these stage fields and rebuilding sampler state from them.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h†L165-L227】【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h†L1310-L1378】
* **Global defaults.** Startup seeds bump-env matrix values and linear filtering for both stages, implying the bgfx backend must seed identical sampler defaults before assets stream in.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp†L563-L622】

## Geometry resources

* **Vertex buffers.** Creation paths fall back after `CreateVertexBuffer` failure by flushing the resource manager and recreating, and both lock/unlock paths branch between DX8 and bgfx storage. Resource translations must respect usage bits, dynamic updates, and the fallback discard bytes call.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8vertexbuffer.cpp†L560-L676】
* **Index buffers.** Similar dual-path logic exists for index buffers: dynamic vs. static storage, bgfx handles, and lock translation. Any backend swap must offer matching upload semantics.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8indexbuffer.cpp†L320-L420】
* **Binding flow.** Render-state application binds vertex and index buffers, accounting for dynamic offsets and resetting references each frame end. The bgfx path already stubs dynamic binding but still needs parity with the DX8 code path for every buffer type.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp†L2820-L3550】

## Texture and surface management

* **Texture creation.** Helper overloads wrap `D3DXCreateTexture` and `D3DXCreateTextureFromFileExA`, handle missing formats, and copy surfaces to textures; bgfx integration currently piggybacks on these helpers but needs a complete path when DX8 allocation fails (e.g., render targets).【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp†L3594-L3785】
* **Surface creation and loading.** `_Create_DX8_Surface` and the file-loading variant rely on `CreateImageSurface`/`TextureLoader`, and optionally stamp bgfx surfaces. Replacements must preserve this loading flow and error handling.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp†L3789-L3840】
* **Texture updates.** System-memory textures are copied into default-pool textures with `UpdateTexture`, which the bgfx backend will need to emulate for dynamic uploads.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp†L3868-L3875】

## Device interaction points

* **Frame lifecycle.** `BeginScene`/`EndScene`, `Present`, cooperative-level tests, and the bgfx fallback frame submission all hang off `DX8Wrapper`; bgfx must plug into the same frame boundaries and reset flow.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp†L2619-L2696】
* **Viewport and clearing.** `SetViewport` and `Clear` translate to bgfx calls when active, otherwise issue the D3D calls. Any new backend must cover viewport rectangles, depth/stencil masks, and clear color encoding.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp†L2790-L2817】
* **Render targets and swap chains.** Render-target creation, texture-backed targets, swap-chain binding, and depth-stencil preservation all flow through wrapper helpers that currently assume DX8 surfaces. A bgfx solution must expose equivalent framebuffer selection semantics.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp†L4051-L4305】【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp†L4240-L4320】
* **Device reset/recovery.** `Reset_Device` releases dynamic resources, tests cooperative level, and rehydrates textures/materials—an essential checklist for recreating bgfx resources on window-size changes or device loss.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp†L1523-L1555】

## Lighting and material handling

* **Material cache.** `Set_Material` hooks into the render-state cache and drives both DX8 and bgfx material uniforms; overriding opacity/UVs in the renderer relies on this cache being authoritative.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8renderer.cpp†L1653-L1876】【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h†L1638-L1656】
* **Lights and environments.** Light setup covers individual lights, environment-driven ambient updates, and toggling DX8 lights on/off. Mapping to bgfx will require translating `Set_Light`/`Set_Light_Environment` behaviour and ambient state writes.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp†L3868-L4010】

## Capability queries

* **Hardware caps.** `dx8caps.cpp` probes hardware for vertex processing, DOT3, DXT compression, bump mapping, shader versions, gamma, and vendor quirks. The bgfx backend still needs analogous capability gating to keep old feature checks functioning.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8caps.cpp†L42-L142】【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8caps.cpp†L150-L204】

## Outstanding DX8 touchpoints during rendering

* **State changes per mesh.** The mesh renderer adjusts blend state, alpha test references, world transforms, and normalization around each draw call. Bgfx command submission has to accept the same per-mesh churn, including staging of deferred (sorted) draw calls.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8renderer.cpp†L1660-L1889】
* **Special cases.** Decal rendering tweaks Z-bias, and other flows insert clip planes or additional states; these inflection points signal where bgfx equivalents are still missing.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8renderer.cpp†L2120-L2132】【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h†L1300-L1378】

## Status and next steps

* **Inventory complete.** This document continues to track every Direct3D 8 touchpoint that the bgfx backend must mirror.
* **Design captured.** `docs/rendering/bgfx_state_backend_design.md` now documents the cache helpers and draw path required to serve the existing `GraphicsBackend` callers.
* **Implementation integrated.** The bgfx backend is wired through `DX8Wrapper`, resource helpers, and the `WW3D_ENABLE_LEGACY_DX8` build toggle so both Generals executables prefer bgfx at runtime.
* **Validation pending.** The outstanding work item is exercising representative scenes with the bgfx backend and recording the results. Start with the smoke-test harness under `tests/rendering/test_bgfx_smoke.py`, capture frames into `BGFX_SMOKE_CAPTURE_DIR`, and compare them against the reference images committed alongside the manifest.
