cmake_minimum_required(VERSION 3.16)
project(stdgeo-gui VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Set build type options
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")

# Export compile commands for IDE integration
# This command removes the squiggly red underlines in VS Code
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler flags for different build types
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Print build configuration
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ flags: ${CMAKE_CXX_FLAGS}")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug flags: ${CMAKE_CXX_FLAGS_DEBUG}")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Release flags: ${CMAKE_CXX_FLAGS_RELEASE}")
endif()

# Find required packages
find_package(wxWidgets REQUIRED COMPONENTS core base gl aui)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

# GLM (header-only library, no find_package needed in most distros)
find_path(GLM_INCLUDE_DIR glm/glm.hpp
    PATHS /usr/include /usr/local/include
)

if(NOT GLM_INCLUDE_DIR)
    message(FATAL_ERROR "GLM not found. Please install libglm-dev")
endif()

# Include wxWidgets macros
include(${wxWidgets_USE_FILE})

# Source files
set(SOURCES
    src/App.cpp
    src/MainFrame.cpp
    src/GLCanvas.cpp
    src/Terminal.cpp
    src/Toolbar.cpp
)

# Headers
set(HEADERS
    src/CommonHeader.h
    src/App.h
    src/MainFrame.h
    src/GLCanvas.h
    src/Terminal.h
    src/Toolbar.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${GLM_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${wxWidgets_LIBRARIES}
    ${OPENGL_LIBRARIES}
    GLEW::GLEW
)

# Copy shaders to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
    COMMENT "Copying shaders to build directory"
)

# Installation rules
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(DIRECTORY shaders DESTINATION share/${PROJECT_NAME})