cmake_minimum_required(VERSION 3.10)
project(MyGame)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

# Add include directories for SDL2 and SDL2_image
include_directories(
    /opt/homebrew/Cellar/sdl2/2.30.8/include/SDL2
    /opt/homebrew/Cellar/sdl2_image/2.8.2_2/include/SDL2
    ${PROJECT_SOURCE_DIR}/include
)

# Add library directories
link_directories(
    /opt/homebrew/Cellar/sdl2/2.30.8/lib
    /opt/homebrew/Cellar/sdl2_image/2.8.2_2/lib
)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp")

# Add executable
add_executable(Platformer_exe ${SOURCES} ${HEADERS})

# Link SDL2 and SDL2_image libraries explicitly
target_link_libraries(Platformer_exe
    SDL2
    SDL2_image
)

# Optional: set up RPATH for portable deployment
set_target_properties(Platformer_exe PROPERTIES
    INSTALL_RPATH "$ORIGIN"
)

add_custom_command(
    TARGET Platformer_exe POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:Platformer_exe>/resources
    COMMENT "Creating symlink to resources directory in output directory"
)