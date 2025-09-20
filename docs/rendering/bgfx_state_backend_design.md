# Designing bgfx Equivalents for the WW3D2 State Cache

## Purpose and scope

The Direct3D 8 wrapper keeps an extensive cache of render state, bound resources, and
helper objects so callers interact with a stable interface while the backend pushes
actual API calls. To bring the bgfx backend to feature parity, we need native
implementations for each cached bucket and helper that currently expect D3D objects.
This note maps the existing expectations—rooted in `BgfxStateData`, the buffer helper
classes, and the texture workflows—to a bgfx-friendly design that can slot behind the
`GraphicsBackend` switch without changing higher-level game code.【F:Generals/Code/Main/GraphicsBackend.h†L20-L36】【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h†L163-L227】

## Integration strategy behind `GraphicsBackend`

* Introduce a `BgfxBackend` facade that implements the handful of device-level
  hooks `DX8Wrapper` exposes (initialization, reset, resource factory helpers,
  render-state setters, draw submission). `DX8Wrapper::Set_Active_Backend` and
  `DX8Wrapper::Is_Bgfx_Active` already gate behaviour behind
  `GraphicsBackend`; the new facade plugs into those switches so callers keep
  using the familiar wrapper entry points while the bgfx-specific work happens
  behind the scenes.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp†L4394-L4434】【F:Generals/Code/Main/GraphicsBackend.h†L24-L36】
* Gate the legacy Direct3D 8 path behind a `WW3D_ENABLE_LEGACY_DX8` build flag so
  shipping builds default to bgfx and any Direct3D requests downgrade to bgfx
  when the legacy renderer is disabled.【F:Generals/Code/Main/GraphicsBackend.h†L20-L36】【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp†L181-L214】【F:Generals/Code/Main/WinMain.cpp†L1119-L1155】
* Reuse `BgfxStateData` as the authoritative mirror of fixed-function state but
  move the logic that currently lives inline in `DX8Wrapper` into dedicated
  cache helpers owned by the facade. This isolates bgfx bookkeeping from the
  legacy D3D arrays (`RenderStates`/`TextureStageStates`) and lets the wrapper
  delegate to bgfx through a clean interface that mirrors the Direct3D API
  concepts without touching the D3D device directly.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp†L526-L639】
* Maintain feature detection and fallback paths that still consult DX8 caps for
  now so the wider engine can continue to query capabilities until those call
  sites are redirected to bgfx-oriented probes. Capability queries can be
  routed through the facade as read-only helpers once the DX8 implementation no
  longer needs to respond.

The result is a layered design: engine code continues to call `DX8Wrapper`, the
wrapper switches on `GraphicsBackend`, and the bgfx branch forwards to a
cohesive set of bgfx helpers. When the Direct3D 8 implementation is removed the
facade can replace the wrapper entirely without the rest of the renderer
noticing.

## Render-state bucket design

### Pipeline state and transforms

`BgfxStateData` already aggregates the render-state flags, matrices, and alpha/fog
configuration the game toggles today. We will formalize a `BgfxStateCache` helper that
accepts the same setter calls `DX8Wrapper` exposes (`Set_DX8_Render_State`,
`Set_World_Transform`, etc.) and translates the legacy values into bgfx equivalents via
utility functions like `Convert_D3D_Blend_Factor_To_Bgfx` and
`Convert_D3D_Depth_Compare_To_Bgfx`. The cache is responsible for building the final
`stateFlags` bitfield, guarding redundant updates, and pushing matrices through
`bgfx::setTransform` when the frame dispatcher deems them dirty.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h†L165-L227】【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h†L229-L320】

The helper exposes a set of thin adapters that line up with the existing
wrapper signatures so the bridge logic inside `DX8Wrapper` simply forwards to
the cache when `GRAPHICS_BACKEND_BGFX` is active:

| Wrapper call | Bgfx cache responsibility |
| --- | --- |
| `Set_DX8_Render_State(state, value)` | Update `stateFlags`, blend/depth/stencil fields via the converter helpers, mark dirty bits |
| `Set_World_Transform(matrix)` / `Set_View_Transform(matrix)` / `Set_Projection_Transform(matrix)` | Copy matrices into `BgfxStateData`, mark valid flags, and note whether combined view-projection must be recomputed |
| `Set_Material(material)` | Translate material structure into uniform payload and cache it until submission |
| `Set_Fog_State(param, value)` | Convert to fixed-function fog uniforms and update `stateFlags` |

During draw submission the facade will invoke `BgfxStateCache::Apply`, which
submits transforms and uniforms, then returns the prepared `stateFlags` and
`BgfxProgramHandle` for the call to `bgfx::submit`.

Matrix setters mark `worldTransformValid`/`viewTransformValid`/`projectionTransformValid`
so we can lazily submit them at draw time. The implementation should coalesce world and
view-projection updates into a single `bgfx::setTransform` call per draw, mirroring the
existing `Apply` semantics.

### Material, lighting, and fog state

`BgfxStateData` tracks material colors, shininess, light enable flags, ambient
sources, fog parameters, and stencil configuration. The bgfx backend should continue to
populate the material uniforms in `SubmitBgfxMaterialUniforms` and the fixed-function
uniforms in `SubmitBgfxFixedFunctionUniforms`, but move responsibility for computing the
uniform payloads into the new cache helper. That helper will:

* Update `materialLightingEnabled`, `ambientSource`, `diffuseSource`, etc., while
  keeping the legacy source enums so shader code can interpret them consistently.
* Derive fog density/curve parameters and alpha-test thresholds from cached DX8-style
  values before enqueuing them via `bgfx::setUniform`.
* Convert stencil state through `UpdateBgfxStencilState`, guaranteeing that
  depth/stencil combinations match the expectations of the fixed-function shader.

These duties ensure draw calls observe the same lighting/fog rules regardless of the
selected backend.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp†L1258-L1339】【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h†L190-L221】

### Texture stage cache and sampler synthesis

Each texture stage maintains bindings, enable flags, UV sources, addressing modes, and
filtering preferences. `BgfxStateData` mirrors these fields, and helper functions like
`UpdateBgfxSamplerFlagsForStage` convert them into bgfx sampler bits. The new design
promotes this into a `BgfxSamplerCache` component that:

* Updates sampler flags whenever a texture, addressing mode, or filter changes,
  reusing `ComputeBgfxSamplerFlags` to pick defaults from the `TextureClass` and then
  overriding them with stage overrides (wrap/clamp, point/linear).
* Ensures uniform handles for sampler and transform matrices are lazily created via
  the existing `EnsureBgfxTextureSamplerDefaults`/`EnsureBgfxTextureMatrixDefaults`
  paths, but owned by the bgfx backend so the DX8 arrays can be retired.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp†L570-L624】【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp†L683-L700】
* Emits texture-transform matrices and UV source indices through
  `SubmitBgfxFixedFunctionUniforms` and binds actual textures with `bgfx::setTexture`
  during draw submission.

To keep the delegation boundary crisp, the facade maps each wrapper-stage call
into sampler-cache verbs:

| Wrapper call | Sampler cache work |
| --- | --- |
| `Set_Texture(stage, texture)` | Store `TextureClass*`, compute sampler flags, mark stage dirty |
| `Set_Texture_State(stage, state, value)` | Apply overrides for addressing/filtering, recompute sampler flags |
| `Set_Texture_Transform(stage, matrix)` / `Enable_Texture_Transform(stage, enabled)` | Update matrix cache, toggle `textureTransformUsed`, mark uniform dirty |

During submission the cache iterates over dirty stages, binds textures via
`bgfx::setTexture(stage, handle, textureHandle, samplerFlags)`, and emits the
matching transform uniforms. This keeps `DX8Wrapper` oblivious to bgfx binding
semantics.

## Geometry buffer helper design

### Static buffers

`DX8VertexBufferClass` and `DX8IndexBufferClass` currently wrap both the D3D handle and
optional bgfx handles. Our bgfx implementation will extend the helper data stored inside
these classes (or an adjacent companion) with immutable bgfx handles created on-demand.
When `Set_Vertex_Buffer` or `Set_Index_Buffer` assigns a buffer, the backend records the
buffer type (`BUFFER_TYPE_*`) and offsets so it can bind the correct bgfx handle during
`Apply`. Static buffers simply call `bgfx::setVertexBuffer` / `bgfx::setIndexBuffer` with
cached handles and draw ranges computed from the D3D-style offsets.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8vertexbuffer.h†L90-L131】【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp†L2820-L2870】

To minimize churn in the wrapper, the facade will offer a narrow set of
buffer-management entry points that mirror the Direct3D-era responsibilities:

| Wrapper call | Bgfx buffer helper |
| --- | --- |
| `_Create_DX8_Vertex_Buffer(desc, initial_data)` | Allocate bgfx static buffer, store handle on `DX8VertexBufferClass` |
| `_Create_DX8_Index_Buffer(desc, initial_data)` | Same for index buffers |
| `_Lock_Dynamic_Vertex_Buffer(access)` / `_Unlock_Dynamic_Vertex_Buffer(access)` | Map/unmap ring buffer staging area, schedule bgfx updates |
| `Set_Vertex_Buffer(buffer, base, stride)` | Remember current handle/offset to bind in `Apply` |
| `Set_Index_Buffer(buffer, base)` | Same for indices |

`BgfxBackend::ApplyDrawBuffers` uses the stored metadata to bind vertex and
index buffers with the correct ranges before calling `bgfx::submit`.

### Dynamic ring buffers

`DynamicVBAccessClass` and `DynamicIBAccessClass` wrap ring buffers that hand out a
contiguous region per draw, filling D3D dynamic buffers and tracking offsets for index
remapping. The bgfx design keeps these semantics by allocating matching bgfx dynamic
buffers (or transient buffers once available) when the access object is created. Each
write lock copies the caller’s data into both the D3D memory (for legacy draws) and the
bgfx dynamic buffer. During submission the backend branches on the buffer type to select
the dynamic handle (`Uses_Bgfx_Dynamic_Buffer`) and binds the offset/count recorded in
`render_state`. This mirrors the existing bgfx submission path inside
`Draw_Sorting_IB_VB` and keeps both backends working until DX8 is removed.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8vertexbuffer.h†L135-L207】【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8indexbuffer.h†L71-L168】【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp†L2878-L2937】【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp†L3031-L3140】

### Resource lifetime and fallback behavior

Both buffer helpers increment/decrement engine references and recreate buffers when
allocation fails. The bgfx backend should centralize bgfx handle creation and release in
matching helper structures (e.g., `BgfxVertexBufferData`) so failure paths like the
vertex buffer flush in `_Create_DX8_Vertex_Buffer` can also destroy bgfx resources before
retrying. This ensures reset and reclamation flows treat bgfx and D3D resources
uniformly.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp†L2820-L2937】

## Texture creation, binding, and updates

Texture helpers create D3D textures from blank allocations, files, or surfaces, and then
mirror them into bgfx textures when the bgfx path is active. To remove D3D dependencies,
we will:

* Promote `Create_Bgfx_Texture_For_Owner` into the primary creation routine so render
  targets and static textures generate bgfx handles first, falling back to D3D only when
  the DX8 backend is active. The helper already derives format, mip-count, and render
  target flags from the original request, and uploads via `Upload_Bgfx_Texture` when no
  D3D texture exists.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp†L3031-L3099】【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp†L3594-L3785】
* Encapsulate stage binding inside the sampler cache so `Set_Texture` simply records the
  `TextureClass*` and lets bgfx bind the handle during draw calls, mirroring the `textureBindings`
  bookkeeping in `BgfxStateData`.
* Mirror system-memory to video-memory updates in `_Update_Texture` by invoking the
  equivalent bgfx texture update path (e.g., `bgfx::updateTexture2D`) using the handles
  cached on the `TextureClass`.

These changes decouple resource creation from DX8 while maintaining compatibility with
existing `TextureClass` ownership rules and fallback textures.

## Frame lifecycle and program selection

`BgfxStateData` stores the currently selected program handle, defaulting to the fixed
function program returned by `Get_Default_Bgfx_Program`. The new backend should manage
program selection through shader-specific state objects so that material overrides or
future programmable shaders can swap programs while still using the same cached state.
During `BeginScene`/`EndScene` equivalents the backend resets dirty flags, ensures
sampler uniforms are initialized, and queues `bgfx::submit` calls with the prepared
state bitfield and program handle.【F:Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp†L1235-L1300】

## Deliverables and remaining objectives

Completing this design satisfies objective (1) from the migration plan: we now have a
bgfx-native blueprint for every cached state, buffer helper, and texture workflow. The
remaining objectives are:

1. Implement the bgfx execution path using the above caches, covering draw submission,
   resource upload, and device reset/recovery flows.
2. Retire or gate the legacy Direct3D 8 paths and validate rendering output across both
   Generals variants once the bgfx backend achieves feature parity.【F:docs/rendering/dx8_usage_summary.md†L45-L55】
