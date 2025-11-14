#!/usr/bin/env bash
set -euo pipefail

# Simple, portable build script for a clean checkout
# Usage:
#   ./scripts/build.sh           # Configure + build (Release)
#   ./scripts/build.sh debug     # Configure + build (Debug)
#   ./scripts/build.sh clean     # Clean target in existing build
#   ./scripts/build.sh distclean # Remove all build artifacts

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
root="${here}/.."
build_dir="${root}/build"

action="build"
build_type="Release"

if [[ ${1:-} == "clean" ]]; then
  action="clean"
elif [[ ${1:-} == "distclean" ]]; then
  action="distclean"
elif [[ ${1:-} =~ ^(debug|Debug)$ ]]; then
  build_type="Debug"
elif [[ ${1:-} =~ ^(release|Release)$ ]]; then
  build_type="Release"
elif [[ $# -gt 0 ]]; then
  echo "Unknown argument: $1" >&2
  echo "Usage: $0 [debug|release|clean|distclean]" >&2
  exit 2
fi

if [[ "$action" == "clean" ]]; then
  if [[ -d "$build_dir" ]]; then
    cmake --build "$build_dir" --target clean || true
    echo "Clean complete in $build_dir"
  else
    echo "Nothing to clean (no build dir)"
  fi
  exit 0
fi

if [[ "$action" == "distclean" ]]; then
  if [[ -d "$build_dir" ]]; then
    # Try the deep clean target if available, then remove the folder
    cmake --build "$build_dir" --target clean-all >/dev/null 2>&1 || true
    rm -rf "$build_dir"
    echo "Distclean complete (removed $build_dir)"
  else
    echo "Nothing to distclean (no build dir)"
  fi
  exit 0
fi

echo "Build type: $build_type"
mkdir -p "$build_dir"

# Help CMake find OpenSSL on macOS/Homebrew if available
openssl_args=()
if command -v brew >/dev/null 2>&1; then
  if prefix=$(brew --prefix openssl@3 2>/dev/null); then
    openssl_args+=("-DOPENSSL_ROOT_DIR=$prefix")
  elif prefix=$(brew --prefix openssl 2>/dev/null); then
    openssl_args+=("-DOPENSSL_ROOT_DIR=$prefix")
  fi
fi

cmake -S "$root" -B "$build_dir" -DCMAKE_BUILD_TYPE="$build_type" "${openssl_args[@]:-}"
cmake --build "$build_dir" -j

bin_path="$build_dir/stegobmp"
if [[ -x "$bin_path" ]]; then
  echo "Build successful: $bin_path"
else
  echo "Build finished. Executable should be under $build_dir (generator-dependent)."
fi
