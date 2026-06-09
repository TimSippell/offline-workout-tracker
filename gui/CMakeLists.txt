set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/extern/imgui)
set(IMPLOT_DIR ${CMAKE_SOURCE_DIR}/extern/implot)

add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/imgui_impl_opengl3.cpp
    ${IMPLOT_DIR}/implot.cpp
    ${IMPLOT_DIR}/implot_items.cpp
)

target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMPLOT_DIR}
)

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)

target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

add_executable(owt-gui
    main.cpp
    app.cpp
    theme.cpp
    views/workout_view.cpp
    views/history_view.cpp
    views/exercise_view.cpp
    views/template_view.cpp
    views/progress_view.cpp
    views/settings_view.cpp
)

target_link_libraries(owt-gui PRIVATE owt imgui)
