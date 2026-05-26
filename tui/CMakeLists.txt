find_package(Curses REQUIRED)

add_executable(simple-workout-tracker
    main.cpp
    app.cpp
    views/workout_view.cpp
    views/history_view.cpp
    views/exercise_view.cpp
    views/progress_view.cpp
    views/template_view.cpp
    views/data_view.cpp
    widgets/input.cpp
    widgets/table.cpp
    widgets/menu.cpp
)

target_include_directories(simple-workout-tracker PRIVATE ${CURSES_INCLUDE_DIRS})
target_link_libraries(simple-workout-tracker PRIVATE swt ${CURSES_LIBRARIES})
