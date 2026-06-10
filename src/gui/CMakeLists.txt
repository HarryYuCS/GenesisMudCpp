add_library(gui OBJECT)

target_sources(gui
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/app.cpp
)

target_include_directories(gui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

target_link_libraries(gui
    PUBLIC
        mudcore
        ${wxWidgets_LIBRARIES}
)
