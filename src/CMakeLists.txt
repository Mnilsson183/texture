# CMakeLists.txt in project_root/src/

# Add library for module1
add_library(Dictionary STATIC dict.c)
add_library(Utils STATIC utils.c)
add_library(Keymap STATIC keymap.c)
add_library(Render STATIC render.c)

# Add executable
add_executable(MyProject texture.c)

target_link_libraries(MyProject PRIVATE Dictionary)
target_link_libraries(MyProject PRIVATE Utils)
target_link_libraries(MyProject PRIVATE Keymap)
target_link_libraries(MyProject PRIVATE Render)

