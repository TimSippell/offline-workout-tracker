if(WIN32)
    find_path(PDCURSES_INCLUDE_DIR curses.h)
    find_library(PDCURSES_LIBRARY NAMES pdcurses)
    if(NOT PDCURSES_INCLUDE_DIR OR NOT PDCURSES_LIBRARY)
        message(FATAL_ERROR "PDCurses not found. Install via: vcpkg install pdcurses")
    endif()
    set(CURSES_INCLUDE_DIRS ${PDCURSES_INCLUDE_DIR})
    set(CURSES_LIBRARIES ${PDCURSES_LIBRARY})
else()
    find_package(Curses REQUIRED)
endif()

add_executable(offline-workout-tracker
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

target_include_directories(offline-workout-tracker PRIVATE ${CURSES_INCLUDE_DIRS})
target_link_libraries(offline-workout-tracker PRIVATE owt ${CURSES_LIBRARIES})
