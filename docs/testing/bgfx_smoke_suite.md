# bgfx smoke-test suite

This suite provides the first automated checks for verifying that the bgfx backend
renders representative scenes consistently. It lives under
`tests/rendering/test_bgfx_smoke.py` and is designed to work even when the Generals
executables are unavailable: structural checks always run, while image comparisons
activate once you capture fresh frames locally.

## Files

| Path | Purpose |
| --- | --- |
| `tests/rendering/bgfx_smoke_manifest.json` | Lists the scenes, cameras, and reference screenshots under test. |
| `tests/rendering/reference/` | Stores the reference PNGs generated from known-good bgfx captures. |
| `tests/rendering/test_bgfx_smoke.py` | Loads the manifest, checks structure, and optionally compares captured images to the references. |

## Capturing frames

1. Launch a build with the bgfx backend active (the Direct3D 8 path can remain gated
   off).
2. Load each scene listed in the manifest and capture a screenshot that matches the
   described camera framing. Save the PNGs in a temporary directory.
3. Set `BGFX_SMOKE_CAPTURE_DIR` to the directory containing the freshly captured PNGs.

   ```bash
   export BGFX_SMOKE_CAPTURE_DIR="/path/to/captures"
   ```

4. Run the smoke-test suite:

   ```bash
   python -m unittest tests.rendering.test_bgfx_smoke
   ```

5. If the comparison test fails, inspect the diff. When the new capture is correct,
   replace the corresponding file under `tests/rendering/reference/` and commit the
   update alongside any rendering changes.

## Updating the manifest

Add new entries to the manifest as we broaden coverage. Each entry must include:

* `scene` – An identifier describing the map or mission.
* `camera` – The camera preset or framing instructions needed to reproduce the shot.
* `reference_image` – The filename of the committed PNG under `reference/`.
* Optional metadata such as `reference_description` or `notes` to give future
  maintainers enough context to recapture the scene.

When you extend the manifest, also drop the new PNG into the reference directory and
re-run the tests to ensure the structural checks continue to pass.
