# Find the ncurses package
find_package(Curses REQUIRED)

# Include the ncurses headers
include_directories(${CURSES_INCLUDE_DIR})

# Define libraries
add_library(Assert assert.c)
add_library(Dictionary dict.c)
add_library(Editor editor.c)
add_library(Highlight highlight.c)
add_library(Keymap keymap.c)
add_library(Logger logger.c)
add_library(Render render.c)
add_library(Utils utils.c)
add_library(Vector vector.c)
add_library(Texture texture.c)
add_library(AssertionTests assert.c)

# Set include directories and link with ncurses for each library
foreach(lib Assert Dictionary Editor Highlight Keymap Logger Render Utils Vector Texture AssertionTests)
    target_include_directories(${lib} PUBLIC ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${lib} ${CURSES_LIBRARIES})
endforeach()

# Create the main executable
add_executable(MyProject test.c)

# Link libraries to the main executable, including ncurses
target_link_libraries(MyProject
    Assert
    Dictionary
    Editor
    Highlight
    Keymap
    Logger
    Render
    Utils
    Vector
    Texture
    ${CURSES_LIBRARIES}
)

# Set include directories for the main executable
target_include_directories(MyProject PUBLIC ${CMAKE_SOURCE_DIR}/include)
