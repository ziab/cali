cmake_minimum_required(VERSION 3.20)
project(cali LANGUAGES CXX)

# --- Options ---
option(CALI_BUILD_TESTS "Build cali_test" ON)
option(CALI_GRAPHICS_API_D3D11 "Use D3D11 renderer (Windows only)" ON)
# OGL is legacy and requires GLEW/GLFW not vendored for CMake; off by default
option(CALI_GRAPHICS_API_OGL "Use OpenGL renderer (requires GLEW/GLFW)" OFF)

if(NOT WIN32 AND CALI_GRAPHICS_API_D3D11)
    message(WARNING "D3D11 is Windows-only. Disabling D3D11, falling back to OGL if enabled.")
    set(CALI_GRAPHICS_API_D3D11 OFF)
endif()

if(NOT CALI_GRAPHICS_API_D3D11 AND NOT CALI_GRAPHICS_API_OGL)
    message(FATAL_ERROR "No graphics API selected. Enable CALI_GRAPHICS_API_D3D11 or CALI_GRAPHICS_API_OGL")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Keep build tree tidy: exes -> bin/<Config>, static libs -> lib/<Config>
# Without this, VS puts every .lib next to the .exe (the issue you saw in build/Release).
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
foreach(_cfg IN ITEMS Debug Release RelWithDebInfo MinSizeRel)
  string(TOUPPER ${_cfg} _ucfg)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_ucfg} ${CMAKE_BINARY_DIR}/bin/${_cfg})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${_ucfg} ${CMAKE_BINARY_DIR}/lib/${_cfg})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${_ucfg} ${CMAKE_BINARY_DIR}/lib/${_cfg})
endforeach()

# Use Unicode for D3D11 (matching vcxproj), MultiByte for OGL
if(CALI_GRAPHICS_API_D3D11)
    add_compile_definitions(UNICODE _UNICODE)
endif()

# Common MSVC flags
if(MSVC)
    add_compile_options(/W3)
    add_compile_definitions(_CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE)
endif()

# ---------------------------------------------------------------------------
# Essential Math libraries
# ---------------------------------------------------------------------------
set(ESSENTIAL_MATH_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/depends/essential_math/common)

# IvMath
add_library(IvMath STATIC
    ${ESSENTIAL_MATH_ROOT}/IvMath/IvDoubleVector3.cpp
    ${ESSENTIAL_MATH_ROOT}/IvMath/IvGaussianElim.cpp
    ${ESSENTIAL_MATH_ROOT}/IvMath/IvLine3.cpp
    ${ESSENTIAL_MATH_ROOT}/IvMath/IvLineSegment3.cpp
    ${ESSENTIAL_MATH_ROOT}/IvMath/IvMath.cpp
    ${ESSENTIAL_MATH_ROOT}/IvMath/IvMatrix33.cpp
    ${ESSENTIAL_MATH_ROOT}/IvMath/IvMatrix44.cpp
    ${ESSENTIAL_MATH_ROOT}/IvMath/IvPlane.cpp
    ${ESSENTIAL_MATH_ROOT}/IvMath/IvQuat.cpp
    ${ESSENTIAL_MATH_ROOT}/IvMath/IvRay3.cpp
    ${ESSENTIAL_MATH_ROOT}/IvMath/IvTriangle.cpp
    ${ESSENTIAL_MATH_ROOT}/IvMath/IvVector2.cpp
    ${ESSENTIAL_MATH_ROOT}/IvMath/IvVector3.cpp
    ${ESSENTIAL_MATH_ROOT}/IvMath/IvVector4.cpp
)
target_include_directories(IvMath PUBLIC
    ${ESSENTIAL_MATH_ROOT}/IvMath
    ${ESSENTIAL_MATH_ROOT}/IvUtility
)
target_compile_definitions(IvMath PRIVATE _LIB)

# IvUtility
add_library(IvUtility STATIC
    ${ESSENTIAL_MATH_ROOT}/IvUtility/IvDebugger.cpp
    ${ESSENTIAL_MATH_ROOT}/IvUtility/IvImage.cpp
    ${ESSENTIAL_MATH_ROOT}/IvUtility/IvStackAllocator.cpp
)
target_include_directories(IvUtility PUBLIC
    ${ESSENTIAL_MATH_ROOT}/IvUtility
)
target_compile_definitions(IvUtility PRIVATE _LIB)

# IvCollision
add_library(IvCollision STATIC
    ${ESSENTIAL_MATH_ROOT}/IvCollision/IvAABB.cpp
    ${ESSENTIAL_MATH_ROOT}/IvCollision/IvBoundingSphere.cpp
    ${ESSENTIAL_MATH_ROOT}/IvCollision/IvCapsule.cpp
    ${ESSENTIAL_MATH_ROOT}/IvCollision/IvCovariance.cpp
    ${ESSENTIAL_MATH_ROOT}/IvCollision/IvOBB.cpp
)
target_include_directories(IvCollision PUBLIC
    ${ESSENTIAL_MATH_ROOT}/IvCollision
    ${ESSENTIAL_MATH_ROOT}/IvMath
    ${ESSENTIAL_MATH_ROOT}/IvUtility
)
target_link_libraries(IvCollision PUBLIC IvMath IvUtility)
target_compile_definitions(IvCollision PRIVATE _LIB)

# IvGraphics - choose D3D11 or OGL sources
set(IVGRAPHICS_COMMON_SOURCES
    ${ESSENTIAL_MATH_ROOT}/IvGraphics/IvConstantBufferD3D11.cpp
    ${ESSENTIAL_MATH_ROOT}/IvGraphics/IvRenderer.cpp
    ${ESSENTIAL_MATH_ROOT}/IvGraphics/IvRendererHelp.cpp
    ${ESSENTIAL_MATH_ROOT}/IvGraphics/IvTeapot.cpp
)

if(CALI_GRAPHICS_API_D3D11)
    set(IVGRAPHICS_API_SOURCES
        ${ESSENTIAL_MATH_ROOT}/IvGraphics/D3D11/IvConstantTableD3D11.cpp
        ${ESSENTIAL_MATH_ROOT}/IvGraphics/D3D11/IvFragmentShaderD3D11.cpp
        ${ESSENTIAL_MATH_ROOT}/IvGraphics/D3D11/IvIndexBufferD3D11.cpp
        ${ESSENTIAL_MATH_ROOT}/IvGraphics/D3D11/IvPointRendererD3D11.cpp
        ${ESSENTIAL_MATH_ROOT}/IvGraphics/D3D11/IvRendererD3D11.cpp
        ${ESSENTIAL_MATH_ROOT}/IvGraphics/D3D11/IvRendererHelpD3D11.cpp
        ${ESSENTIAL_MATH_ROOT}/IvGraphics/D3D11/IvRenderTextureD3D11.cpp
        ${ESSENTIAL_MATH_ROOT}/IvGraphics/D3D11/IvResourceManagerD3D11.cpp
        ${ESSENTIAL_MATH_ROOT}/IvGraphics/D3D11/IvShaderProgramD3D11.cpp
        ${ESSENTIAL_MATH_ROOT}/IvGraphics/D3D11/IvTextureD3D11.cpp
        ${ESSENTIAL_MATH_ROOT}/IvGraphics/D3D11/IvUniformD3D11.cpp
        ${ESSENTIAL_MATH_ROOT}/IvGraphics/D3D11/IvVertexBufferD3D11.cpp
        ${ESSENTIAL_MATH_ROOT}/IvGraphics/D3D11/IvVertexShaderD3D11.cpp
    )
else()
    set(IVGRAPHICS_API_SOURCES
        ${ESSENTIAL_MATH_ROOT}/IvGraphics/OGL/IvFragmentShaderOGL.cpp
        ${ESSENTIAL_MATH_ROOT}/IvGraphics/OGL/IvIndexBufferOGL.cpp
        ${ESSENTIAL_MATH_ROOT}/IvGraphics/OGL/IvRendererHelpOGL.cpp
        ${ESSENTIAL_MATH_ROOT}/IvGraphics/OGL/IvRendererOGL.cpp
        ${ESSENTIAL_MATH_ROOT}/IvGraphics/OGL/IvResourceManagerOGL.cpp
        ${ESSENTIAL_MATH_ROOT}/IvGraphics/OGL/IvShaderProgramOGL.cpp
        ${ESSENTIAL_MATH_ROOT}/IvGraphics/OGL/IvTextureOGL.cpp
        ${ESSENTIAL_MATH_ROOT}/IvGraphics/OGL/IvUniformOGL.cpp
        ${ESSENTIAL_MATH_ROOT}/IvGraphics/OGL/IvVertexBufferOGL.cpp
        ${ESSENTIAL_MATH_ROOT}/IvGraphics/OGL/IvVertexShaderOGL.cpp
    )
endif()

add_library(IvGraphics STATIC
    ${IVGRAPHICS_COMMON_SOURCES}
    ${IVGRAPHICS_API_SOURCES}
)
target_include_directories(IvGraphics PUBLIC
    ${ESSENTIAL_MATH_ROOT}/IvGraphics
    ${ESSENTIAL_MATH_ROOT}/IvMath
    ${ESSENTIAL_MATH_ROOT}/IvUtility
    ${ESSENTIAL_MATH_ROOT}
    ${ESSENTIAL_MATH_ROOT}/..
)
if(CALI_GRAPHICS_API_D3D11)
    target_include_directories(IvGraphics PUBLIC ${ESSENTIAL_MATH_ROOT}/IvGraphics/D3D11)
else()
    target_include_directories(IvGraphics PUBLIC ${ESSENTIAL_MATH_ROOT}/IvGraphics/OGL)
    # OGL needs GLEW/GLFW includes - user must provide
    find_package(GLEW QUIET)
    find_package(glfw3 QUIET)
    if(GLEW_FOUND)
        target_link_libraries(IvGraphics PUBLIC GLEW::GLEW)
    endif()
    if(glfw3_FOUND)
        target_link_libraries(IvGraphics PUBLIC glfw)
    endif()
endif()
target_link_libraries(IvGraphics PUBLIC IvMath IvUtility)
target_compile_definitions(IvGraphics PRIVATE _LIB)
if(CALI_GRAPHICS_API_D3D11 AND MSVC)
    # match vcxproj - no GLEW_STATIC for D3D11
else()
    target_compile_definitions(IvGraphics PRIVATE GLEW_STATIC)
endif()

# IvEngine
set(IVENGINE_COMMON_SOURCES
    ${ESSENTIAL_MATH_ROOT}/IvEngine/IvAdditionalInputHandler.cpp
    ${ESSENTIAL_MATH_ROOT}/IvEngine/IvClock.cpp
    ${ESSENTIAL_MATH_ROOT}/IvEngine/IvEventHandler.cpp
    ${ESSENTIAL_MATH_ROOT}/IvEngine/IvGame.cpp
)
if(CALI_GRAPHICS_API_D3D11)
    list(APPEND IVENGINE_COMMON_SOURCES ${ESSENTIAL_MATH_ROOT}/IvEngine/D3D11/IvMainD3D11.cpp)
else()
    list(APPEND IVENGINE_COMMON_SOURCES ${ESSENTIAL_MATH_ROOT}/IvEngine/OGL/IvMainOGL.cpp)
endif()

add_library(IvEngine STATIC ${IVENGINE_COMMON_SOURCES})
target_include_directories(IvEngine PUBLIC
    ${ESSENTIAL_MATH_ROOT}/IvEngine
    ${ESSENTIAL_MATH_ROOT}/IvGraphics
    ${ESSENTIAL_MATH_ROOT}/IvMath
    ${ESSENTIAL_MATH_ROOT}/IvUtility
    ${ESSENTIAL_MATH_ROOT}/IvCollision
)
target_link_libraries(IvEngine PUBLIC IvGraphics IvCollision IvMath IvUtility)
target_compile_definitions(IvEngine PRIVATE _LIB)

# ---------------------------------------------------------------------------
# DirectXTK
# ---------------------------------------------------------------------------
set(DIRECTXTK_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/depends/DirectXTK-master)

set(DIRECTXTK_SOURCES
    ${DIRECTXTK_ROOT}/Src/AlphaTestEffect.cpp
    ${DIRECTXTK_ROOT}/Src/BasicEffect.cpp
    ${DIRECTXTK_ROOT}/Src/CommonStates.cpp
    ${DIRECTXTK_ROOT}/Src/DDSTextureLoader.cpp
    ${DIRECTXTK_ROOT}/Src/DGSLEffect.cpp
    ${DIRECTXTK_ROOT}/Src/DGSLEffectFactory.cpp
    ${DIRECTXTK_ROOT}/Src/DualTextureEffect.cpp
    ${DIRECTXTK_ROOT}/Src/BinaryReader.cpp
    ${DIRECTXTK_ROOT}/Src/EffectCommon.cpp
    ${DIRECTXTK_ROOT}/Src/EffectFactory.cpp
    ${DIRECTXTK_ROOT}/Src/EnvironmentMapEffect.cpp
    ${DIRECTXTK_ROOT}/Src/GamePad.cpp
    ${DIRECTXTK_ROOT}/Src/GeometricPrimitive.cpp
    ${DIRECTXTK_ROOT}/Src/Geometry.cpp
    ${DIRECTXTK_ROOT}/Src/GraphicsMemory.cpp
    ${DIRECTXTK_ROOT}/Src/Keyboard.cpp
    ${DIRECTXTK_ROOT}/Src/Model.cpp
    ${DIRECTXTK_ROOT}/Src/ModelLoadCMO.cpp
    ${DIRECTXTK_ROOT}/Src/ModelLoadSDKMESH.cpp
    ${DIRECTXTK_ROOT}/Src/ModelLoadVBO.cpp
    ${DIRECTXTK_ROOT}/Src/Mouse.cpp
    ${DIRECTXTK_ROOT}/Src/NormalMapEffect.cpp
    ${DIRECTXTK_ROOT}/Src/pch.cpp
    ${DIRECTXTK_ROOT}/Src/ScreenGrab.cpp
    ${DIRECTXTK_ROOT}/Src/SimpleMath.cpp
    ${DIRECTXTK_ROOT}/Src/SkinnedEffect.cpp
    ${DIRECTXTK_ROOT}/Src/SpriteBatch.cpp
    ${DIRECTXTK_ROOT}/Src/PrimitiveBatch.cpp
    ${DIRECTXTK_ROOT}/Src/SpriteFont.cpp
    ${DIRECTXTK_ROOT}/Src/VertexTypes.cpp
    ${DIRECTXTK_ROOT}/Src/WICTextureLoader.cpp
)

add_library(DirectXTK STATIC ${DIRECTXTK_SOURCES})
target_include_directories(DirectXTK PUBLIC ${DIRECTXTK_ROOT}/Inc PRIVATE ${DIRECTXTK_ROOT}/Src)
target_compile_definitions(DirectXTK PRIVATE _LIB)
# Match original DirectXTK_Desktop_2015.vcxproj defines
target_compile_definitions(DirectXTK PUBLIC _WIN32_WINNT=0x0600 _WIN7_PLATFORM_UPDATE _CRT_STDIO_ARBITRARY_WIDE_SPECIFIERS)
if(MSVC)
    target_compile_options(DirectXTK PRIVATE /W4 /fp:fast)
endif()
# Disable precompiled headers - we compile pch.cpp normally
set_target_properties(DirectXTK PROPERTIES DISABLE_PRECOMPILE_HEADERS ON)

# ---------------------------------------------------------------------------
# cali executable
# ---------------------------------------------------------------------------
set(CALI_SOURCES
    src/cali/AABB.cpp
    src/cali/Box.cpp
    src/cali/Bruneton.cpp
    src/cali/Camera.cpp
    src/cali/CommonFileSystem.cpp
    src/cali/CommonTexture.cpp
    src/cali/Procedural.cpp
    src/cali/DebugInfo.cpp
    src/cali/Frustum.cpp
    src/cali/Game.cpp
    src/cali/Icosahedron.cpp
    src/cali/InputController.cpp
    src/cali/Model.cpp
    src/cali/PostEffect.cpp
    src/cali/Renderable.cpp
    src/cali/Sky.cpp
    src/cali/Stars.cpp
    src/cali/Sun.cpp
    src/cali/Terrain.cpp
    src/cali/Grid.cpp
    src/cali/TerrainIcosahedron.cpp
    src/cali/TerrainQuad.cpp
)

add_executable(cali ${CALI_SOURCES})

target_include_directories(cali PRIVATE
    src/cali
    depends/bitmap_image
    ${DIRECTXTK_ROOT}/Inc
    ${ESSENTIAL_MATH_ROOT}/IvMath
    ${ESSENTIAL_MATH_ROOT}/IvUtility
    ${ESSENTIAL_MATH_ROOT}/IvGraphics
    ${ESSENTIAL_MATH_ROOT}/IvEngine
    ${ESSENTIAL_MATH_ROOT}/IvCollision
)

target_link_libraries(cali PRIVATE
    IvEngine
    IvGraphics
    IvMath
    IvUtility
    IvCollision
    DirectXTK
)

if(CALI_GRAPHICS_API_D3D11)
    target_compile_definitions(cali PRIVATE WIN32 _WINDOWS)
    target_link_libraries(cali PRIVATE d3d11 d3dcompiler dxgi dxguid comctl32)
    set_target_properties(cali PROPERTIES WIN32_EXECUTABLE TRUE)
    # Entry point is wWinMain (IvMainD3D11.cpp)
    if(MSVC)
        set_target_properties(cali PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:wWinMainCRTStartup")
    endif()
else()
    target_compile_definitions(cali PRIVATE WIN32 _WINDOWS GLEW_STATIC)
    target_link_libraries(cali PRIVATE opengl32)
    if(MSVC)
        set_target_properties(cali PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
    endif()
endif()

# Copy shaders and assets next to the executable
add_custom_command(TARGET cali POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cali/shaders
        $<TARGET_FILE_DIR:cali>/shaders
    COMMENT "Copying shaders to output directory"
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/cali/bitmaps)
    add_custom_command(TARGET cali POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/src/cali/bitmaps
            $<TARGET_FILE_DIR:cali>/bitmaps
        COMMENT "Copying bitmaps to output directory"
    )
endif()

# Also copy spritefont if present at runtime (optional)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/cali/courier_new.spritefont)
    add_custom_command(TARGET cali POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/src/cali/courier_new.spritefont
            $<TARGET_FILE_DIR:cali>/courier_new.spritefont
    )
endif()

# ---------------------------------------------------------------------------
# cali_test
# ---------------------------------------------------------------------------
if(CALI_BUILD_TESTS)
    enable_testing()

    # Bundled gtest - keep offline build working
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/depends/gtest/gtest-all.cpp)
        add_library(gtest_bundled STATIC depends/gtest/gtest-all.cpp)
        target_include_directories(gtest_bundled PUBLIC depends/gtest)
        # gtest needs threads on some platforms
        find_package(Threads QUIET)
        if(Threads_FOUND)
            target_link_libraries(gtest_bundled PUBLIC Threads::Threads)
        endif()
        set(GTEST_LIB gtest_bundled)
    else()
        # Fallback to FetchContent
        include(FetchContent)
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        )
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
        set(GTEST_LIB gtest_main)
    endif()

    add_executable(cali_test src/cali_test/cali_test_main.cpp)
    target_include_directories(cali_test PRIVATE src/cali depends/gtest)
    target_link_libraries(cali_test PRIVATE ${GTEST_LIB})

    # cali_test currently only tests TerrainQuadTree which is header-only,
    # but link cali sources if needed in future - for now standalone
    # To allow testing of cali math, add include path
    target_include_directories(cali_test PRIVATE src/cali)

    add_test(NAME cali_test COMMAND cali_test)
endif()

# ---------------------------------------------------------------------------
# Packaging / install
# ---------------------------------------------------------------------------
install(TARGETS cali RUNTIME DESTINATION bin)
install(DIRECTORY src/cali/shaders DESTINATION bin)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/cali/courier_new.spritefont)
    install(FILES src/cali/courier_new.spritefont DESTINATION bin)
endif()
