cmake_minimum_required(VERSION 3.12)
project(steam_test)

if (NOT DEFINED STEAM_SDK_PATH)
    message( "Expected cmake -DSTEAM_SDK_PATH=<path> )")
    message( FATAL_ERROR "Please provide a valid sdk path" )
else()
    message("STEAM SDK PATH: ${STEAM_SDK_PATH}")
    set(STEAM_INCLUDE_DIRS ${STEAM_SDK_PATH}/public)
    
    if (WIN32)
        set(STEAM_LINK_LIBRARIES ${STEAM_SDK_PATH}/redistributable_bin/win64/steam_api64.lib)
        set(STEAM_BINARIES ${STEAM_SDK_PATH}/redistributable_bin/win64/steam_api64.dll)
    elseif(APPLE)
        set(STEAM_LINK_LIBRARIES ${STEAM_SDK_PATH}/redistributable_bin/osx/libsteam_api.dylib)
        set(STEAM_BINARIES ${STEAM_LINK_LIBRARIES})
    else()
        message( FATAL_ERROR "Uh Oh" )
    endif()

    message("STEAM LINK LIBRARIES: ${STEAM_LINK_LIBRARIES}")
endif()

if (NOT DEFINED EOS_SDK_PATH)
    message( "Expected cmake -DEOS_SDK_PATH=<path> )")
    message( FATAL_ERROR "Please provide a valid sdk path" )
else()
    message("EOS SDK PATH: ${EOS_SDK_PATH}")
    set(EOS_INCLUDE_DIRS ${EOS_SDK_PATH}/Include)
    if (WIN32)
        SET(CMAKE_FIND_LIBRARY_PREFIXES "")
        SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib" ".dll")
    elseif (APPLE)
        SET(CMAKE_FIND_LIBRARY_PREFIXES "lib")
        SET(CMAKE_FIND_LIBRARY_SUFFIXES ".dylib" ".a")
    else ()
        SET(CMAKE_FIND_LIBRARY_PREFIXES "lib")
        SET(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".a")
    endif ()

    if (WIN32)
        set(EOS_LINK_LIBRARIES ${EOS_SDK_PATH}/Lib/EOSSDK-Win64-Shipping.lib)
    elseif(APPLE)
        set(EOS_LINK_LIBRARIES ${EOS_SDK_PATH}/Bin/libEOSSDK-Mac-Shipping.dylib)
        set(EOS_BINARIES ${EOS_SDK_PATH}/Bin/libEOSSDK-Mac-Shipping.dylib)
    else()
        set(EOS_LINK_LIBRARIES ${EOS_SDK_PATH}/Bin/libEOSSDK-Linux-Shipping.so)
        set(EOS_BINARIES ${EOS_SDK_PATH}/Bin/libEOSSDK-Linux-Shipping.so)
    endif()

    message("EOS LINK LIBRARIES: ${EOS_LINK_LIBRARIES}")
endif()

set(CMAKE_CXX_STANDARD 11)
include_directories(${STEAM_INCLUDE_DIRS})
include_directories(${EOS_INCLUDE_DIRS})
include_directories(include)

add_executable(main main.cpp)
target_link_libraries(main ${STEAM_LINK_LIBRARIES})
target_link_libraries(main ${EOS_LINK_LIBRARIES})
add_custom_command(TARGET main POST_BUILD        # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${STEAM_BINARIES}"      # <--this is in-file
        $<TARGET_FILE_DIR:main>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  
        "${EOS_BINARIES}"
        $<TARGET_FILE_DIR:main>)
