add_library(gui OBJECT)

target_sources(gui
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/app.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/main_frame.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gui_controller.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/connection_settings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/controls/main_display.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/controls/magic_map.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/controls/input_bar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/panels/comms.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/panels/magic_map_panel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/panels/map_notebook.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/panels/right_column.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/panels/vitals_bar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/panels/system_log.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/panels/connection_footer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/menu/menu_bar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/connect_dialog.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/error_dialog.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/settings_dialog.cpp
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
