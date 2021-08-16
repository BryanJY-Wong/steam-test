cmake_minimum_required(VERSION 3.12)
project(steam_test)

if (NOT DEFINED STEAM_SDK_PATH)
    message( "Expected cmake -DSTEAM_SDK_PATH=<path> )")
    message( FATAL_ERROR "Please provide a valid sdk path" )
else()
    message("STEAM SDK PATH: ${STEAM_SDK_PATH}")
    set(STEAM_INCLUDE_DIRS ${STEAM_SDK_PATH}/public)
    set(STEAM_BINARIES ${STEAM_SDK_PATH}/redistributable_bin/win64/steam_api64.dll)
    if (WIN32)
        set(STEAM_LINK_LIBRARIES ${STEAM_SDK_PATH}/redistributable_bin/win64/steam_api64.lib)
    else()
        message( FATAL_ERROR "Only windows!" )
    endif()

    message("STEAM LINK LIBRARIES: ${STEAM_LINK_LIBRARIES}")
    message("Binary Directory: ${CMAKE_BINARY_DIR}")
endif()

set(CMAKE_CXX_STANDARD 11)
include_directories(${STEAM_INCLUDE_DIRS})
include_directories(include)

add_executable(main main.cpp)
target_link_libraries(main ${STEAM_LINK_LIBRARIES})
add_custom_command(TARGET main POST_BUILD        # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${STEAM_BINARIES}"      # <--this is in-file
        $<TARGET_FILE_DIR:main>)


