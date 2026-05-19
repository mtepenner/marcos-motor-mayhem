cmake_minimum_required(VERSION 3.20)
project(marcos-motor-mayhem
    VERSION 1.0.0
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")

# Default to Release build
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Find required packages
find_package(Vulkan REQUIRED)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Engine library
add_library(marcos_engine STATIC
    # Core
    src/engine/core/main.cpp
    src/engine/core/game_loop.cpp
    src/engine/core/memory_pool.cpp
    # Physics
    src/engine/physics/collision.cpp
    src/engine/physics/rigidbody.cpp
    src/engine/physics/kart_dynamics.cpp
    # Rendering
    src/engine/render/renderer.cpp
    src/engine/render/vulkan_backend.cpp
    src/engine/render/camera.cpp
    src/engine/render/shader_manager.cpp
    # Input
    src/engine/input/input_manager.cpp
    src/engine/input/controller_map.cpp
)

target_include_directories(marcos_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(marcos_engine PUBLIC
    ${Vulkan_LIBRARIES}
)

# Game library
add_library(marcos_game STATIC
    # Entities
    src/game/entities/entity.cpp
    src/game/entities/kart.cpp
    src/game/entities/hazard.cpp
    # Items
    src/game/items/item_manager.cpp
    src/game/items/projectile.cpp
    src/game/items/boost.cpp
    # Modes
    src/game/modes/race_state.cpp
    src/game/modes/grand_prix.cpp
    src/game/modes/time_trial.cpp
)

target_include_directories(marcos_game PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(marcos_game PUBLIC
    marcos_engine
)

# Main game executable
add_executable(marcos-motor-mayhem
    src/engine/core/main.cpp
)

target_link_libraries(marcos-motor-mayhem PRIVATE
    marcos_engine
    marcos_game
)

# Testing
enable_testing()

# Catch2 unit tests for engine
add_executable(engine_tests
    tests/engine_tests/test_collision.cpp
    tests/engine_tests/test_kart_dynamics.cpp
    tests/engine_tests/test_memory_pool.cpp
)

target_link_libraries(engine_tests PRIVATE
    marcos_engine
)

add_test(NAME EngineTests COMMAND engine_tests)

# Compiler-specific options
if(MSVC)
    target_compile_options(marcos_engine PRIVATE /W4)
    target_compile_options(marcos_game PRIVATE /W4)
else()
    target_compile_options(marcos_engine PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(marcos_game PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install targets
install(TARGETS marcos-motor-mayhem
    RUNTIME DESTINATION bin
)

install(DIRECTORY assets/
    DESTINATION share/marcos-motor-mayhem
)

# Print build configuration
message(STATUS "marcos-motor-mayhem build configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Vulkan: ${Vulkan_FOUND}")
