## Add library for module1
#add_library(Dictionary STATIC dict.c)
#add_library(Utils STATIC utils.c)
#add_library(Keymap STATIC keymap.c)
#add_library(Render STATIC render.c)
#add_library(Vector STATIC vector.c)
#add_library(Editor STATIC editor.c)
#add_library(Highlight STATIC highlight.c)
#add_library(Logger STATIC logger.c)
#
## Add executable
#add_executable(Texture texture.c)
#
#target_link_libraries(Texture PRIVATE Dictionary)
#target_link_libraries(Texture PRIVATE Utils)
#target_link_libraries(Texture PRIVATE Keymap)
#target_link_libraries(Texture PRIVATE Render)
#target_link_libraries(Texture PRIVATE Vector)
#target_link_libraries(Texture PRIVATE Editor)
#target_link_libraries(Texture PRIVATE Highlight)
#target_link_libraries(Texture PRIVATE Logger)
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

# Set include directories
target_include_directories(Assert PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(Dictionary PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(Editor PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(Highlight PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(Keymap PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(Logger PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(Render PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(Utils PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(Vector PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(Texture PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Create the main executable
add_executable(MyProject test.c)

# Link libraries to the main executable
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
)
