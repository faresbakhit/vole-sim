cmake_minimum_required(VERSION 3.5)

project(vole-sim
        VERSION 0.1.0
        DESCRIPTION "Vole Machine Simulator & GUI"
        LANGUAGES CXX)

if(POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW)
endif()

set(EMSCRIPTEN_DIR imgui/)

if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
  set(USE_FLAGS "-s USE_SDL=2 -s USE_FREETYPE=1")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${USE_FLAGS}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${USE_FLAGS} --shell-file ../emscripten/shell.html")
  set(CMAKE_EXECUTABLE_SUFFIX .html)
else()
  find_package(SDL2 REQUIRED)
  find_package(OpenGL REQUIRED)
  find_package(Freetype REQUIRED)
endif()

set(IMGUI_DIR imgui/)

if(MSVC)
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
endif()

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
  add_executable(
    vole-sim
    src/cli.cpp
    src/vole.cpp
    src/vole.h)
endif()

add_executable(
  vole-sim-gui
  WIN32
  src/gui.cpp
  src/vole.cpp
  src/vole.h
  ${GLAD_GL}
  # Dear ImGui files
  ${IMGUI_DIR}/imconfig.h
  ${IMGUI_DIR}/imgui.h
  ${IMGUI_DIR}/imstb_rectpack.h
  ${IMGUI_DIR}/imstb_textedit.h
  ${IMGUI_DIR}/imstb_truetype.h
  ${IMGUI_DIR}/backends/imgui_impl_sdl2.h
  ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
  ${IMGUI_DIR}/backends/imgui_impl_opengl3.h
  ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
  ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
  ${IMGUI_DIR}/misc/freetype/imgui_freetype.cpp
  ${IMGUI_DIR}/misc/freetype/imgui_freetype.h
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/imgui_tables.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp)

target_link_libraries(vole-sim-gui
  ${OPENGL_LIBRARIES}
  ${SDL2_LIBRARIES}
  ${FREETYPE_LIBRARIES})

if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
  set_target_properties(vole-sim-gui PROPERTIES OUTPUT_NAME "index")
endif()

if(MSVC)
    if(${CMAKE_VERSION} VERSION_LESS "3.6.0")
        message("\n\t[ WARNING ]\n\n\tCMake version lower than 3.6.\n\n\t - Please update CMake and rerun; OR\n\t - Manually set 'vole-sim-gui' as StartUp Project in Visual Studio.\n")
    else()
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT vole-sim-gui)
    endif()
endif()

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
  set_property(TARGET vole-sim PROPERTY CXX_STANDARD 17)
  set_property(TARGET vole-sim PROPERTY CXX_STANDARD_REQUIRED On)
  set_property(TARGET vole-sim PROPERTY CXX_EXTENSIONS Off)
endif()
set_property(TARGET vole-sim-gui PROPERTY CXX_STANDARD 17)
set_property(TARGET vole-sim-gui PROPERTY CXX_STANDARD_REQUIRED On)
set_property(TARGET vole-sim-gui PROPERTY CXX_EXTENSIONS Off)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
      # vole-sim
      target_compile_options(vole-sim PRIVATE -Wall -Wextra -fno-omit-frame-pointer -fsanitize=address)
      target_link_options(vole-sim PRIVATE -fno-omit-frame-pointer -fsanitize=address)
    endif()
    # vole-sim-gui
    target_compile_options(vole-sim-gui PRIVATE -Wall -Wextra -fno-omit-frame-pointer -fsanitize=address)
    target_link_options(vole-sim-gui PRIVATE -fno-omit-frame-pointer -fsanitize=address)
  endif()
endif()

target_include_directories(vole-sim-gui PRIVATE
  ${IMGUI_DIR}
  ${IMGUI_DIR}/backends
  ${SDL2_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIRS}
  ${FREETYPE_INCLUDE_DIRS})
