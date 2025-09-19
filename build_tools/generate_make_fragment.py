#!/usr/bin/env python3
"""Generate GNU Make fragment listing sources and per-file build rules."""
from __future__ import annotations

import argparse
import hashlib
import pathlib
from typing import List, Tuple


def sanitize_path(path: pathlib.Path) -> str:
    """Replace spaces with underscores in a filesystem path."""
    return str(path).replace(" ", "_")


def escape_dependency(path: pathlib.Path) -> str:
    """Escape spaces in dependency paths for inclusion in Makefiles."""
    return str(path).replace(" ", "\\ ")


CPP_EXTENSIONS = {
    ".cpp",
    ".cxx",
    ".cc",
    ".c++",
    ".cp",
}

C_EXTENSIONS = {".c"}


def is_cpp_source(path: pathlib.Path) -> bool:
    """Return True if *path* should be compiled as C++."""

    suffix = path.suffix
    lower = suffix.lower()
    if lower in CPP_EXTENSIONS:
        return True

    # Some legacy sources use an upper-case ".C" extension to denote C++
    # files.  Treat those as C++ as well so they are routed through the
    # C++ compiler instead of the C compiler.
    return suffix == ".C"


def is_c_source(path: pathlib.Path) -> bool:
    """Return True if *path* should be compiled as C."""

    suffix = path.suffix
    lower = suffix.lower()
    if lower in C_EXTENSIONS:
        # Avoid mis-classifying upper-case ".C" files as C sources; those are
        # handled by ``is_cpp_source`` above.
        return suffix != ".C"
    return False


def is_excluded(path: pathlib.Path, src_dir: pathlib.Path, excludes: List[pathlib.Path]) -> bool:
    rel_parts = path.relative_to(src_dir).parts
    for excl in excludes:
        excl_parts = excl.parts
        if rel_parts[:len(excl_parts)] == excl_parts:
            return True
    return False


def discover_sources(
    src_dir: pathlib.Path, excludes: List[pathlib.Path]
) -> Tuple[List[pathlib.Path], List[pathlib.Path]]:
    cpp_files: List[pathlib.Path] = []
    c_files: List[pathlib.Path] = []

    for path in sorted(src_dir.rglob("*")):
        if not path.is_file():
            continue

        if excludes and is_excluded(path, src_dir, excludes):
            continue

        if is_cpp_source(path):
            cpp_files.append(path)
        elif is_c_source(path):
            c_files.append(path)

    return cpp_files, c_files


def register_object(
    path: pathlib.Path,
    src_dir: pathlib.Path,
    obj_dir: pathlib.Path,
    compiler: str,
    seen: dict,
    objects: List[str],
    entries: List[Tuple[str, str, str]],
) -> None:
    rel = path.relative_to(src_dir)
    base = obj_dir / rel.with_suffix("")
    sanitized = sanitize_path(base)
    unique = sanitized
    if unique in seen:
        digest = hashlib.md5(str(path).encode("utf-8")).hexdigest()[:6]
        unique = f"{sanitized}__{digest}"
    seen[unique] = str(path)
    obj_name = f"{unique}.o"
    dep = escape_dependency(path)
    objects.append(obj_name)
    entries.append((obj_name, dep, compiler))


def write_fragment(
    out_path: pathlib.Path,
    cpp_sources: List[pathlib.Path],
    c_sources: List[pathlib.Path],
    src_dir: pathlib.Path,
    obj_dir: pathlib.Path,
    variable_prefix: str,
) -> None:
    objects: List[str] = []
    entries: List[Tuple[str, str, str]] = []
    seen: dict = {}

    for path in cpp_sources:
        register_object(path, src_dir, obj_dir, "cxx", seen, objects, entries)
    for path in c_sources:
        register_object(path, src_dir, obj_dir, "c", seen, objects, entries)

    prefix = f"{variable_prefix}_" if variable_prefix else ""

    cpp_var = f"{prefix}CPP_SOURCE_COUNT"
    c_var = f"{prefix}C_SOURCE_COUNT"
    objects_var = f"{prefix}OBJECTS"

    with out_path.open("w", encoding="utf-8", newline="\n") as fh:
        fh.write(f"{cpp_var} := {len(cpp_sources)}\n")
        fh.write(f"{c_var} := {len(c_sources)}\n")
        if objects:
            fh.write(f"{objects_var} := \\\n")
            for index, obj in enumerate(objects):
                suffix = " \\\n" if index < len(objects) - 1 else "\n"
                fh.write(f"  {obj}{suffix}")
        else:
            fh.write(f"{objects_var} :=\n")
        for obj_name, dep, compiler in entries:
            fh.write(f"{obj_name}: {dep}\n")
            fh.write("\t@mkdir -p $(@D)\n")
            if compiler == "cxx":
                fh.write('\t$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c "$<" -o "$@"\n')
            else:
                fh.write('\t$(CC) $(CPPFLAGS) $(CFLAGS) -c "$<" -o "$@"\n')


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("src_dir", type=pathlib.Path)
    parser.add_argument("obj_dir", type=pathlib.Path)
    parser.add_argument("output", type=pathlib.Path)
    parser.add_argument(
        "--variable-prefix",
        dest="variable_prefix",
        default="",
        help="Prefix to apply to generated variable names",
    )
    parser.add_argument(
        "--exclude",
        dest="excludes",
        action="append",
        default=[],
        help="Relative paths under src_dir that should be excluded",
    )
    args = parser.parse_args()

    exclude_paths = [pathlib.Path(item) for item in args.excludes]
    cpp_sources, c_sources = discover_sources(args.src_dir, exclude_paths)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    write_fragment(
        args.output,
        cpp_sources,
        c_sources,
        args.src_dir,
        args.obj_dir,
        args.variable_prefix,
    )


if __name__ == "__main__":
    main()
