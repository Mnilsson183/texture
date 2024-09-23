# CMakeLists.txt in project_root/src/

# Add library for module1
add_library(Dictionary STATIC dict.c)
add_library(Utils STATIC utils.c)
add_library(Keymap STATIC keymap.c)
add_library(Render STATIC render.c)
add_library(Vector STATIC vector.c)
add_library(Editor STATIC editor.c)
add_library(Highlight STATIC highlight.c)

# Add executable
add_executable(Texture texture.c)

target_link_libraries(Texture PRIVATE Dictionary)
target_link_libraries(Texture PRIVATE Utils)
target_link_libraries(Texture PRIVATE Keymap)
target_link_libraries(Texture PRIVATE Render)
target_link_libraries(Texture PRIVATE Vector)
target_link_libraries(Texture PRIVATE Editor)
target_link_libraries(Texture PRIVATE Highlight)
