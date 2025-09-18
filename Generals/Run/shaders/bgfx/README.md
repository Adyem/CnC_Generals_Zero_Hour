# BGFX shader sources

This directory contains the reference sources used by the runtime shader compilation
path inside `W3DShaderManager`. Each shader has an associated `.sc` file for the vertex
and fragment stages as well as a shared `varying.def.sc` declaration. The shader manager
looks for compiled binaries in `shaders/bgfx/*.bin` at runtime and will invoke
`shaderc` to rebuild them when the source timestamp is newer than the binary.

The `BgfxProgramDefinition` metadata registered from C++ describes the uniform and
sampler layout exposed by each shader program. These definitions act as the contract for
future work that will bind BGFX uniforms instead of legacy Direct3D texture stage state.

## Uniform conventions

* `u_modelViewProj` – world/view/projection transform for the current draw.
* `u_texTransform` – array of up to four texture projection matrices used to replicate
  the legacy camera-space projections for cloud/noise passes.
* `u_layerConfig` – general purpose parameters for terrain/road layering. Components
  map to detail and noise blend factors.
* `u_fadeConfig`, `u_shroudParams`, `u_maskParams`, `u_cloudParams` – shader specific
  parameters that will be populated by the shader manager when the bgfx path is wired
  up to the game logic.

All shader sources rely on standard BGFX includes and assume the vertex format exposes
`a_position`, `a_color0`, and at least two sets of texture coordinates. The extra
varyings are written even when unused so the compiled programs have a uniform interface
across the different terrain permutations.
