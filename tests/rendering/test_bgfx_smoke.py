"""Smoke-test scaffolding for validating bgfx rendering output.

This suite keeps lightweight checks that ensure the bgfx validation manifest is
well-formed and, when captures are available, it compares them against the
checked-in reference images.
"""

from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import unittest
from typing import Iterable, Mapping, Any


BASE_DIR = Path(__file__).resolve().parent
MANIFEST_PATH = BASE_DIR / "bgfx_smoke_manifest.json"
REFERENCE_DIR = BASE_DIR / "reference"

_CAPTURE_ENV = os.environ.get("BGFX_SMOKE_CAPTURE_DIR")
CAPTURE_DIR = Path(_CAPTURE_ENV).expanduser() if _CAPTURE_ENV else None


def _load_manifest() -> list[Mapping[str, Any]]:
    with MANIFEST_PATH.open("r", encoding="utf-8") as handle:
        manifest = json.load(handle)
    if not isinstance(manifest, list):
        raise TypeError("Manifest must be a list of scene entries")
    return manifest


def _required_fields() -> Iterable[str]:
    return ("scene", "camera", "reference_image")


class BgfxSmokeManifestTests(unittest.TestCase):
    """Structural checks that do not require running the game."""

    @classmethod
    def setUpClass(cls) -> None:
        cls.manifest = _load_manifest()

    def test_manifest_contains_at_least_one_scene(self) -> None:
        self.assertGreater(len(self.manifest), 0, "Add at least one scene to validate.")

    def test_required_fields_present(self) -> None:
        for entry in self.manifest:
            for field in _required_fields():
                self.assertIn(field, entry, f"Missing '{field}' in manifest entry: {entry}")

    def test_reference_images_exist(self) -> None:
        for entry in self.manifest:
            reference_path = REFERENCE_DIR / entry["reference_image"]
            self.assertTrue(reference_path.is_file(), f"Reference image not found: {reference_path}")


class BgfxSmokeCaptureComparisonTests(unittest.TestCase):
    """Optional comparisons against locally captured frames."""

    @classmethod
    def setUpClass(cls) -> None:
        cls.manifest = _load_manifest()

    @unittest.skipIf(CAPTURE_DIR is None, "Set BGFX_SMOKE_CAPTURE_DIR to compare captures against references.")
    def test_captures_match_reference_images(self) -> None:
        self.assertTrue(CAPTURE_DIR.is_dir(), f"Capture directory does not exist: {CAPTURE_DIR}")
        for entry in self.manifest:
            reference_path = REFERENCE_DIR / entry["reference_image"]
            capture_name = entry.get("capture_image", entry["reference_image"])
            capture_path = CAPTURE_DIR / capture_name
            self.assertTrue(capture_path.is_file(), f"Missing capture for {entry['scene']}: {capture_path}")
            reference_hash = hashlib.sha256(reference_path.read_bytes()).hexdigest()
            capture_hash = hashlib.sha256(capture_path.read_bytes()).hexdigest()
            self.assertEqual(
                reference_hash,
                capture_hash,
                f"Capture mismatch for {entry['scene']} ({capture_path} vs {reference_path})."
                " Regenerate the reference or investigate the rendering regression.",
            )


if __name__ == "__main__":  # pragma: no cover
    unittest.main()
