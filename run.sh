#!/usr/bin/env bash
# run.sh - Build and run Cali (CMake, D3D11 on Windows / OGL elsewhere)
# Usage:
#   ./run.sh              # dev (Debug) build + run
#   ./run.sh dev          # same
#   ./run.sh release      # Release build + run
#   ./run.sh dev --clean  # clean + build + run
#   ./run.sh dev --no-build  # just run
#   ./run.sh release --test  # build + run tests + run app
#   ./run.sh --help
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
CONFIG="dev"
CLEAN=0
NOBUILD=0
NORUN=0
RUN_TESTS=0

for arg in "$@"; do
  case "$arg" in
    dev|debug|Debug) CONFIG="dev" ;;
    release|Release|rel) CONFIG="release" ;;
    --clean|-c) CLEAN=1 ;;
    --no-build) NOBUILD=1 ;;
    --no-run) NORUN=1 ;;
    --test) RUN_TESTS=1 ;;
    --help|-h)
      echo "Usage: $0 [dev|release] [--clean] [--no-build] [--no-run] [--test]"
      echo "  dev (default) -> Debug, release -> Release"
      echo "  --test  run ctest after build (off by default)"
      exit 0
      ;;
    *) echo "Unknown arg: $arg"; exit 1 ;;
  esac
done

if [[ "$CONFIG" == "dev" ]]; then
  CMAKE_CONFIG="Debug"
  PRESET="windows-x64-debug"
else
  CMAKE_CONFIG="Release"
  PRESET="windows-x64-release"
fi

BUILD_DIR="$ROOT/build"
# VS multi-config: build/bin/<Config>/cali.exe (libs in build/lib/<Config>)
EXE="$BUILD_DIR/bin/$CMAKE_CONFIG/cali.exe"
if [[ "$OSTYPE" != "msys"* && "$OSTYPE" != "cygwin"* && "$OSTYPE" != "win32"* ]]; then
  # Ninja single-config layout: build/bin/cali or build/cali
  if [[ -f "$BUILD_DIR/bin/cali" ]]; then EXE="$BUILD_DIR/bin/cali"; fi
  if [[ -f "$BUILD_DIR/cali" ]]; then EXE="$BUILD_DIR/cali"; fi
  if [[ -f "$ROOT/build-ninja-debug/cali" ]]; then BUILD_DIR="$ROOT/build-ninja-debug"; EXE="$BUILD_DIR/cali"; fi
  # OGL needs no DirectX
fi

echo "==> Cali run.sh [$CMAKE_CONFIG] preset=$PRESET"

if [[ $CLEAN -eq 1 ]]; then
  echo "Cleaning $BUILD_DIR ..."
  rm -rf "$BUILD_DIR"
  rm -rf "$ROOT/x64" 2>/dev/null || true
fi

if [[ $NOBUILD -eq 0 ]]; then
  if [[ ! -f "$BUILD_DIR/CMakeCache.txt" ]]; then
    echo "Configuring (cmake --preset $PRESET) ..."
    if ! cmake --preset "$PRESET" 2>/dev/null; then
      echo "Preset failed, trying manual configure..."
      cmake -S "$ROOT" -B "$BUILD_DIR" -G "Visual Studio 17 2022" -A x64 2>/dev/null \
        || cmake -S "$ROOT" -B "$BUILD_DIR" -G Ninja -DCMAKE_BUILD_TYPE="$CMAKE_CONFIG"
    fi
  else
    echo "Build dir already configured, skipping configure."
  fi

  echo "Building (cmake --build --config $CMAKE_CONFIG) ..."
  cmake --build "$BUILD_DIR" --config "$CMAKE_CONFIG" --parallel

  if [[ $RUN_TESTS -eq 1 ]]; then
    echo "Running tests (ctest -C $CMAKE_CONFIG) ..."
    ctest --test-dir "$BUILD_DIR" -C "$CMAKE_CONFIG" --output-on-failure
  fi
fi

if [[ $NORUN -eq 0 ]]; then
  if [[ ! -f "$EXE" ]]; then
    # fallbacks for old layout
    if [[ -f "$BUILD_DIR/$CMAKE_CONFIG/cali.exe" ]]; then EXE="$BUILD_DIR/$CMAKE_CONFIG/cali.exe"
    elif [[ -f "$BUILD_DIR/bin/cali" ]]; then EXE="$BUILD_DIR/bin/cali"
    elif [[ -f "$BUILD_DIR/cali" ]]; then EXE="$BUILD_DIR/cali"
    elif [[ -f "$BUILD_DIR/cali.exe" ]]; then EXE="$BUILD_DIR/cali.exe"
    elif [[ -f "$ROOT/src/cali/caliD3D11_d.exe" ]]; then EXE="$ROOT/src/cali/caliD3D11_d.exe"
    else echo "Executable not found at $EXE. Build first."; exit 1
    fi
  fi
  echo "Launching $EXE ..."
  # ensure shaders next to exe
  if [[ -d "$ROOT/src/cali/shaders" && ! -d "$(dirname "$EXE")/shaders" ]]; then
    cp -r "$ROOT/src/cali/shaders" "$(dirname "$EXE")/"
  fi
  if [[ -f "$ROOT/src/cali/courier_new.spritefont" ]]; then
    cp -f "$ROOT/src/cali/courier_new.spritefont" "$(dirname "$EXE")/" 2>/dev/null || true
  fi
  (cd "$(dirname "$EXE")" && "./$(basename "$EXE")")
fi
