cmake_minimum_required(VERSION 3.15)
project(GameEngine2D CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MINGW)
    add_compile_options(-Wno-narrowing)
endif()

set(GLFW_BIN "${CMAKE_CURRENT_SOURCE_DIR}/third_party/glfw-3.3.8.bin.WIN32")
# GLFW headers are shared, but the import library and DLL must match the target.
set(GLFW_LIB_DIR "${GLFW_BIN}/lib-mingw")
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(GLFW_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/glfw-3.3.8.bin.WIN64/lib-mingw-w64")
endif()

add_executable(Game
    src/GLLoader.cpp
    src/Shader.cpp
    src/Renderer2D.cpp
    src/Entity.cpp
    src/GridSystem.cpp
    src/WaveSpawner.cpp
    src/SpatialCollision.cpp
    src/UIManager.cpp
    src/AudioEngine.cpp
    src/GameEngine.cpp
    src/Main.cpp
)

target_include_directories(Game PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${GLFW_BIN}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

target_link_libraries(Game PRIVATE
    "${GLFW_LIB_DIR}/libglfw3dll.a"
    opengl32
    gdi32
    user32
    shell32
    winmm
    psapi
)

add_custom_command(TARGET Game POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${GLFW_LIB_DIR}/glfw3.dll"
    "$<TARGET_FILE_DIR:Game>"
)
