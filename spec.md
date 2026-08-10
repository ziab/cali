# Cali — Project Spec

## Overview
Cali is a DirectX 11 demo for refreshing linear algebra and real-time atmospheric scattering. Two main references:
- *Essential Mathematics for Games* (`depends/essential_math` — IvMath/IvEngine/IvGraphics)
- *Precomputed Atmospheric Scattering* Bruneton ported from OpenGL to D3D11 (`src/cali/Bruneton.*`, `src/cali/shaders/bruneton_*.hlslf`)

Shows a procedural planet with quad-tree terrain (`TerrainQuad`/`TerrainQuadTree`), sky scattering, sun, stars, bloom post-effect. Window + input via `IvEngine` (Win32, D3D11). See `img/cali.gif`.

## Tech Stack
- **Language:** C++17 (`CMakeLists.txt:19`), MSVC 19.44 (VS2022, toolset `v143`)
- **Graphics:** D3D11 only (OGL path exists `IvGraphics/OGL` but off by default `CMakeLists.txt:8`). Link: `d3d11`, `d3dcompiler`, `dxgi`, `dxguid`, `comctl32`, `shlwapi`
- **Deps (vendored):**
  - `depends/essential_math/common` — IvMath, IvUtility, IvCollision, IvGraphics (D3D11/OGL), IvEngine (`IvEngine/D3D11/IvMainD3D11.cpp:1`)
  - `depends/DirectXTK-master` — SpriteBatch/Font, DDSTextureLoader, Effects etc. (`DirectXTK/Inc`, `DirectXTK/Src/pch.cpp:1`)
  - `depends/bitmap_image/bitmap_image.hpp:1` — heightmap loader (`CommonTexture.cpp:8`)
  - `depends/gtest` — bundled `gtest-all.cpp:1` for `cali_test`
- **Build:** CMake 3.20+, Visual Studio 17 2022 x64 (multi-config) or Ninja. Presets in `CMakePresets.json:1`. Legacy VS `.sln`/`.vcxproj` removed (`51e1714`).

## Directory Structure
```
cali/
├─ CMakeLists.txt              # top-level, single CMake project
├─ CMakePresets.json           # windows-x64-debug/release, ninja-debug
├─ run.ps1 / run.sh / run.cmd  # dev/release wrappers (see below)
├─ spec.md                     # this file
├─ src/cali/
│  ├─ Game.cpp:59              # IvGame::Create, PostRendererInitialize (bruneton, terrain, sky, stars, sun)
│  ├─ Bruneton.cpp:24          # initialize() + precompute() — 6 shader programs, throws on failure
│  ├─ TerrainQuad.cpp:132      # loads bitmaps/heightmap.bmp via CommonTexture, shader terrain_quad.hlslv/terrain.hlslf
│  ├─ TerrainQuadTree.h / Grid.* / Icosahedron.* / Terrain.* # LOD, frustum culling
│  ├─ Camera.* / InputController.* / Frustum.* / AABB.* / Box.* / Icosahedron.*
│  ├─ Sky.* / Sun.* / Stars.* / PostEffect.* / Model.* / Renderable.* / DebugInfo.*
│  ├─ CommonFileSystem.cpp:37  # get_executable_file_directory() + construct_shader_path()
│  ├─ CommonTexture.cpp:13     # load_texture_from_bmp()
│  ├─ ConstantBuffer*.h / Signal.h / World.h / Constants.h
│  ├─ shaders/                 # 26 .hlslv/.hlslf/.fx (bruneton_*, sky, terrain, bloom, draw_quad)
│  ├─ bitmaps/heightmap.bmp    # 6.9 MB heightmap (copied at build)
│  └─ courier_new.spritefont   # DirectXTK font
├─ src/cali_test/cali_test_main.cpp:42 # 3 TerrainQuadTree tests, int main return
├─ src/precompiled_depends/    # legacy, now unused (gtest built via CMake)
├─ depends/essential_math/common/Iv*  # see above
├─ depends/DirectXTK-master/Inc|Src
├─ build/                      # out-of-tree (ignored)
│  ├─ bin/Debug|Release/cali.exe + cali_test.exe + shaders/ + bitmaps/ + courier_new.spritefont
│  └─ lib/Debug|Release/*.lib + *.pdb (Iv*, DirectXTK, gtest_bundled)
└─ img/
```

## Build System (CMake)

**Top-level `CMakeLists.txt:1` is the single source of truth.** No `add_subdirectory` for deps — they are `add_library(STATIC)` directly.

- Output layout (`CMakeLists.txt:21`):
  ```cmake
  CMAKE_RUNTIME_OUTPUT_DIRECTORY = build/bin
  CMAKE_ARCHIVE_OUTPUT_DIRECTORY = build/lib
  # per-config: build/bin/Debug, build/lib/Release, etc.
  ```
  Before fix, libs were next to exe (`build/Release/*.lib`) — now separated.

- Targets:
  - `IvMath`, `IvUtility`, `IvCollision` (`IvMath:40`, `IvUtility:63`, `IvCollision:74`)
  - `IvGraphics` — `D3D11/` 13 files vs `OGL/` 10 files (`CMakeLists.txt:97`)
  - `IvEngine` — `IvMainD3D11.cpp` vs `IvMainOGL.cpp` (`CMakeLists.txt:168`)
  - `DirectXTK` — 32 files, `DISABLE_PRECOMPILE_HEADERS`, `_WIN32_WINNT=0x0600` (`CMakeLists.txt:190`)
  - `cali` — `src/cali/*.cpp` 20 files, `WIN32_EXECUTABLE`, `/ENTRY:wWinMainCRTStartup`, copies shaders/bitmaps/font via `POST_BUILD` (`CMakeLists.txt:300`)
  - `cali_test` — `gtest_bundled` + `cali_test_main.cpp` (`CMakeLists.txt:320`), `enable_testing()`

- Presets (`CMakePresets.json:3`):
  - `windows-x64-debug` -> `build` + `Debug`
  - `windows-x64-release` -> `build` + `Release`
  - `ninja-debug` -> `build-ninja-debug`

### Building
```powershell
cmake --preset windows-x64-release
cmake --build build --config Release --parallel
cmake --build build --config Debug --parallel
ctest --test-dir build -C Release --output-on-failure
# or via wrappers:
.\run.ps1 release        # configure if needed, build Release, launch build/bin/Release/cali.exe
.\run.ps1 dev            # Debug
.\run.ps1 release -Test  # also run ctest
.\run.ps1 release -Clean # rm build, reconfigure
.\run.ps1 release -NoRun # build only
```
Bash: `./run.sh release [--test] [--clean] [--no-build]`

`run.cmd` is thin wrapper for `run.ps1` (`run.cmd:1`).

**Legacy VS:** `cali.sln` + 70 `.vcxproj`/`.sln` deleted in `51e1714`. Toolset was `v142` -> `v143` before deletion.

## Runtime Assets & Paths

All runtime loads are exe-relative via `CommonFileSystem.cpp:37`:
- `construct_shader_path(name)` -> `<exe_dir>/shaders/<name>` (Bruneton, TerrainQuad)
- `get_executable_file_directory()+"\\bitmaps\\heightmap.bmp"` (`TerrainQuad.cpp:132`, `Terrain.cpp:93`)
- `get_executable_file_directory_w()+L"\\courier_new.spritefont"` (`DebugInfo.cpp:13` fixed from relative `L"courier_new.spritefont"` which broke under windbg where cwd != exe dir). Previously `bitmap_image - file .../bitmaps/heightmap.bmp not found` and `BinaryReader failed to load 'courier_new.spritefont'` caused `throw`/`abort()` in `Game::PostRendererInitialize:109` (Debug shows assert, Release exits silently).

CMake `POST_BUILD` copies `src/cali/shaders`, `src/cali/bitmaps`, `courier_new.spritefont` to `<TARGET_FILE_DIR:cali>` (`CMakeLists.txt:300`). Working directory is set to exe dir in `run.ps1:125`.

## Testing
- `src/cali_test/cali_test_main.cpp:4` — `TEST(TerrainQuadTree, quad)` etc., 3 tests. Previously `void main` + `system("pause")` hung `ctest`; fixed to `int main return RUN_ALL_TESTS()` (`cali_test_main.cpp:42`).
- `CALI_BUILD_TESTS=ON` by default. `ctest --test-dir build -C <Config>`.

## Debugging
- Windbg fastest for abort: `cdb -c "g; k; q" build/bin/Debug/cali.exe` showed stack `terrain_quad::terrain_quad+0x530` -> `Game::PostRendererInitialize` -> `wWinMain` and `SpriteFont::SpriteFont` for missing font.
- `DEBUG_OUT` (`IvDebugger.h:28`) only active in Debug (`NDEBUG` off), goes to `OutputDebugStringA` + optional file. Shader errors logged there (`IvFragmentShaderD3D11.cpp:168`).
- Release vs Debug: `ASSERT(x)` (`IvAssert.h:31`) active only in Debug.

## Gotchas for Next Agent
- Do not reintroduce `*.vcxproj`/`*.sln` — project is CMake-only.
- Keep exe-relative asset loading; never use cwd-relative paths.
- Keep `bin/` vs `lib/` separation; `run.*` scripts expect `build/bin/<Config>/cali.exe` with fallbacks to old `build/<Config>/cali.exe` for compat (`run.ps1:101`, `run.sh:79`).
- If adding new `src/cali` files, update `CALI_SOURCES` in `CMakeLists.txt:238`.
- If adding OGL, enable `CALI_GRAPHICS_API_OGL` and provide GLEW/glfw.
- Heightmap is large (6.9 MB) — copied every build; consider `copy_if_different` if slow.
- `.gitignore:38` ignores `build/`, `build-*/`, `out/`; legacy VS artefacts (`*.lib`, `*.pdb`) still ignored but no longer tracked.
